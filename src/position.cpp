#include "position.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <cinttypes>

namespace lesschess {

// #define lsb __builtin_ctzll
// #define popcountll __builtin_popcountll
// 
// [[nodiscard]]
// constexpr u64 clear_lsb(u64 b) noexcept {
//     return b & (b - 1);
// }
// 
// [[nodiscard]]
// constexpr bool is_power_of_two(u64 b) noexcept {
//     return b & (b - 1);
// }
// 
// [[nodiscard]]
// constexpr bool more_than_one_piece(u64 b) noexcept {
//     return is_power_of_two(b);
// }


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
    : moves_(1u)
    , halfmoves_(0u)
    , wtm_(1u)
    , ep_target_(Position::ENPASSANT_NONE)
    , castle_rights_(Position::CASTLE_NONE)
{
    boards_.fill(0ull);
    sidemask_.fill(0ull);
    sq2pc_.fill(NO_PIECE);
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
                position.sq2pc_[sq.value()] = piece;
                position.sidemask_[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position.kings_[piece.color()] = sq;
                } else {
                    position.boards_[piece.value()] |= sq.mask();
                }
            }
        }
        if (it < last && *it == '/') {
            ++it;
        }
    }
    it = expect(it, ' ');

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
    it = expect(it, ' ');

    // Castling Availability
    if (it == last) {
        throw std::runtime_error("Expected castling availability specification");
    } else if (*it == '-') {
        position.castle_rights_ = Position::CASTLE_NONE;
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
        position.castle_rights_ = flags;
    }
    it = expect(it, ' ');

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
            position.halfmoves_ *= 10;
            position.halfmoves_ += *it - '0';
            ++it;
        }

        // assuming that if halfmove is there then move spec is also there
        it = expect(it, ' ');
        position.moves_ = 0;
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid move specification");
            }
            position.moves_ *= 10;
            position.moves_ += *it - '0';
            ++it;
        }
    }

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
    result += wtm_ == WHITE ? 'w' : 'b';
    result += ' ';

    if ((castle_rights_ & Position::CASTLE_WHITE_KING_SIDE) != 0) {
        result += 'K';
    }
    if ((castle_rights_ & Position::CASTLE_WHITE_QUEEN_SIDE) != 0) {
        result += 'Q';
    }
    if ((castle_rights_ & Position::CASTLE_BLACK_KING_SIDE) != 0) {
        result += 'k';
    }
    if ((castle_rights_ & Position::CASTLE_BLACK_QUEEN_SIDE) != 0) {
        result += 'q';
    }
    if (castle_rights_ == Position::CASTLE_NONE) {
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

    assert(wtm_ == WHITE || wtm_ == BLACK);
    const Color side = static_cast<Color>(wtm_);
    const Color contra = flip_color(side);
    const Square from = move.from();
    const Square to = move.to();
    const Piece piece = piece_on_square(from);
    const Piece captured = piece_on_square(to);
    const Move::Flags flags = move.flags();
    const PieceKind kind = piece.kind();
    u64* board = kind != KING ? &boards_[piece.value()] : 0;
    Square new_ep_target = Square();

    assert(to != from);
    assert(captured.kind() != KING);
    assert(piece.color() == side);
    assert(captured.empty() || captured.color() == contra || (move.is_castle() && captured.color() == side));

    sp.halfmoves = halfmoves_;
    sp.ep_target = ep_target_;
    sp.castle_rights = castle_rights_;
    sp.captured = captured;

    if (flags == Move::Flags::NONE) {
        if (board) {
            *board &= ~from.mask();
            *board |= to.mask();
        } else {
            kings_[side] = to; // to.value();
            if (side == WHITE) {
                castle_rights_ &= ~Position::CASTLE_WHITE;
            } else {
                castle_rights_ &= ~Position::CASTLE_BLACK;
            }
        }
        sq2pc_[from.value()] = NO_PIECE;
        sq2pc_[to.value()] = piece;
        sidemask_[side] &= ~from.mask();
        sidemask_[side] |= to.mask();

        if (!captured.empty()) {
            boards_[captured.value()] &= ~to.mask();
            sidemask_[contra] &= ~to.mask();
            castle_rights_ &= ~rook_square_to_castle_flag(to);
        } else if (kind == PAWN && is_rank2(side, from) && is_enpassant_square(side, to)) {
            u8 target = side == WHITE ? to.value() - 8 : to.value() + 8;
            new_ep_target = target;
            assert((target >= A3 && target <= H3) || (target >= A6 && target <= H6));
        }

        if (kind == ROOK) {
            castle_rights_ &= ~rook_square_to_castle_flag(from);
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
        boards_[Piece(contra, PAWN).value()] &= ~epsq.mask();
        sq2pc_[from.value()] = NO_PIECE;
        sq2pc_[to.value()] = piece;
        sq2pc_[epsq.value()] = NO_PIECE;
        sidemask_[side] &= ~from.mask();
        sidemask_[side] |= to.mask();
        sidemask_[contra] &= ~epsq.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        const PieceKind promotion_kind = move.promotion();
        const Piece promotion_piece = Piece(side, promotion_kind);
        assert(kind == PAWN);
        *board &= ~from.mask();
        boards_[promotion_piece.value()] |= to.mask();
        sq2pc_[to.value()] = promotion_piece;
        sq2pc_[from.value()] = NO_PIECE;
        sidemask_[side] &= ~from.mask();
        sidemask_[side] |= to.mask();
        if (!captured.empty()) {
            boards_[captured.value()] &= ~to.mask();
            sidemask_[contra] &= ~to.mask();
            castle_rights_ &= ~rook_square_to_castle_flag(to);
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(kind == KING);
        u64* rooks = &boards_[Piece(side, ROOK).value()];
        auto [ksq, rsq] = _get_castle_squares(to);
        kings_[side] = ksq;
        *rooks &= ~to.mask();
        *rooks |= rsq.mask();

        sq2pc_[from.value()] = NO_PIECE;
        sq2pc_[to.value()] = NO_PIECE;
        sq2pc_[ksq.value()] = Piece(side, KING);
        sq2pc_[rsq.value()] = Piece(side, ROOK);
        sidemask_[side] &= ~(to.mask() | from.mask());
        sidemask_[side] |= ksq.mask() | rsq.mask();
        if (side == WHITE) {
            castle_rights_ &= ~Position::CASTLE_WHITE;
        } else {
            castle_rights_ &= ~Position::CASTLE_BLACK;
        }
    } else {
        assert(0);
        __builtin_unreachable();
    }

    _set_enpassant_square(new_ep_target.value());

    wtm_ = contra;
    if (kind == PAWN || !captured.empty() && !move.is_castle()) {
        halfmoves_ = 0;
    } else {
        ++halfmoves_;
    }
    if (side == BLACK) {
        ++moves_;
    }

    _validate();
}

void Position::undo_move(const Savepos& save, Move move) noexcept {
    _validate();
    assert(wtm_ == WHITE || wtm_ == BLACK);

    const Color contra = static_cast<Color>(wtm_);
    const Color side = flip_color(contra);
    const Square from = move.from();
    const Square to = move.to();
    const Move::Flags flags = move.flags();
    const Piece piece = piece_on_square(to);
    const PieceKind kind = piece.kind();
    u64* board = &boards_[piece.value()];
    const Piece captured = save.captured;

    halfmoves_ = save.halfmoves;
    ep_target_ = save.ep_target;
    castle_rights_ = save.castle_rights;
    if (side == BLACK) {
        --moves_;
    }
    wtm_ = side;

    if (flags == Move::Flags::NONE) {
        if (kind != KING) {
            *board |= from.mask();
            *board &= ~to.mask();
        } else {
            kings_[side] = from;
        }
        sidemask_[side] |= from.mask();
        sidemask_[side] &= ~to.mask();
        sq2pc_[from.value()] = piece;
        sq2pc_[to.value()] = captured;
        if (!captured.empty()) {
            boards_[captured.value()] |= to.mask();
            sidemask_[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(move.is_castle());
        Piece rook = Piece(side, ROOK);
        Piece king = Piece(side, KING);
        auto [ksq, rsq] = _get_castle_squares(to);
        sq2pc_[from.value()] = king;
        sq2pc_[to.value()] = rook;
        sq2pc_[ksq.value()] = NO_PIECE;
        sq2pc_[rsq.value()] = NO_PIECE;
        sidemask_[side] |= to.mask() | from.mask();
        sidemask_[side] &= ~(ksq.mask() | rsq.mask());
        kings_[side] = from;
        // TODO(peter): verify only doing 1 load here
        boards_[rook.value()] &= ~rsq.mask();
        boards_[rook.value()] |= to.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        assert(move.is_promotion());
        Piece pawn = Piece(side, PAWN);
        Piece promoted = Piece(side, move.promotion());
        boards_[pawn.value()] |= from.mask();
        boards_[promoted.value()] &= ~to.mask();
        sq2pc_[to.value()] = captured;
        sq2pc_[from.value()] = pawn;
        sidemask_[side] |= from.mask();
        sidemask_[side] &= ~to.mask();
        if (!captured.empty()) {
            boards_[captured.value()] |= to.mask();
            sidemask_[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::ENPASSANT) {
        // TODO(peter): better name for :epsq:
        Square epsq = side == WHITE ? to.value() - 8 : to.value() + 8;
        Piece opp_pawn = Piece(contra, PAWN);
        sq2pc_[from.value()] = piece;
        *board |= from.mask();
        *board &= ~to.mask();
        sidemask_[side] |= from.mask();
        sidemask_[side] &= ~to.mask();
        sidemask_[contra] |= epsq.mask();
        sq2pc_[to.value()] = NO_PIECE;
        sq2pc_[epsq.value()] = opp_pawn;
        boards_[opp_pawn.value()] |= epsq.mask();
    } else {
        assert(0);
        __builtin_unreachable();
    }

    _validate();
}

bool Position::operator==(const Position& rhs) const noexcept {
    const Position& lhs = *this;
    return ((lhs.boards_ == rhs.boards_) &&
            (lhs.sidemask_ == rhs.sidemask_) &&
            (lhs.sq2pc_ == rhs.sq2pc_) &&
            (lhs.kings_ == rhs.kings_) &&
            (lhs.moves_ == rhs.moves_) &&
            (lhs.halfmoves_ == rhs.halfmoves_) &&
            (lhs.wtm_ == rhs.wtm_) &&
            (lhs.ep_target_ == rhs.ep_target_) &&
            (lhs.castle_rights_ == rhs.castle_rights_));
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

    // Verify sidemask_s
    u64 white_mask = 0ull;
    u64 black_mask = 0ull;
    for (auto piece: pieces) {
        u64 piece_mask = piece.kind() == KING ?  kings_[piece.color()].mask() : boards_[piece.value()];
        assert((white_mask & piece_mask) == 0ull);
        assert((black_mask & piece_mask) == 0ull);
        if (piece.color() == WHITE) {
            white_mask |= piece_mask;
        } else {
            black_mask |= piece_mask;
        }
    }
    u64 full_mask = white_mask | black_mask;
    if (sidemask_[BLACK] != black_mask) {
        printf("sidemask_   = %" PRIu64 "\n", sidemask_[BLACK]);
        printf("black_mask = %" PRIu64 "\n", black_mask);
        printf("side - black = %" PRIu64 "\n", sidemask_[BLACK] & ~black_mask);
        printf("black - side = %" PRIu64 "\n", black_mask & ~sidemask_[BLACK]);
    }
    assert(sidemask_[WHITE] == white_mask);
    assert(sidemask_[BLACK] == black_mask);
    assert((white_mask & black_mask) == 0ull); // no bits should overlap

    // Verify sq2pc_ == boards_
    for (int i = 0; i < 64; ++i) {
        Piece piece = sq2pc_[i];
        Square square(i);
        if (piece.empty()) {
            assert((white_mask & square.mask()) == 0ull);
            assert((black_mask & square.mask()) == 0ull);
        } else if (piece.kind() == KING){
            assert(kings_[piece.color()] == square);
        } else {
            assert((boards_[piece.value()] & square.mask()) != 0);
        }
    }

    std::array<int, 14> counts;
    counts.fill(0);
    for (auto&& piece: sq2pc_) {
        ++counts[piece.value()];
    }
    assert(counts[Piece(WHITE, KING).value()] == 1);
    assert(counts[Piece(BLACK, KING).value()] == 1);
    assert(counts[Piece(WHITE, PAWN).value()] <= 8);
    assert(counts[Piece(BLACK, PAWN).value()] <= 8);
#endif
}

} // ~namespace lesschess
