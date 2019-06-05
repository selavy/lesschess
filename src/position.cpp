#include "position.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <cinttypes>
#include "detail/magic_tables.generated.h"

namespace lesschess {

[[nodiscard]]
constexpr u64 lsb(u64 x) noexcept { return __builtin_ctzll(x); }

[[nodiscard]]
constexpr int popcountll(u64 x) noexcept { return __builtin_popcountll(x); }

[[nodiscard]]
constexpr u64 clear_lsb(u64 b) noexcept { return b & (b - 1); }

[[nodiscard]]
constexpr bool is_power_of_two(u64 x) noexcept { return x & (x - 1); }

[[nodiscard]]
constexpr bool more_than_one_piece(u64 x) noexcept { return is_power_of_two(x); }

namespace
{

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


// [[nodiscard]] Move*
// generate_knight_moves(u64 knights, const u64 targets, Move* moves) noexcept
// {
//     int from;
//     int to;
//     uint64_t posmoves;
//     while (knights) {
//         from = lsb(knights);
//         posmoves = knight_attacks(from) & targets;
//         while (posmoves) {
//             to = lsb(posmoves);
//             *moves++ = Move(from, to);
//             posmoves = clear_lsb(posmoves);
//         }
//         knights = clear_lsb(knights);
//     }
//     return moves;
// }

Position::Position() noexcept
    : _moves(1u)
    , _halfmoves(0u)
    , _wtm(1u)
    , _ep_target(Position::ENPASSANT_NONE)
    , _castle_rights(Position::CASTLE_NONE)
{
    _boards.fill(0ull);
    _sidemask.fill(0ull);
    _sq2pc.fill(NO_PIECE);
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
        position._castle_rights = Position::CASTLE_NONE;
        ++it;
    } else {
        u8 flags = 0;
        while (it < last && *it != ' ') {
            char c = *it++;
            switch (c) {
            case 'K':
                flags |= Position::CASTLE_WHITE_KING_SIDE;
                break;
            case 'k':
                flags |= Position::CASTLE_BLACK_KING_SIDE;
                break;
            case 'Q':
                flags |= Position::CASTLE_WHITE_QUEEN_SIDE;
                break;
            case 'q':
                flags |= Position::CASTLE_BLACK_QUEEN_SIDE;
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

    expect('\n');
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

    position._validate();
    return position;
}

constexpr PieceKind piece_kind_from_san(char c) noexcept
{
    switch (c) {
        case 'K': case 'k': return KING;
        case 'Q': case 'q': return QUEEN;
        case 'B': case 'b': return BISHOP;
        case 'N': case 'n': return KNIGHT;
        default: return PAWN;
    }
}

// Move Position::move_from_san(std::string_view san) const {
//     Move result;
//     PieceKind kind;
//     int file;
//     int rank;
// 
//     // Supporting:
//     //   normal move:  [KNBRQknbrq]([a-h]|[0-9])?[a-h][0-9]
//     //   pawn move:    (Pp)?[a-h][0-9]
//     //   kside castle: O-O|o-o|0-0
//     //   qside castle: O-O-O|o-o-o|0-0-0
//     //   promotion:    (Pp)?[a-h][0-9]=[
// 
//     // Check if has piece: K, N, B, R, Q, P
//     auto len = san.length();
//     if (len < 2) {
//         throw std::runtime_error("too short");
//     } else if (san == "O-O") {
//         // king side castle
//         auto castle_kind = white_to_move() ?
//             Move::CastleKind::WHITE_KING_SIDE :
//             Move::CastleKind::BLACK_KING_SIDE ;
//         return Move::make_castle_move(castle_kind);
//     } else if (san == "O-O-O") {
//         // queen side castle
//         auto castle_kind = white_to_move() ?
//             Move::CastleKind::WHITE_QUEEN_SIDE :
//             Move::CastleKind::BLACK_QUEEN_SIDE ;
//         return Move::make_castle_move(castle_kind);
//     } else if (len == 2) {
//         // pawn move
//         if (!(san[0] >= 'a' && san[0] <= 'h')) {
//             throw std::runtime_error("invalid file");
//         }
//         if (!(san[1] >= '1' && san[1] <= '8')) {
//             throw std::runtime_error("invalid rank");
//         }
//         file = san[0] - 'a';
//         rank = san[1] - '1';
//         kind = PAWN;
//     } else if (len == 3 && san[1] == 'x') {
// 
//     }
// 
// 
//     return result;
// }

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

    if ((_castle_rights & Position::CASTLE_WHITE_KING_SIDE) != 0) {
        result += 'K';
    }
    if ((_castle_rights & Position::CASTLE_WHITE_QUEEN_SIDE) != 0) {
        result += 'Q';
    }
    if ((_castle_rights & Position::CASTLE_BLACK_KING_SIDE) != 0) {
        result += 'k';
    }
    if ((_castle_rights & Position::CASTLE_BLACK_QUEEN_SIDE) != 0) {
        result += 'q';
    }
    if (_castle_rights == Position::CASTLE_NONE) {
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
    std::string result = "\n|";
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
        case A8: return Position::CASTLE_BLACK_QUEEN_SIDE;
        case H8: return Position::CASTLE_BLACK_KING_SIDE;
        case A1: return Position::CASTLE_WHITE_QUEEN_SIDE;
        case H1: return Position::CASTLE_WHITE_KING_SIDE;
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

    if (flags == Move::Flags::NONE) {
        if (board) {
            *board &= ~from.mask();
            *board |= to.mask();
        } else {
            _kings[side] = to; // to.value();
            if (side == WHITE) {
                _castle_rights &= ~Position::CASTLE_WHITE;
            } else {
                _castle_rights &= ~Position::CASTLE_BLACK;
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
        const Square epsq = side == WHITE ? to.value() - 8 : to.value() + 8;
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
            _castle_rights &= ~Position::CASTLE_WHITE;
        } else {
            _castle_rights &= ~Position::CASTLE_BLACK;
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
            (lhs._halfmoves == rhs._halfmoves) &&
            (lhs._wtm == rhs._wtm) &&
            (lhs._ep_target == rhs._ep_target) &&
            (lhs._castle_rights == rhs._castle_rights));
}

bool Position::operator!=(const Position& rhs) const noexcept {
    return !(*this == rhs);
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
    if (_sidemask[BLACK] != black_mask) {
        printf("_sidemask   = %" PRIu64 "\n", _sidemask[BLACK]);
        printf("black_mask = %" PRIu64 "\n", black_mask);
        printf("side - black = %" PRIu64 "\n", _sidemask[BLACK] & ~black_mask);
        printf("black - side = %" PRIu64 "\n", black_mask & ~_sidemask[BLACK]);
    }
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
        case Move::CastleKind::WHITE_KING_SIDE:  return G1;
        case Move::CastleKind::BLACK_KING_SIDE:  return G8;
        case Move::CastleKind::WHITE_QUEEN_SIDE: return C1;
        case Move::CastleKind::BLACK_QUEEN_SIDE: return C8;
    }
    __builtin_unreachable();
}

bool Position::is_legal_move(Move move) const noexcept
{
    Color side = wtm();
    Color contra = flip_color(side);
    Square ksq = _kings[side];
    Square tosq = move.to();
    Square frsq = move.from();
    Piece topc = piece_on_square(tosq);
    Piece frpc = piece_on_square(frsq);
    u64 pinned = _generate_pinned(side, side);

    // must move own piece
    if (frpc.empty() || frpc.color() != side) {
        return false;
    }

    // can't capture own piece
    if (!topc.empty() && topc.color() == side) {
        return false;
    }

    if (move.is_enpassant()) {
        // legal if
        //   + after making the move there are no attacks on the king
        auto capture_sq = Square(side == WHITE ? tosq.value() - 8 : tosq.value() + 8);
        u64 prev_occupied = _occupied();
        u64 queens = _bboard(contra, QUEEN);
        u64 rooks = _bboard(contra, ROOK);
        u64 bishops = _bboard(contra, BISHOP);
        u64 occupied = (prev_occupied ^ frsq.mask() ^ capture_sq.mask()) | tosq.mask();
        u64 straight_attacks = rook_attacks(ksq.value(), occupied) & (queens | rooks);
        u64 diagonal_attacks = bishop_attacks(ksq.value(), occupied) & (queens | bishops);
        return (straight_attacks | diagonal_attacks) == 0;
    }

    if (move.is_castle()) {
        // legal if:
        //   + castling right existed before move
        //   + no pieces between king and rook
        //   + doesn't castle into check
        //   + doesn't castle out of check
        //   + doesn't castle through check

        if (!castle_kind_allowed(move.castle_kind())) {
            return false;
        }

        Color contra = flip_color(side);
        int from = frsq.value();
        int to   = castles_king_square(move);
        int step = to > from ? 1 : -1;
        for (int sq = to; sq != from; sq += step) {
            if (attacks(contra, Square(sq))) {
                return false;
            }
            if (sq != to && sq != from && !piece_on_square(Square(sq)).empty()) {
                return false;
            }
        }

        return true;
    }

    // TODO: finish
    return true;
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


bool Position::_is_legal(u64 pinned, Move m) const noexcept
{
    return false;
#if 0
    Color side = Color(_wtm);
    Color contra = flip_color(side);
    Piece piece = piece_on_square(m.from());
    Square tosq = m.to();
    Square frsq = m.from();
    Square ksq = _kings[side];

    if (m.is_castle()) {
        return true;
    } else if (m.is_enpassant()){
        auto capture_sq = Square(side == WHITE ? tosq.value() - 8 : tosq.value() + 8);
        u64 prev_occupied = _occupied();
        u64 queens = _bboard(contra, QUEEN);
        u64 rooks = _bboard(contra, ROOK);
        u64 bishops = _bboard(contra, BISHOP);
        u64 occupied = (prev_occupied ^ frsq.mask() ^ capture_sq.mask()) | tosq.mask();
        u64 straight_attacks = rook_attacks(ksq.value(), occupied) & (queens | rooks);
        u64 diagonal_attacks = bishop_attacks(ksq.value(), occupied) & (queens | bishops);
        return (straight_attacks | diagonal_attacks) == 0;
    } else if (piece.kind() == KING){
        return !attacks(contra, m.to());
    } else {
        // legal if either:
        //   + nothing is pinned
        //   + piece being moved isn't a pinned piece
        //   + moving on the same ray as the king OR
        //   + will still be blocking after moving
        return !pinned ||    // REVISIT: is it worth checking this?
               (pinned & frsq.mask()) == 0 ||
               lined_up(frsq.value(), tosq.value(), ksq.value());
    }
#endif
}

} // ~namespace lesschess
