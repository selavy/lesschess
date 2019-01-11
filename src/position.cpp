#include "position.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <cinttypes>

namespace {

Piece translate_fen_piece(char c) {
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
    : moves(1u)
    , halfmoves(0u)
    , wtm(1u)
    , ep_target(Position::ENPASSANT_NONE)
    , castle(Position::CASTLE_NONE)
{
    boards.fill(0ull);
    sidemask.fill(0ull);
    sq2p.fill(NO_PIECE);
}

Position Position::from_fen(std::string_view fen) {
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
                position.sq2p[sq.value()] = piece;
                position.sidemask[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position.kings[piece.color()] = sq;
                } else {
                    position.boards[piece.value()] |= sq.mask();
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
        position._set_castle_flags(Position::CASTLE_NONE);
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
        position._set_castle_flags(flags);
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
            position.halfmoves *= 10;
            position.halfmoves += *it - '0';
            ++it;
        }

        // assuming that if halfmove is there then move spec is also there
        it = expect(it, ' ');
        position.moves = 0;
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid move specification");
            }
            position.moves *= 10;
            position.moves += *it - '0';
            ++it;
        }
    }

    position._validate();
    return position;
}

std::string Position::dump_fen() const noexcept {
    // std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    result += wtm == WHITE ? 'w' : 'b';
    result += ' ';

    if ((castle & Position::CASTLE_WHITE_KING_SIDE) != 0) {
        result += 'K';
    }
    if ((castle & Position::CASTLE_WHITE_QUEEN_SIDE) != 0) {
        result += 'Q';
    }
    if ((castle & Position::CASTLE_BLACK_KING_SIDE) != 0) {
        result += 'k';
    }
    if ((castle & Position::CASTLE_BLACK_QUEEN_SIDE) != 0) {
        result += 'q';
    }
    if (castle == Position::CASTLE_NONE) {
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
constexpr u8 rook_square_to_castle_flag(Square square) noexcept {
    switch (square.value()) {
        case A8: return Position::CASTLE_BLACK_QUEEN_SIDE;
        case H8: return Position::CASTLE_BLACK_KING_SIDE;
        case A1: return Position::CASTLE_WHITE_QUEEN_SIDE;
        case H1: return Position::CASTLE_WHITE_KING_SIDE;
        default: return 0;
    }
}

[[nodiscard]]
constexpr bool is_rank2(u8 side, Square square) noexcept {
    constexpr u64 SECOND_RANK{0xff00ull};
    constexpr u64 SEVENTH_RANK{0xff000000000000ull};
    u64 mask = side == WHITE ? SECOND_RANK : SEVENTH_RANK;
    return square.mask() & mask;
}

[[nodiscard]]
constexpr bool is_rank7(u8 side, Square square) noexcept {
    return is_rank2(side ^ 1, square);
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
    // return std::make_pair(Square(), Square());
}

void Position::make_move(Savepos& sp, Move move) noexcept {
    _validate();

    assert(wtm == WHITE || wtm == BLACK);
    const Color side = static_cast<Color>(wtm);
    const Color contra = static_cast<Color>(side ^ 1);
    const Square from = move.from();
    const Square to = move.to();
    const Piece piece = piece_on_square(from);
    const Piece captured = piece_on_square(to);
    const Move::Flags flags = move.flags();
    const PieceKind kind = piece.kind();
    u64* board = kind != KING ? &boards[piece.value()] : 0;
    Square new_ep_target = Square();

    assert(to != from);
    assert(captured.kind() != KING);
    assert(piece.color() == side);
    assert(captured.empty() || captured.color() == contra || (move.is_castle() && captured.color() == side));

    sp.halfmoves = halfmoves;
    sp.ep_target = ep_target;
    sp.castle = castle;
    sp.captured = captured;

    if (flags == Move::Flags::NONE) {
        if (board) {
            *board &= ~from.mask();
            *board |= to.mask();
        } else {
            kings[side] = to; // to.value();
            if (side == WHITE) {
                castle &= ~Position::CASTLE_WHITE;
            } else {
                castle &= ~Position::CASTLE_BLACK;
            }
        }
        sq2p[from.value()] = NO_PIECE;
        sq2p[to.value()] = piece;
        sidemask[side] &= ~from.mask();
        sidemask[side] |= to.mask();

        if (!captured.empty()) {
            boards[captured.value()] &= ~to.mask();
            sidemask[contra] &= ~to.mask();
            castle &= ~rook_square_to_castle_flag(to);
        } else if (kind == PAWN && is_rank2(side, from) && is_enpassant_square(side, to)) {
            u8 target = side == WHITE ? to.value() - 8 : to.value() + 8;
            new_ep_target = target;
            assert((target >= A3 && target <= H3) || (target >= A6 && target <= H6));
        }

        if (kind == ROOK) {
            castle &= ~rook_square_to_castle_flag(from);
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
        boards[Piece(contra, PAWN).value()] &= ~epsq.mask();
        sq2p[from.value()] = NO_PIECE;
        sq2p[to.value()] = piece;
        sq2p[epsq.value()] = NO_PIECE;
        sidemask[side] &= ~from.mask();
        sidemask[side] |= to.mask();
        sidemask[contra] &= ~epsq.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        const PieceKind promotion_kind = move.promotion();
        const Piece promotion_piece = Piece(side, promotion_kind);
        assert(kind == PAWN);
        *board &= ~from.mask();
        boards[promotion_piece.value()] |= to.mask();
        sq2p[to.value()] = promotion_piece;
        sq2p[from.value()] = NO_PIECE;
        sidemask[side] &= ~from.mask();
        sidemask[side] |= to.mask();
        if (!captured.empty()) {
            boards[captured.value()] &= ~to.mask();
            sidemask[contra] &= ~to.mask();
            castle &= ~rook_square_to_castle_flag(to);
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(kind == KING);
        u64* rooks = &boards[Piece(side, ROOK).value()];
        auto [ksq, rsq] = _get_castle_squares(to);
        kings[side] = ksq;
        *rooks &= ~to.mask();
        *rooks |= rsq.mask();

        sq2p[from.value()] = NO_PIECE;
        sq2p[to.value()] = NO_PIECE;
        sq2p[ksq.value()] = Piece(side, KING);
        sq2p[rsq.value()] = Piece(side, ROOK);
        sidemask[side] &= ~(to.mask() | from.mask());
        sidemask[side] |= ksq.mask() | rsq.mask();
        if (side == WHITE) {
            castle &= ~Position::CASTLE_WHITE;
        } else {
            castle &= ~Position::CASTLE_BLACK;
        }
    } else {
        assert(0);
        __builtin_unreachable();
    }

    _set_enpassant_square(new_ep_target.value());

    wtm = contra;
    if (kind == PAWN || !captured.empty() && !move.is_castle()) {
        halfmoves = 0;
    } else {
        ++halfmoves;
    }
    if (side == BLACK) {
        ++moves;
    }

    _validate();
}

void Position::undo_move(const Savepos& save, Move move) noexcept {
    _validate();
    assert(wtm == WHITE || wtm == BLACK);

    const Color contra = static_cast<Color>(wtm);
    const Color side = static_cast<Color>(contra ^ 1);
    const Square from = move.from();
    const Square to = move.to();
    const Move::Flags flags = move.flags();
    const Piece piece = piece_on_square(to);
    const PieceKind kind = piece.kind();
    u64* board = &boards[piece.value()];
    const Piece captured = save.captured;

    halfmoves = save.halfmoves;
    ep_target = save.ep_target;
    castle = save.castle;
    if (side == BLACK) {
        --moves;
    }
    wtm = side;

    if (flags == Move::Flags::NONE) {
        if (kind != KING) {
            *board |= from.mask();
            *board &= ~to.mask();
        } else {
            kings[side] = from;
        }
        sidemask[side] |= from.mask();
        sidemask[side] &= ~to.mask();
        sq2p[from.value()] = piece;
        sq2p[to.value()] = captured;
        if (!captured.empty()) {
            boards[captured.value()] |= to.mask();
            sidemask[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::CASTLE) {
        assert(move.is_castle());
        Piece rook = Piece(side, ROOK);
        Piece king = Piece(side, KING);
        auto [ksq, rsq] = _get_castle_squares(to);
        sq2p[from.value()] = king;
        sq2p[to.value()] = rook;
        sq2p[ksq.value()] = NO_PIECE;
        sq2p[rsq.value()] = NO_PIECE;
        sidemask[side] |= to.mask() | from.mask();
        sidemask[side] &= ~(ksq.mask() | rsq.mask());
        kings[side] = from;
        // TODO(peter): verify only doing 1 load here
        boards[rook.value()] &= ~rsq.mask();
        boards[rook.value()] |= to.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        assert(move.is_promotion());
        Piece pawn = Piece(side, PAWN);
        Piece promoted = Piece(side, move.promotion());
        boards[pawn.value()] |= from.mask();
        boards[promoted.value()] &= ~to.mask();
        sq2p[to.value()] = captured;
        sq2p[from.value()] = pawn;
        sidemask[side] |= from.mask();
        sidemask[side] &= ~to.mask();
        if (!captured.empty()) {
            boards[captured.value()] |= to.mask();
            sidemask[contra] |= to.mask();
        }
    } else if (flags == Move::Flags::ENPASSANT) {
        // TODO(peter): better name for :epsq:
        Square epsq = side == WHITE ? to.value() - 8 : to.value() + 8;
        Piece opp_pawn = Piece(contra, PAWN);
        sq2p[from.value()] = piece;
        *board |= from.mask();
        *board &= ~to.mask();
        sidemask[side] |= from.mask();
        sidemask[side] &= ~to.mask();
        sidemask[contra] |= epsq.mask();
        sq2p[to.value()] = NO_PIECE;
        sq2p[epsq.value()] = opp_pawn;
        boards[opp_pawn.value()] |= epsq.mask();
    } else {
        assert(0);
        __builtin_unreachable();
    }

    // const Color contra = static_cast<Color>(wtm);
    // const Color side = static_cast<Color>(contra ^ 1);
    // const Square from = move.from();
    // const Square to = move.to();
    // const Piece piece = piece_on_square(to);
    // const Piece captured = sp.captured;


    // const uint8_t side = FLIP(pos->wtm);
    // const uint8_t contra = pos->wtm;
    // const uint32_t fromsq = FROM(m);
    // const uint32_t tosq = TO(m);
    // const uint32_t promo = PROMO_PC(m);
    // const uint32_t promopc = PIECE(side, promo);
    // const uint32_t flags = FLAGS(m);
    // const uint32_t pc = pos->sqtopc[tosq];
    // const uint32_t cappc = sp->captured_pc;
    // const uint64_t from = MASK(fromsq);
    // const uint64_t to = MASK(tosq); // REVISIT: all uses of `to' are `~to' so just calculate that?
    // const uint32_t epsq = side == WHITE ? tosq - 8 : tosq + 8;
    // uint64_t *restrict pcs = pc != PIECE(side, KING) ? &pos->brd[pc] : 0;
    // uint8_t *restrict s2p = pos->sqtopc;
    // uint64_t *restrict rooks = &pos->brd[PIECE(side, ROOK)];
    // uint64_t *restrict sidebb = &pos->side[side];
    // uint64_t *restrict contrabb = &pos->side[contra];
    // int ksq;
    // int rsq;

    // assert(validate_position(pos) == 0);
    // assert(fromsq >= A1 && fromsq <= H8);
    // assert(tosq >= A1 && tosq <= H8);
    // assert(side == WHITE || side == BLACK);
    // assert(flags == FLG_NONE || flags == FLG_EP || flags == FLG_PROMO || flags == FLG_CASTLE);
    // assert(cappc == EMPTY || (cappc >= PIECE(WHITE, KNIGHT) && cappc <= PIECE(BLACK, KING)));
    // assert(flags != FLG_PROMO || (promopc >= PIECE(side, KNIGHT) && promopc <= PIECE(side, KING)));

    // pos->halfmoves = sp->halfmoves;

    // pos->enpassant = sp->enpassant;
    // // reset castling flags
    // pos->castle = sp->castle;

    // // reset side to move
    // pos->wtm = side;

    // --pos->nmoves;

    // switch (flags) {
    // case FLG_NONE:
    //     s2p[fromsq] = pc;
    //     if (pcs) {
    //         *pcs |= from;
    //         *pcs &= ~to;
    //     } else {
    //         KSQ(*pos, side) = fromsq;
    //     }
    //     *sidebb |= from;
    //     *sidebb &= ~to;
    //     s2p[tosq] = cappc;
    //     if (cappc != EMPTY) {
    //         pos->brd[cappc] |= to;
    //         *contrabb |= to;
    //     }
    //     break;
    // case FLG_EP:
    //     s2p[fromsq] = pc;
    //     *pcs |= from;
    //     *pcs &= ~to;
    //     *sidebb |= from;
    //     *sidebb &= ~to;
    //     *contrabb |= MASK(epsq);
    //     s2p[tosq] = EMPTY;
    //     assert((side == WHITE && epsq == (tosq - 8)) || (side == BLACK && epsq == (tosq + 8)));
    //     s2p[epsq] = PIECE(contra, PAWN);
    //     pos->brd[PIECE(contra, PAWN)] |= MASK(epsq);
    //     break;
    // case FLG_PROMO:
    //     pos->brd[PIECE(side, PAWN)] |= from;
    //     pos->brd[promopc] &= ~to;
    //     s2p[tosq] = cappc;
    //     s2p[fromsq] = PIECE(side, PAWN);
    //     *sidebb |= from;
    //     *sidebb &= ~to;
    //     if (cappc != EMPTY) {
    //         pos->brd[cappc] |= to;
    //         *contrabb |= to;
    //     }
    //     break;
    // case FLG_CASTLE:
    //     assert(cappc == PIECE(side, ROOK));
    //     switch (tosq) {
    //     case A1:
    //         ksq = C1;
    //         rsq = D1;
    //         break;
    //     case H1:
    //         ksq = G1;
    //         rsq = F1;
    //         break;
    //     case A8:
    //         ksq = C8;
    //         rsq = D8;
    //         break;
    //     case H8:
    //         ksq = G8;
    //         rsq = F8;
    //         break;
    //     default:
    //         unreachable();
    //         break;
    //     }
    //     s2p[fromsq] = PIECE(side, KING);
    //     s2p[tosq] = PIECE(side, ROOK);
    //     s2p[ksq] = EMPTY;
    //     s2p[rsq] = EMPTY;
    //     *sidebb |= to | from;
    //     *sidebb &= ~(MASK(ksq) | MASK(rsq));
    //     *rooks &= ~MASK(rsq);
    //     *rooks |= to;
    //     KSQ(*pos, side) = fromsq;
    //     break;
    // default:
    //     unreachable();
    // }

    // assert(validate_position(pos) == 0);
    _validate();
}

bool Position::operator==(const Position& rhs) const noexcept {
    const Position& lhs = *this;
    return ((lhs.boards == rhs.boards) &&
            (lhs.sidemask == rhs.sidemask) &&
            (lhs.sq2p == rhs.sq2p) &&
            (lhs.kings == rhs.kings) &&
            (lhs.moves == rhs.moves) &&
            (lhs.halfmoves == rhs.halfmoves) &&
            (lhs.wtm == rhs.wtm) &&
            (lhs.ep_target == rhs.ep_target) &&
            (lhs.castle == rhs.castle));
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

    // Verify sidemasks
    u64 white_mask = 0ull;
    u64 black_mask = 0ull;
    for (auto piece: pieces) {
        u64 piece_mask = piece.kind() == KING ?  kings[piece.color()].mask() : boards[piece.value()];
        assert((white_mask & piece_mask) == 0ull);
        assert((black_mask & piece_mask) == 0ull);
        if (piece.color() == WHITE) {
            white_mask |= piece_mask;
        } else {
            black_mask |= piece_mask;
        }
    }
    u64 full_mask = white_mask | black_mask;
    if (sidemask[BLACK] != black_mask) {
        printf("sidemask   = %" PRIu64 "\n", sidemask[BLACK]);
        printf("black_mask = %" PRIu64 "\n", black_mask);
        printf("side - black = %" PRIu64 "\n", sidemask[BLACK] & ~black_mask);
        printf("black - side = %" PRIu64 "\n", black_mask & ~sidemask[BLACK]);
    }
    assert(sidemask[WHITE] == white_mask);
    assert(sidemask[BLACK] == black_mask);
    assert((white_mask & black_mask) == 0ull); // no bits should overlap

    // Verify sq2p == boards
    for (int i = 0; i < 64; ++i) {
        Piece piece = sq2p[i];
        Square square(i);
        if (piece.empty()) {
            assert((white_mask & square.mask()) == 0ull);
            assert((black_mask & square.mask()) == 0ull);
        } else if (piece.kind() == KING){
            assert(kings[piece.color()] == square);
        } else {
            assert((boards[piece.value()] & square.mask()) != 0);
        }
    }

    std::array<int, 14> counts;
    counts.fill(0);
    for (auto&& piece: sq2p) {
        ++counts[piece.value()];
    }
    assert(counts[Piece(WHITE, KING).value()] == 1);
    assert(counts[Piece(BLACK, KING).value()] == 1);
    assert(counts[Piece(WHITE, PAWN).value()] <= 8);
    assert(counts[Piece(BLACK, PAWN).value()] <= 8);
#endif
}


