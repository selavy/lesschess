#include "position.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <cinttypes>
#include "detail/magic_tables.generated.h"

namespace lesschess {

/*static*/ u64 Zobrist::_values[SIZE];

struct PossibleMoves
{
    struct Iterator {
        explicit Iterator(u64 b) noexcept : _b{b} {}
        int operator*() const { return lsb(_b); }
        Iterator& operator++() {
            _b = clear_lsb(_b);
            return *this;
        }
        bool operator==(Iterator other) noexcept { return _b == other._b; }
        bool operator!=(Iterator other) noexcept { return _b != other._b; }
        u64 _b;
    };

    explicit PossibleMoves(u64 b) noexcept : _b{b} {}
    Iterator begin() const { return Iterator{ _b }; }
    Iterator end()   const { return Iterator{ 0ull }; }
    u64 _b;
};

namespace
{

Move* add_pawn_moves_to_square(int frsq, int tosq, Move* moves) noexcept
{
    if (tosq >= A8 || tosq <= H1)  { // promotion
        *moves++ = Move::make_promotion(frsq, tosq, KNIGHT);
        *moves++ = Move::make_promotion(frsq, tosq, BISHOP);
        *moves++ = Move::make_promotion(frsq, tosq, ROOK);
        *moves++ = Move::make_promotion(frsq, tosq, QUEEN);
    } else {
        *moves++ = Move(frsq, tosq);
    }
    return moves;
}

Piece translate_fen_piece(char c)
{

    Color color = std::isupper(c) ? WHITE : BLACK;
    c = std::tolower(c);
    PieceKind piece;
    switch (c) {
        case 'p': piece = PAWN; break;
        case 'n': piece = KNIGHT; break;
        case 'b': piece = BISHOP; break;
        case 'r': piece = ROOK; break;
        case 'q': piece = QUEEN; break;
        case 'k': piece = KING; break;
        default: throw std::runtime_error{"Invalid piece"};
    }
    return Piece{color, piece};
}

} // ~anonymous namespace

Position::Position() noexcept
    : _moves(1u)
    , _halfmoves(0u)
    , _wtm(1u)
    , _ep_target(Position::ENPASSANT_NONE)
    , _castle_rights(CASTLE_NONE)
{
    _boards.fill(0ull);
    _sidemask.fill(0ull);
    _sq2pc.fill(NO_PIECE);
    _compute_zobrist_hash();
}

template <class Iter>
void parse_fen_spec(Iter it, Iter last, Position& position)
{
    auto expect = [&it, last](char c) {
        if (it == last || *it != c) {
            std::string msg = "Expected '";
            msg += c;
            msg += "', received '";
            if (it == last) {
                msg += "<eof>";
            } else {
                msg += *it;
            }
            msg += "'";
            throw std::runtime_error(msg.c_str());
        }
        ++it;
    };

    // Active Color
    if (it == last) {
        throw std::runtime_error("Expected color specification");
    }
    switch (*it++) {
        case 'W':
        case 'w':
            position._set_white_to_move(true);
            break;
        case 'B':
        case 'b':
            position._set_white_to_move(false);
            break;
        default:
            throw std::runtime_error("Invalid character in color specification");
    }
    expect(' ');

    // Castling Availability
    if (it == last) {
        throw std::runtime_error("Expected castling availability specification");
    } else if (*it == '-') {
        position._castle_rights = CASTLE_NONE;
        ++it;
    } else {
        u8 flags = 0;
        while (it < last && *it != ' ') {
            char c = *it++;
            switch (c) {
            case 'K':
                flags |= static_cast<u8>(CastleKind::WHITE_KING_SIDE);
                break;
            case 'k':
                flags |= static_cast<u8>(CastleKind::BLACK_KING_SIDE);
                break;
            case 'Q':
                flags |= static_cast<u8>(CastleKind::WHITE_QUEEN_SIDE);
                break;
            case 'q':
                flags |= static_cast<u8>(CastleKind::BLACK_QUEEN_SIDE);
                break;
            default:
                throw std::runtime_error("Invalid character in castling specification");
            }
        }
        position._castle_rights = flags;
    }
    expect(' ');

    // En passant Target Square
    if (it == last) {
        throw std::runtime_error("Expected en passant target square");
    } else if (*it == '-') {
        position._set_enpassant_square(Position::ENPASSANT_NONE);
        ++it;
    } else {
        char c = *it++;
        if (!(c >= 'a' && c <= 'h')) {
            throw std::runtime_error("Invalid file for enpassant target square");
        }
        u8 file = c - 'a';
        u8 rank = *it++ - '1';
        assert(file >= 0 && file <= 7);
        assert(rank >= 0 && rank <= 7);
        Square square{file, rank};
        if (rank == RANK_3) {
            assert(square.value() >= A3 && square.value() <= H3);
            position._set_enpassant_square(square.value());
        } else if (rank == RANK_6) {
            assert(square.value() >= A6 && square.value() <= H6);
            position._set_enpassant_square(square.value());
        } else {
            throw std::runtime_error("Invalid rank for enpassant target square");
        }
    }

    // may or may not have halfmove and move specifications
    bool has_halfmove = it != last && *it++ == ' ';
    if (has_halfmove && it != last) {
        // Halfmove spec (50-move rule)
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid halfmove specification");
            }
            position._halfmoves *= 10;
            position._halfmoves += *it - '0';
            ++it;
        }

        // assuming that if halfmove is there then move spec is also there
        expect(' ');
        position._moves = 0;
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid move specification");
            }
            position._moves *= 10;
            position._moves += *it - '0';
            ++it;
        }
    }
}

Position Position::from_ascii(std::string_view ascii)
{
    Position position;
    auto it = ascii.begin();
    auto end = ascii.end();

    auto expect = [&it, end](char c) {
        if (it == end || *it != c) {
            std::string msg = "Expected '";
            msg += c;
            msg += "', received '";
            if (it == end) {
                msg += "<eof>";
            } else {
                msg += *it;
            }
            msg += "'";
            throw std::runtime_error(msg.c_str());
        }
        ++it;
    };

    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            if (it == end) {
                throw std::runtime_error("Board specification too short");
            }
            expect('|');
            if (*it != ' ') {
                Piece piece = translate_fen_piece(*it);
                Square sq(static_cast<u8>(file), static_cast<u8>(rank));
                position._sq2pc[sq.value()] = piece;
                position._sidemask[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position._kings[piece.color()] = sq;
                } else {
                    position._boards[piece.value()] |= sq.mask();
                }
            }
            ++it;
        }
        expect('|');
        expect('\n');
    }

    parse_fen_spec(it, end, position);
    position._compute_zobrist_hash();
    position._validate();
    return position;
}

Position Position::from_fen(std::string_view fen)
{
    Position position;
    auto it = fen.begin();
    const auto last = fen.end();

    auto expect = [last](std::string_view::iterator it, char c) {
        if (it == last || *it != c) {
            std::string msg = "Expected '";
            msg += c;
            msg += "', received '";
            if (it == last) {
                msg += "<eof>";
            } else {
                msg += *it;
            }
            msg += "'";
            throw std::runtime_error(msg.c_str());
        }
        ++it;
        return it;
    };

    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            if (it == last) {
                throw std::runtime_error("Board specification too short");
            }
            char c = *it++;
            if (c >= '1' && c <= '8') {
                file += c - '0' - 1;
                if (file >= 8) {
                    throw std::runtime_error("Invalid board specification");
                }
            } else {
                Piece piece = translate_fen_piece(c);
                Square sq(static_cast<u8>(file), static_cast<u8>(rank));
                position._sq2pc[sq.value()] = piece;
                position._sidemask[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position._kings[piece.color()] = sq;
                } else {
                    position._boards[piece.value()] |= sq.mask();
                }
            }
        }
        if (it < last && *it == '/') {
            ++it;
        }
    }
    it = expect(it, ' ');

    parse_fen_spec(it, last, position);
    position._compute_zobrist_hash();

    position._validate();
    return position;
}

Move Position::move_from_long_algebraic(std::string_view move) const
{
    if (move.size() < 4) {
        // TODO: don't use exceptions
        throw std::runtime_error{"invalid move"};
    }
    auto to_square = [](char c, char r) {
        int rv = 0;
        if (r >= '1' && r <= '8') {
            rv += 8*(r - '1');
        } else {
            return -1;
        }
        if (c >= 'a' && c <= 'h') {
            rv += c - 'a';
        } else if (c >= 'A' && c <= 'H') {
            rv += c - 'A';
        } else {
            return -1;
        }
        return rv;
    };
    int from = to_square(move[0], move[1]);
    int to   = to_square(move[2], move[3]);
    if (from < 0 || to < 0 || move.size() > 5) {
        throw std::runtime_error("invalid square");
    }
    assert(from >= A1 && from <= H8);
    assert(to   >= A1 && to   <= H8);

    // TODO: improve checking that move given is in fact valid. Right now relying
    //       on Arena not sending an invalid move. Don't want unnecessary extra
    //       checking in make_move().

    if (move.size() == 5) {
        auto found = PieceKindAlgebraicNames.find(move[4]);
        int promo = found != PieceKindAlgebraicNames.end() ? found->second : -1;
        return Move::make_promotion(from, to, static_cast<PieceKind>(promo));
    }

    Piece p1 = piece_on_square(from);
    Piece p2 = piece_on_square(to);

    if (p1.empty()) {
        throw std::runtime_error("moving from empty square");
    }

    if (p1.color() != color_to_move()) {
        throw std::runtime_error("moving opponent's piece");
    }

    if (p1.kind() == KING && p2.empty()) {
        if (p1.color() == WHITE && from == E1 && to == G1) {
            return Move::make_castle(CastleKind::WHITE_KING_SIDE);
        } else if (p1.color() == WHITE && from == E1 && to == C1) {
            return Move::make_castle(CastleKind::WHITE_QUEEN_SIDE);
        } else if (p1.color() == BLACK && from == E8 && to == G8) {
            return Move::make_castle(CastleKind::BLACK_KING_SIDE);
        } else if (p1.color() == BLACK && from == E8 && to == C8) {
            return Move::make_castle(CastleKind::BLACK_QUEEN_SIDE);
        }
    }

    if (p1.kind() == PAWN && p2.empty() && Square{from}.file() != Square{to}.file()) {
        if (Square{to} != ep_target()) {
            throw std::runtime_error("invalid en passant");
        }
        return Move::make_enpassant(from, to);
    }

    return Move(from, to);
}

std::string Position::dump_fen() const noexcept
{
    std::string result;
    int emptys = 0;
    for (int rank = RANK_8; rank >= RANK_1; --rank) {
        for (int file = FILE_A; file <= FILE_H; ++file) {
            const Square square(static_cast<u8>(file), static_cast<u8>(rank));
            const Piece piece = piece_on_square(square);
            if (piece.empty()) {
                ++emptys;
            } else {
                if (emptys > 0) {
                    result += std::to_string(emptys);
                    emptys = 0;
                }
                result += piece.fen();
            }
        }
        if (emptys > 0) {
            result += std::to_string(emptys);
            emptys = 0;
        }
        if (rank != RANK_1) {
            result += '/';
        }
    }

    result += ' ';
    result += _wtm == WHITE ? 'w' : 'b';
    result += ' ';

    if (castle_kind_allowed(CastleKind::WHITE_KING_SIDE)) {
        result += 'K';
    }
    if (castle_kind_allowed(CastleKind::WHITE_QUEEN_SIDE)) {
        result += 'Q';
    }
    if (castle_kind_allowed(CastleKind::BLACK_KING_SIDE)) {
        result += 'k';
    }
    if (castle_kind_allowed(CastleKind::BLACK_QUEEN_SIDE)) {
        result += 'q';
    }
    if (_castle_rights == CASTLE_NONE) {
        result += '-';
    }
    result += ' ';

    if (enpassant_available()) {
        result += enpassant_target_square().name();
    } else {
        result += '-';
    }
    result += ' ';

    result += std::to_string(fifty_move_rule_moves());
    result += ' ';
    result += std::to_string(move_number());

    return result;
}

std::string fen_to_ascii(std::string fen) {
    std::string result = "|";
    int rows = 0;
    auto it = fen.begin();
    for (; it != fen.end(); ++it) {
        char c = *it;
        if (c == ' ') {
            result += '\n';
            ++it;
            while (it != fen.end()) {
                result += *it++;
            }
            break;
        } else if (c == '/') {
            result += "\n|";
            ++rows;
        } else if (c >= '1' && c <= '8') {
            int skip = c - '1';
            while (skip-- >= 0) {
                result += " |";
            }
        } else if (c >= 'a' && c <= 'z') {
            result += c;
            result += '|';
        } else if (c >= 'A' && c <= 'Z') {
            result += c;
            result += '|';
        } else {
            throw std::runtime_error("invalid character in FEN");
        }
    }
    return result;
}

[[nodiscard]]
std::string Position::dump_ascii() const noexcept {
    return fen_to_ascii(dump_fen());
}

[[nodiscard]]
constexpr u8 rook_square_to_castle_flag(Square square) noexcept
{
    switch (square.value()) {
        case A8: return static_cast<u8>(CastleKind::BLACK_QUEEN_SIDE);
        case H8: return static_cast<u8>(CastleKind::BLACK_KING_SIDE);
        case A1: return static_cast<u8>(CastleKind::WHITE_QUEEN_SIDE);
        case H1: return static_cast<u8>(CastleKind::WHITE_KING_SIDE);
        default: return 0;
    }
}

[[nodiscard]]
constexpr bool is_rank2(Color side, Square square) noexcept
{
    constexpr u64 SECOND_RANK{0xff00ull};
    constexpr u64 SEVENTH_RANK{0xff000000000000ull};
    u64 mask = side == WHITE ? SECOND_RANK : SEVENTH_RANK;
    return square.mask() & mask;
}

[[nodiscard]]
constexpr bool is_rank7(Color side, Square square) noexcept {
    return is_rank2(flip_color(side), square);
}

[[nodiscard]]
constexpr bool is_enpassant_square(u8 side, Square square) noexcept {
    constexpr u64 WHITE_ENPASSANT_SQUARES{0x00000000ff000000ull};
    constexpr u64 BLACK_ENPASSANT_SQUARES{0x000000ff00000000ull};
    u64 mask = side == WHITE ? WHITE_ENPASSANT_SQUARES : BLACK_ENPASSANT_SQUARES;
    return square.mask() & mask;
}

constexpr std::pair<Square, Square>
_get_castle_squares(Square to) noexcept {
    switch (to.value()) {
        case H1: return std::make_pair(Square(G1), Square(F1));
        case A1: return std::make_pair(Square(C1), Square(D1));
        case H8: return std::make_pair(Square(G8), Square(F8));
        case A8: return std::make_pair(Square(C8), Square(D8));
        default:
            assert(0 && "invalid castle target square");
            __builtin_unreachable();
    }
}

void Position::make_move(Savepos& sp, Move move) noexcept {
    _validate();

    assert(_wtm == WHITE || _wtm == BLACK);
    const Color side = static_cast<Color>(_wtm);
    const Color contra = flip_color(side);
    const Square from = move.from();
    const Square to = move.to();
    const Piece piece = piece_on_square(from);
    const Piece captured = piece_on_square(to);
    const Move::Flags flags = move.flags();
    const PieceKind kind = piece.kind();
    u64* board = kind != KING ? &_boards[piece.value()] : 0;
    Square new_ep_target = Square();

    assert(to != from);
    assert(captured.kind() != KING);
    assert(piece.color() == side);
    assert(captured.empty() || captured.color() == contra || (move.is_castle() && captured.color() == side));

    sp.halfmoves = _halfmoves;
    sp.ep_target = _ep_target;
    sp.castle_rights = _castle_rights;
    sp.captured = captured;
    _hash ^= Zobrist::side_to_move();
    if (enpassant_available()) {
        _hash ^= Zobrist::enpassant(enpassant_target_square());
    }

    if (flags == Move::Flags::NONE) {
        if (board) {
            *board &= ~from.mask();
            *board |= to.mask();
        } else {
            _kings[side] = to;
            if (side == WHITE) {
                if (castle_kind_allowed(CastleKind::WHITE_KING_SIDE)) {
                    _hash ^= Zobrist::castle_rights(CastleKind::WHITE_KING_SIDE);
                }
                if (castle_kind_allowed(CastleKind::WHITE_QUEEN_SIDE)) {
                    _hash ^= Zobrist::castle_rights(CastleKind::WHITE_QUEEN_SIDE);
                }
                _castle_rights &= ~CASTLE_WHITE_ALL;
            } else {
                if (castle_kind_allowed(CastleKind::BLACK_KING_SIDE)) {
                    _hash ^= Zobrist::castle_rights(CastleKind::BLACK_KING_SIDE);
                }
                if (castle_kind_allowed(CastleKind::BLACK_QUEEN_SIDE)) {
                    _hash ^= Zobrist::castle_rights(CastleKind::BLACK_QUEEN_SIDE);
                }
                _castle_rights &= ~CASTLE_BLACK_ALL;
            }
        }
        _sq2pc[from.value()] = NO_PIECE;
        _sq2pc[to.value()] = piece;
        _sidemask[side] &= ~from.mask();
        _sidemask[side] |= to.mask();

        if (!captured.empty()) {
            _boards[captured.value()] &= ~to.mask();
            _sidemask[contra] &= ~to.mask();
            _castle_rights &= ~rook_square_to_castle_flag(to);
        } else if (kind == PAWN && is_rank2(side, from) && is_enpassant_square(side, to)) {
            u8 target = side == WHITE ? to.value() - 8 : to.value() + 8;
            new_ep_target = target;
            _hash ^= Zobrist::enpassant(new_ep_target);
            assert((target >= A3 && target <= H3) || (target >= A6 && target <= H6));
        }

        if (kind == ROOK) {
            _castle_rights &= ~rook_square_to_castle_flag(from);
        }
    } else if (flags == Move::Flags::ENPASSANT) {
        assert(enpassant_available() == true);
        assert(kind == PAWN);
        assert(captured.empty());
        // TODO(peter): better name than epsq
        // NOTE(peter): :epsq: is the square that the contra pawn is on.
        const Square epsq = pawn_backward(side, to.value());
        *board &= ~from.mask();
        *board |= to.mask();
        _boards[Piece(contra, PAWN).value()] &= ~epsq.mask();
        _sq2pc[from.value()] = NO_PIECE;
        _sq2pc[to.value()] = piece;
        _sq2pc[epsq.value()] = NO_PIECE;
        _sidemask[side] &= ~from.mask();
        _sidemask[side] |= to.mask();
        _sidemask[contra] &= ~epsq.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        const PieceKind promotion_kind = move.promotion();
        const Piece promotion_piece = Piece(side, promotion_kind);
        assert(kind == PAWN);
        *board &= ~from.mask();
        _boards[promotion_piece.value()] |= to.mask();
        _sq2pc[to.value()] = promotion_piece;
        _sq2pc[from.value()] = NO_PIECE;
        _sidemask[side] &= ~from.mask();
        _sidemask[side] |= to.mask();
        if (!captured.empty()) {
            _boards[captured.value()] &= ~to.mask();
            _sidemask[contra] &= ~to.mask();
            _castle_rights &= ~rook_square_to_castle_flag(to);
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(kind == KING);
        u64* rooks = &_boards[Piece(side, ROOK).value()];
        auto [ksq, rsq] = _get_castle_squares(to);
        _kings[side] = ksq;
        *rooks &= ~to.mask();
        *rooks |= rsq.mask();

        _sq2pc[from.value()] = NO_PIECE;
        _sq2pc[to.value()] = NO_PIECE;
        _sq2pc[ksq.value()] = Piece(side, KING);
        _sq2pc[rsq.value()] = Piece(side, ROOK);
        _sidemask[side] &= ~(to.mask() | from.mask());
        _sidemask[side] |= ksq.mask() | rsq.mask();
        if (side == WHITE) {
            _castle_rights &= ~CASTLE_WHITE_ALL;
        } else {
            _castle_rights &= ~CASTLE_BLACK_ALL;
        }
    } else {
        assert(0);
        __builtin_unreachable();
    }

    _set_enpassant_square(new_ep_target.value());

    _wtm = contra;
    if (kind == PAWN || !captured.empty() && !move.is_castle()) {
        _halfmoves = 0;
    } else {
        ++_halfmoves;
    }
    if (side == BLACK) {
        ++_moves;
    }

    _validate();
}

void Position::undo_move(const Savepos& save, Move move) noexcept {
    _validate();
    assert(_wtm == WHITE || _wtm == BLACK);

    const Color contra = static_cast<Color>(_wtm);
    const Color side = flip_color(contra);
    const Square from = move.from();
    const Square to = move.to();
    const Move::Flags flags = move.flags();
    const Piece piece = piece_on_square(to);
    const PieceKind kind = piece.kind();
    u64* board = &_boards[piece.value()];
    const Piece captured = save.captured;

    _halfmoves = save.halfmoves;
    _ep_target = save.ep_target;
    _castle_rights = save.castle_rights;
    if (side == BLACK) {
        --_moves;
    }
    _wtm = side;

    if (flags == Move::Flags::NONE) {
        if (kind != KING) {
            *board |= from.mask();
            *board &= ~to.mask();
        } else {
            _kings[side] = from;
        }
        _sidemask[side] |= from.mask();
        _sidemask[side] &= ~to.mask();
        _sq2pc[from.value()] = piece;
        _sq2pc[to.value()] = captured;
        if (!captured.empty()) {
            _boards[captured.value()] |= to.mask();
            _sidemask[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(move.is_castle());
        Piece rook = Piece(side, ROOK);
        Piece king = Piece(side, KING);
        auto [ksq, rsq] = _get_castle_squares(to);
        _sq2pc[from.value()] = king;
        _sq2pc[to.value()] = rook;
        _sq2pc[ksq.value()] = NO_PIECE;
        _sq2pc[rsq.value()] = NO_PIECE;
        _sidemask[side] |= to.mask() | from.mask();
        _sidemask[side] &= ~(ksq.mask() | rsq.mask());
        _kings[side] = from;
        // TODO(peter): verify only doing 1 load here
        _boards[rook.value()] &= ~rsq.mask();
        _boards[rook.value()] |= to.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        assert(move.is_promotion());
        Piece pawn = Piece(side, PAWN);
        Piece promoted = Piece(side, move.promotion());
        _boards[pawn.value()] |= from.mask();
        _boards[promoted.value()] &= ~to.mask();
        _sq2pc[to.value()] = captured;
        _sq2pc[from.value()] = pawn;
        _sidemask[side] |= from.mask();
        _sidemask[side] &= ~to.mask();
        if (!captured.empty()) {
            _boards[captured.value()] |= to.mask();
            _sidemask[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::ENPASSANT) {
        // TODO(peter): better name for :epsq:
        Square epsq = side == WHITE ? to.value() - 8 : to.value() + 8;
        Piece opp_pawn = Piece(contra, PAWN);
        _sq2pc[from.value()] = piece;
        *board |= from.mask();
        *board &= ~to.mask();
        _sidemask[side] |= from.mask();
        _sidemask[side] &= ~to.mask();
        _sidemask[contra] |= epsq.mask();
        _sq2pc[to.value()] = NO_PIECE;
        _sq2pc[epsq.value()] = opp_pawn;
        _boards[opp_pawn.value()] |= epsq.mask();
    } else {
        assert(0);
        __builtin_unreachable();
    }

    _validate();
}

bool Position::operator==(const Position& rhs) const noexcept {
    const Position& lhs = *this;
    return ((lhs._boards == rhs._boards) &&
            (lhs._sidemask == rhs._sidemask) &&
            (lhs._sq2pc == rhs._sq2pc) &&
            (lhs._kings == rhs._kings) &&
            (lhs._moves == rhs._moves) &&
            // TODO: temp fix me!
            // (lhs._hash == rhs._hash) &&
            (lhs._halfmoves == rhs._halfmoves) &&
            (lhs._wtm == rhs._wtm) &&
            (lhs._ep_target == rhs._ep_target) &&
            (lhs._castle_rights == rhs._castle_rights)
           );
}

bool Position::operator!=(const Position& rhs) const noexcept
{ return !(*this == rhs); }

int Position::generate_legal_moves(Move* moves) const noexcept
{
    Color side = wtm();
    Square ksq = _kings[side];
    u64 pinned = _generate_pinned(side, side);
    u64 checkers = _generate_checkers(side);

    Move* cur = moves;
    Move* end = checkers != 0 ?
        _generate_evasions(checkers, moves) : _generate_non_evasions(moves);

    auto must_double_check = [&](Move move) {
        // need to double check legality if:
        //   + is a king move (but not castling)
        //   + there are pinned pieces
        //   + the move is en passant
        return move.from() == ksq || pinned || move.is_enpassant();
    };

    // walk through list of pseudo-legal moves generated, for special cases
    // where we need to double-check the legality, if they are not legal, then
    // swap with the last element.
    while (cur != end) {
        if (must_double_check(*cur) && !_is_legal(pinned, *cur)) {
            *cur = *(--end);
        } else {
            ++cur;
        }
    }

    return (int)(end - moves);
}

int Position::generate_captures(Move* moves) const noexcept
{
    Move* cur = moves;
    int nmoves = generate_legal_moves(moves);
    Move* end = cur + nmoves;
    while (cur != end) {
        // TODO: check this
        if (cur->is_enpassant() || !piece_on_square(cur->to()).empty()) {
            ++cur;
        } else {
            *cur = *(--end);
        }
    }
    return (int)(end - moves);
}

u64 Position::_generate_attacked(Color side) const noexcept
{
    Color contra = flip_color(side);
    // XXX: why remove the king position?
    u64 occupied = _occupied() & ~_kings[contra].mask();
    u64 knights = _bboard(side, KNIGHT);
    u64 bishops = _bboard(side, BISHOP);
    u64 rooks   = _bboard(side, ROOK);
    u64 queens  = _bboard(side, QUEEN);
    u64 pawns   = _bboard(side, PAWN);

    u64 rval = 0;

    // knight attacks
    {
        u64 pieces = knights;
        while (pieces) {
            int from = lsb(pieces);
            rval |= knight_attacks(from);
            pieces = clear_lsb(pieces);
        }
    }

    // diagonal attacks
    {
        u64 pieces = bishops | queens;
        while (pieces) {
            int from = lsb(pieces);
            rval |= bishop_attacks(from, occupied);
            pieces = clear_lsb(pieces);
        }
    }

    // straight attacks
    {
        u64 pieces = rooks | queens;
        while (pieces) {
            int from = lsb(pieces);
            rval |= rook_attacks(from, occupied);
            pieces = clear_lsb(pieces);
        }
    }

    // king attacks
    rval |= king_attacks(_kings[side].value());

    // pawn attacks - left
    {
        u64 pieces = pawns & ~A_FILE;
        pieces = side == WHITE ? pieces << 7 : pieces >> 9;
        rval |= pieces;
    }

    // pawn attacks - right
    {
        u64 pieces = pawns & ~H_FILE;
        pieces = side == WHITE ? pieces << 9 : pieces >> 7;
        rval |= pieces;
    }

    return rval;
}

Move* Position::_generate_evasions(u64 checkers, Move* moves) const noexcept
{
    assert(checkers != 0 && "_generate_evasions should only be called if in check");

    // 0. General case: either move king, capture piece, or block
    // 1. Knight or pawn check: either move king, or capture knight
    // 2. If more than 1 checker, then must move king
    // 3. en passant could remove the attacker

    Color side = wtm();
    Color contra = flip_color(side);
    Square ksq = _kings[side];
    u64 attacked = _generate_attacked(contra);
    u64 safe = ~_sidemask[side] & ~attacked;

    // generate king moves to squares that are not under attack
    moves = _generate_king_moves(ksq, safe, moves);

    if (more_than_one_piece(checkers)) {
        // if more than 1 checker, then only option is to move the king
        return moves;
    }

    assert(more_than_one_piece(checkers) == false);
    u64 targets = checkers;
    Square check_square = lsb(checkers);
    Piece  check_piece = piece_on_square(check_square);
    u64 pawns = _bboard(side, PAWN);
    u64 knights = _bboard(side, KNIGHT);
    u64 bishops = _bboard(side, BISHOP);
    u64 rooks   = _bboard(side, ROOK);
    u64 queens  = _bboard(side, QUEEN);
    u64 occupied = _occupied();

    if (check_piece.kind() == PAWN) {
        if (_ep_target != Position::ENPASSANT_NONE && ep_capture_square() == check_square) {
            // capture left
            if (_ep_target != H6 && _ep_target != H3) {
                int from = side == WHITE ? _ep_target - 7 : _ep_target + 9;
                if (piece_on_square(from) == Piece(side, PAWN)) {
                    *moves++ = Move::make_enpassant(from, _ep_target);
                }
            }

            // capture right
            if (_ep_target != A6 && _ep_target != A3) {
                int from = side == WHITE ? _ep_target - 9 : _ep_target + 7;
                if (piece_on_square(from) == Piece(side, PAWN)) {
                    *moves++ = Move::make_enpassant(from, _ep_target);
                }
            }
        }
    } else if (check_piece.kind() != KNIGHT) {
        u64 between = between_sqs(check_square.value(), ksq.value());

        // try to advance pawns 1 square to block
        {
            u64 posmoves = (side == WHITE ? pawns << 8 : pawns >> 8) & between;
            for (int tosq : PossibleMoves{posmoves}) {
                int frsq = pawn_backward(side, tosq);
                assert(piece_on_square(frsq) == Piece(side, PAWN));
                moves = add_pawn_moves_to_square(frsq, tosq, moves);
            }
        }

        // for rank 2 pawns, try advancing 2 squares
        {
            u64 posmoves = pawns & RANK2(side);
            posmoves = (side == WHITE ? posmoves << 16 : posmoves >> 16) & between;
            for (int tosq : PossibleMoves{posmoves}) {
                int frsq = side == WHITE ? tosq - 16 : tosq + 16;
                assert(piece_on_square(frsq) == Piece(side, PAWN));
                // TODO: do this with a bitmask
                if (piece_on_square(pawn_backward(side, tosq)).empty()) {
                    *moves++ = Move(frsq, tosq);
                }
            }
        }

        // includes blockers in target squares (because checker isn't a knight or pawn)
        targets |= between;
    }

    moves = _generate_knight_moves(knights, targets, moves);
    moves = _generate_bishop_moves(bishops | queens, occupied, targets, moves);
    moves = _generate_rook_moves(rooks | queens, occupied, targets, moves);

    // capture left
    {
        u64 posmoves = pawns & ~A_FILE;
        posmoves = (side == WHITE ? posmoves << 7 : posmoves >> 9) & checkers;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = side == WHITE ? tosq - 7 : tosq + 9;
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            moves = add_pawn_moves_to_square(frsq, tosq, moves);
        }
    }

    // capture right
    {
        u64 posmoves = pawns & ~H_FILE;
        posmoves = (side == WHITE ? posmoves << 9 : posmoves >> 7) & checkers;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = side == WHITE ? tosq - 9 : tosq + 7;
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            moves = add_pawn_moves_to_square(frsq, tosq, moves);
        }
    }

    return moves;
}

Move* Position::_generate_non_evasions(Move* moves) const noexcept
{
    Color side = wtm();
    Color contra = flip_color(side);
    u64 occupied = _occupied();
    u64 targets = _sidemask[contra];
    u64 opp_or_empty = ~_sidemask[side];
    u64 knights = _bboard(side, KNIGHT);
    u64 bishops = _bboard(side, BISHOP);
    u64 rooks   = _bboard(side, ROOK);
    u64 queens  = _bboard(side, QUEEN);
    u64 pawns   = _bboard(side, PAWN);
    Square ksq = _kings[side];

    moves = _generate_knight_moves(knights, opp_or_empty, moves);
    moves = _generate_bishop_moves(bishops | queens, occupied, opp_or_empty, moves);
    moves = _generate_rook_moves(rooks | queens, occupied, opp_or_empty, moves);
    moves = _generate_king_moves(ksq, opp_or_empty, moves);
    moves = _generate_castle_moves(side, ksq, moves);

    // 1-square pawn moves
    {
        u64 posmoves = (side == WHITE ? pawns << 8 : pawns >> 8) & ~occupied;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = pawn_backward(side, tosq);
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            moves = add_pawn_moves_to_square(frsq, tosq, moves);
        }
    }

    // 2-square pawn moves
    {
        u64 posmoves = pawns & RANK2(side);
        posmoves = (side == WHITE ? posmoves << 16 : posmoves >> 16) & ~occupied;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = pawn_backward(side, tosq, /*ranks*/2);
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            if (piece_on_square(pawn_backward(side, tosq)).empty()) {
                *moves++ = Move(frsq, tosq);
            }
        }
    }

    // pawn capture left
    {
        u64 posmoves = pawns & ~A_FILE;
        posmoves = (side == WHITE ? posmoves << 7 : posmoves >> 9) & targets;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = pawn_capture_backward_left(side, tosq);
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            assert(piece_on_square(tosq).empty() == false);
            moves = add_pawn_moves_to_square(frsq, tosq, moves);
        }
    }

    // pawn capture right
    {
        u64 posmoves = pawns & ~H_FILE;
        posmoves = (side == WHITE ? posmoves << 9 : posmoves >> 7) & targets;
        for (int tosq : PossibleMoves{posmoves}) {
            int frsq = pawn_capture_backward_right(side, tosq);
            assert(piece_on_square(frsq) == Piece(side, PAWN));
            assert(piece_on_square(tosq).empty() == false);
            moves = add_pawn_moves_to_square(frsq, tosq, moves);
        }
    }

    // en passant captures
    if (_ep_target != Position::ENPASSANT_NONE) {
        assert(piece_on_square(_ep_target).empty() == true);

        // capture left
        if (_ep_target != H6 && _ep_target != H3) {
            int frsq = pawn_capture_backward_left(side, _ep_target);
            if (piece_on_square(frsq) == Piece(side, PAWN)) {
                *moves++ = Move::make_enpassant(frsq, _ep_target);
            }
        }

        // capture right
        if (_ep_target != A6 && _ep_target != A3) {
            int frsq = pawn_capture_backward_right(side, _ep_target);
            if (piece_on_square(frsq) == Piece(side, PAWN)) {
                *moves++ = Move::make_enpassant(frsq, _ep_target);
            }
        }
    }

    return moves;
}

void Position::_compute_zobrist_hash() noexcept
{
    u64 hash = 0;

    for (int i = 0; i < 64; ++i) {
        Square square{i};
        Piece piece = piece_on_square(square);
        if (!piece.empty()) {
            hash ^= Zobrist::board(piece, square);
        }
    }

    if (white_to_move()) {
        hash ^= Zobrist::side_to_move();
    }

    std::initializer_list<CastleKind> kinds = {
        CastleKind::WHITE_KING_SIDE,
        CastleKind::BLACK_KING_SIDE,
        CastleKind::WHITE_QUEEN_SIDE,
        CastleKind::BLACK_QUEEN_SIDE,
    };
    for (auto kind : kinds) {
        if (castle_kind_allowed(kind)) {
            hash ^= Zobrist::castle_rights(kind);
        }
    }

    if (enpassant_available()) {
        hash ^= Zobrist::enpassant(enpassant_target_square());
    }

    _hash = hash;
}

void Position::_validate() const noexcept {
#ifndef NDEBUG
    std::array<Piece, 12> pieces = {
        Piece(WHITE, PAWN),
        Piece(WHITE, KNIGHT),
        Piece(WHITE, BISHOP),
        Piece(WHITE, ROOK),
        Piece(WHITE, QUEEN),
        Piece(WHITE, KING),
        Piece(BLACK, PAWN),
        Piece(BLACK, KNIGHT),
        Piece(BLACK, BISHOP),
        Piece(BLACK, ROOK),
        Piece(BLACK, QUEEN),
        Piece(BLACK, KING),
    };

    // Verify _sidemasks
    u64 white_mask = 0ull;
    u64 black_mask = 0ull;
    for (auto piece: pieces) {
        u64 piece_mask = piece.kind() == KING ?  _kings[piece.color()].mask() : _boards[piece.value()];
        assert((white_mask & piece_mask) == 0ull);
        assert((black_mask & piece_mask) == 0ull);
        if (piece.color() == WHITE) {
            white_mask |= piece_mask;
        } else {
            black_mask |= piece_mask;
        }
    }
    u64 full_mask = white_mask | black_mask;
    assert(_sidemask[WHITE] == white_mask);
    assert(_sidemask[BLACK] == black_mask);
    assert((white_mask & black_mask) == 0ull); // no bits should overlap

    // Verify _sq2pc == _boards
    for (int i = 0; i < 64; ++i) {
        Piece piece = _sq2pc[i];
        Square square(i);
        if (piece.empty()) {
            assert((white_mask & square.mask()) == 0ull);
            assert((black_mask & square.mask()) == 0ull);
        } else if (piece.kind() == KING){
            assert(_kings[piece.color()] == square);
        } else {
            assert((_boards[piece.value()] & square.mask()) != 0);
        }
    }

    std::array<int, 14> counts;
    counts.fill(0);
    for (auto&& piece: _sq2pc) {
        ++counts[piece.value()];
    }
    assert(counts[Piece(WHITE, KING).value()] == 1);
    assert(counts[Piece(BLACK, KING).value()] == 1);
    assert(counts[Piece(WHITE, PAWN).value()] <= 8);
    assert(counts[Piece(BLACK, PAWN).value()] <= 8);

    if (castle_kind_allowed(CastleKind::WHITE_KING_SIDE)) {
        assert(piece_on_square(E1) == Piece(WHITE, KING));
        assert(piece_on_square(H1) == Piece(WHITE, ROOK));
    }
    if (castle_kind_allowed(CastleKind::WHITE_QUEEN_SIDE)) {
        assert(piece_on_square(E1) == Piece(WHITE, KING));
        assert(piece_on_square(A1) == Piece(WHITE, ROOK));
    }
    if (castle_kind_allowed(CastleKind::BLACK_KING_SIDE)) {
        assert(piece_on_square(E8) == Piece(BLACK, KING));
        assert(piece_on_square(H8) == Piece(BLACK, ROOK));
    }
    if (castle_kind_allowed(CastleKind::BLACK_QUEEN_SIDE)) {
        assert(piece_on_square(E8) == Piece(BLACK, KING));
        assert(piece_on_square(A8) == Piece(BLACK, ROOK));
    }
#endif
}

[[nodiscard]]
u64 Position::_generate_pinned(Color blocker_color, Color king_color) const noexcept
{
    u64 ret = 0;
    Color pinning_color = flip_color(blocker_color);
    u64 blockers = _sidemask[blocker_color];
    u64 occupied = _occupied();
    u32 ksq = _kings[king_color].value();
    u64 pinning_rooks = _bboard(pinning_color, ROOK);
    u64 pinning_queens = _bboard(pinning_color, QUEEN);
    u64 pinning_bishops = _bboard(pinning_color, BISHOP);
    u64 straight_pinners = (pinning_rooks | pinning_queens) & rook_attacks(ksq, 0);
    u64 diagonal_pinners = (pinning_bishops | pinning_queens) & bishop_attacks(ksq, 0);
    u64 pinners = straight_pinners | diagonal_pinners;
    while (pinners) {
        int sq = lsb(pinners);
        u64 bb = between_sqs(sq, ksq) & occupied;
        // is this the only piece between the attackers and the king?
        if (!more_than_one_piece(bb)) {
            ret |= bb & blockers;
        }
        pinners = clear_lsb(pinners);
    }
    return ret;
}

constexpr int castles_king_square(Move m) noexcept
{
    switch (m.castle_kind()) {
        case CastleKind::WHITE_KING_SIDE:  return G1;
        case CastleKind::BLACK_KING_SIDE:  return G8;
        case CastleKind::WHITE_QUEEN_SIDE: return C1;
        case CastleKind::BLACK_QUEEN_SIDE: return C8;
    }
    __builtin_unreachable();
}

bool Position::is_legal_move(Move move) const noexcept
{
    // very straight forward stupid implementation for now
    static Move moves[256];
    memset(&moves[0], 0, sizeof(Move)*256);
    int nmoves = generate_legal_moves(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        if (move == moves[i]) {
            return true;
        }
    }
    return false;
}

bool Position::attacks(Color side, Square square) const noexcept
{
    Color contra = flip_color(side);
    u64 occupied = _occupied();
    u64 queens = _bboard(side, QUEEN);
    u64 rooks = _bboard(side, ROOK);
    u64 knights = _bboard(side, KNIGHT);
    u64 bishops = _bboard(side, BISHOP);
    u64 pawns = _bboard(side, PAWN);
    u64 king = _kings[side].mask();
    int sq = square.value();

    if ((rook_attacks(sq, occupied) & (queens | rooks)) != 0) {
        return true;
    }
    if ((bishop_attacks(sq, occupied) & (queens | bishops)) != 0) {
        return true;
    }
    if ((knight_attacks(sq) & knights) != 0) {
        return true;
    }
    if ((pawn_attacks(contra, sq) & pawns) != 0) {
        return true;
    }
    if ((king_attacks(sq) & king) != 0) {
        return true;
    }
    return false;
}


// checks a pseudo-legal move for legality
bool Position::_is_legal(u64 pinned, Move move) const noexcept
{
    if (move.is_castle()) {
        return true;
    }

    Color side = wtm();
    Color contra = flip_color(side);
    Square tosq = move.to();
    Square frsq = move.from();
    Square ksq = _kings[side];

    if (move.is_enpassant()) {
        auto capture_sq = Square(pawn_backward(side, tosq.value()));
        u64 prev_occupied = _occupied();
        u64 queens = _bboard(contra, QUEEN);
        u64 rooks = _bboard(contra, ROOK);
        u64 bishops = _bboard(contra, BISHOP);
        // make the move and see if there are any checks post-move
        assert(piece_on_square(tosq).empty() == true);
        assert(piece_on_square(frsq) == Piece(side, PAWN));
        assert(piece_on_square(capture_sq) == Piece(contra, PAWN));
        assert((prev_occupied & tosq.mask()) == 0);
        assert((prev_occupied & frsq.mask()) != 0);
        assert((prev_occupied & capture_sq.mask()) != 0);
        u64 occupied = prev_occupied ^ frsq.mask() ^ capture_sq.mask() | tosq.mask();
        assert((occupied & tosq.mask()) != 0);
        assert((occupied & frsq.mask()) == 0);
        assert((occupied & capture_sq.mask()) == 0);
        u64 straight_attacks = rook_attacks(ksq.value(),   occupied) & (queens | rooks);
        u64 diagonal_attacks = bishop_attacks(ksq.value(), occupied) & (queens | bishops);
        return (straight_attacks | diagonal_attacks) == 0;
    }

    if (piece_on_square(move.from()).kind() == KING) {
        // don't need to remove the king before checking this, because the king
        // can't block a ray.
        return !attacks(contra, tosq);
    }

    // is the piece pinned?
    if (pinned == 0 || (pinned & frsq.mask()) == 0) {
        return true;
    }

    // trying to move a potentially pinned piece, will it still be pinned after moving?
    assert((pinned & frsq.mask()) != 0);
    return lined_up(frsq.value(), tosq.value(), _kings[side].value());
}

Move* Position::_generate_knight_moves(u64 knights, u64 targets, Move* moves) noexcept
{
    while (knights) {
        int from = lsb(knights);
        u64 posmoves = knight_attacks(from) & targets;
        for (int to : PossibleMoves{posmoves}) {
            *moves++ = Move(from, to);
        }
        knights = clear_lsb(knights);
    }
    return moves;
}

Move* Position::_generate_bishop_moves(u64 bishops, u64 occupied, u64 targets, Move* moves) noexcept
{
    while (bishops) {
        int from = lsb(bishops);
        u64 posmoves = bishop_attacks(from, occupied) & targets;
        for (int to : PossibleMoves{posmoves}) {
            *moves++ = Move(from, to);
        }
        bishops = clear_lsb(bishops);
    }
    return moves;
}

Move* Position::_generate_rook_moves(u64 rooks, u64 occupied, u64 targets, Move* moves) noexcept
{
    while (rooks) {
        int from = lsb(rooks);
        u64 posmoves = rook_attacks(from, occupied) & targets;
        for (int to : PossibleMoves{posmoves}) {
            *moves++ = Move(from, to);
        }
        rooks = clear_lsb(rooks);
    }
    return moves;
}

Move* Position::_generate_king_moves(Square ksq, u64 targets, Move* moves) noexcept
{
    u64 posmoves = king_attacks(ksq.value()) & targets;
    for (int to : PossibleMoves{posmoves}) {
        *moves++ = Move(ksq, to);
    }
    return moves;
}

Move* Position::_generate_castle_moves(Color side, Square ksq, Move* moves) const noexcept
{
    Color contra = flip_color(side);

    if (
            side == WHITE &&
            (castle_kind_allowed(CastleKind::WHITE_KING_SIDE)) &&
            piece_on_square(F1).empty() &&
            piece_on_square(G1).empty() &&
            !attacks(contra, E1) &&
            !attacks(contra, F1) &&
            !attacks(contra, G1)
       )
    {
        assert(piece_on_square(E1) == Piece(WHITE, KING));
        assert(piece_on_square(H1) == Piece(WHITE, ROOK));
        *moves++ = Move::make_castle(CastleKind::WHITE_KING_SIDE);
    }

    if (
            side == BLACK &&
            (castle_kind_allowed(CastleKind::BLACK_KING_SIDE)) &&
            piece_on_square(F8).empty() &&
            piece_on_square(G8).empty() &&
            !attacks(contra, E8) &&
            !attacks(contra, F8) &&
            !attacks(contra, G8)
       )
    {
        assert(piece_on_square(E8) == Piece(BLACK, KING));
        assert(piece_on_square(H8) == Piece(BLACK, ROOK));
        *moves++ = Move::make_castle(CastleKind::BLACK_KING_SIDE);
    }

    if (
            side == WHITE &&
            (castle_kind_allowed(CastleKind::WHITE_QUEEN_SIDE)) &&
            piece_on_square(D1).empty() &&
            piece_on_square(C1).empty() &&
            piece_on_square(B1).empty() &&
            !attacks(contra, E1) &&
            !attacks(contra, D1) &&
            !attacks(contra, C1)
       )
    {
        assert(piece_on_square(E1) == Piece(WHITE, KING));
        assert(piece_on_square(A1) == Piece(WHITE, ROOK));
        *moves++ = Move::make_castle(CastleKind::WHITE_QUEEN_SIDE);
    }

    if (
            side == BLACK &&
            (castle_kind_allowed(CastleKind::BLACK_QUEEN_SIDE)) &&
            piece_on_square(D8).empty() &&
            piece_on_square(C8).empty() &&
            piece_on_square(B8).empty() &&
            !attacks(contra, E8) &&
            !attacks(contra, D8) &&
            !attacks(contra, C8)
       )
    {
        assert(piece_on_square(E8) == Piece(BLACK, KING));
        assert(piece_on_square(A8) == Piece(BLACK, ROOK));
        *moves++ = Move::make_castle(CastleKind::BLACK_QUEEN_SIDE);
    }

    return moves;
}

u64 Position::_generate_checkers(Color side) const noexcept
{
    Color contra = flip_color(side);
    int ksq = _kings[side].value();
    u64 occupied = _occupied();
    u64 king = _kings[contra].mask();
    u64 knights = _bboard(contra, KNIGHT);
    u64 bishops = _bboard(contra, BISHOP);
    u64 rooks =   _bboard(contra, ROOK);
    u64 queens =  _bboard(contra, QUEEN);
    u64 pawns =   _bboard(contra, PAWN);

    u64 rval = 0;
    rval |= rook_attacks(ksq, occupied) & (rooks | queens);
    rval |= bishop_attacks(ksq, occupied) & (bishops | queens);
    rval |= knight_attacks(ksq) & knights;
    rval |= king_attacks(ksq) & king;
    rval |= pawn_attacks(side, ksq) & pawns;
    return rval;
}

std::ostream& operator<<(std::ostream& os, const Position& position)
{
    os << position.dump_fen();
    os << " (";
    os << std::hex << position.zobrist_hash();
    os << ")";
    return os;
}

} // ~namespace lesschess
