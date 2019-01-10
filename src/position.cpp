#include "position.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>

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
    , epsq(Position::ENPASSANT_NONE)
    , castle(Position::CASTLE_NONE)
{
    boards.fill(0ull);
    sidemask.fill(0ull);
    kings.fill(0ull);
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
                Square sq{static_cast<u8>(file), static_cast<u8>(rank)};
                position.sq2p[sq.value()] = piece;
                position.sidemask[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position.kings[piece.color()] = sq.value();
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
            position._set_enpassant_square(square.value() - A3);
        } else if (rank == RANK_6) {
            assert(square.value() >= A6 && square.value() <= H6);
            position._set_enpassant_square(square.value() - A6);
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

    return position;
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

void Position::make_move(Savepos& sp, Move move) noexcept {
    const u8 side = wtm;
    const u8 contra = side ^ 1;
    const Square from = move.from();
    const Square to = move.to();
    const Piece piece = piece_on_square(from);
    const Piece captured = piece_on_square(to);
    const Move::Flags flags = move.flags();
    const PieceKind kind = piece.kind();
    u64* board = kind != KING ? &boards[piece.value()] : 0;

    assert(to != from);
    assert(captured.kind() != KING);
    assert(piece.color() == side);
    assert(captured.empty() || captured.color() == contra);

    sp.halfmoves = halfmoves;
    sp.epsq = epsq;
    sp.castle = castle;
    sp.capture = captured;

    if (flags == Move::Flags::NONE) {
        if (board) {
            *board &= ~from.mask();
            *board |= to.mask();
        } else {
            kings[side] = to.value();
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
            _set_enpassant_square(to.value());
            assert((to.value() >= A3 && to.value() <= H3) || (to.value() >= A6 && to.value() <= H6));
        }

        if (kind == ROOK) {
            castle &= ~rook_square_to_castle_flag(from);
        }
    } else if (flags == Move::Flags::ENPASSANT) {
        const Square target{epsq};
        assert(kind == PAWN);
        assert(captured.empty());
        *board &= ~from.mask();
        *board |= to.mask();
        board[Piece{static_cast<Color>(contra), PAWN}.value()] &= ~target.mask();
        sq2p[target.value()] = NO_PIECE;
        sq2p[from.value()] = NO_PIECE;
        sq2p[to.value()] = piece;
        sidemask[side] &= ~from.mask();
        sidemask[side] |= to.mask();
        sidemask[contra] &= ~target.mask();
    } else if (flags == Move::Flags::PROMOTION) {
        const PieceKind promotion_kind = move.promotion();
        const Piece promotion_piece = Piece{static_cast<Color>(side), promotion_kind};
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
    }
//     } else if (flags == Move::Flags::CASTLE) {
//         assert(kind == KING);
//         int ksq;
//         int rsq;
//         switch (tosq) {
//             case H1:
//                 ksq = G1;
//                 rsq = F1;
//             case A1:
//                 ksq = C1;
//                 rsq = D1;
//             case H8:
//                 ksq = G8;
//                 rsq = F8;
//             case A8:
//                 ksq = C8;
//                 rsq = D8;
//             default:
//                 assert(0);
//                 __builtin_unreachable();
//         }
//         ksqs
//     }
}

void Position::undo_move(const Savepos& sp, Move move) noexcept {

}
