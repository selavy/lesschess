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
    sp.capture = captured;

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
        const Square target = enpassant_target_square();
        assert(kind == PAWN);
        assert(captured.empty());
        *board &= ~from.mask();
        *board |= to.mask();
        board[Piece(contra, PAWN).value()] &= ~target.mask();
        sq2p[target.value()] = NO_PIECE;
        sq2p[from.value()] = NO_PIECE;
        sq2p[to.value()] = piece;
        sidemask[side] &= ~from.mask();
        sidemask[side] |= to.mask();
        sidemask[contra] &= ~target.mask();
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
        Square ksq;
        Square rsq;
        switch (to.value()) {
            case H1:
                ksq = Square(G1);
                rsq = Square(F1);
                break;
            case A1:
                ksq = Square(C1);
                rsq = Square(D1);
                break;
            case H8:
                ksq = Square(G8);
                rsq = Square(F8);
                break;
            case A8:
                ksq = Square(C8);
                rsq = Square(D8);
                break;
            default:
                assert(0 && "invalid castle target square");
                __builtin_unreachable();
        }
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

void Position::undo_move(const Savepos& sp, Move move) noexcept {

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

#include <iostream>

void board_compare(u64 lhs, u64 rhs, const char *left, const char *right) {
    for (u64 i = 0; i < 64; ++i) {
        u64 mask = 1ull << i;
        bool l = (lhs & mask) != 0;
        bool r = (rhs & mask) != 0;
        if (l && l == r) {
            std::cout << Square(i).name() << " in both\n";
            continue;
        } else if (l) {
            std::cout << left << " has " << Square(i).name() << "\n";
        } else if (r) {
            std::cout << right << " has " << Square(i).name() << "\n";
        }
    }
}

void Position::_validate() const noexcept {
#ifndef NDEBUG
    std::array<Color, 2> colors = { WHITE, BLACK };
    std::array<PieceKind, 6> kinds = { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

    // Verify sidemasks
    u64 white_mask = 0ull;
    for (auto kind: kinds) {
        u64 piece_mask;
        if (kind == KING) {
            piece_mask = kings[WHITE].mask();
        } else {
            piece_mask = boards[Piece(WHITE, kind).value()];
        }
        assert((white_mask & piece_mask) == 0ull);
        white_mask |= piece_mask;
    }
    u64 black_mask = 0ull;
    for (auto kind: kinds) {
        u64 piece_mask;
        if (kind == KING) {
            piece_mask = kings[BLACK].mask();
        } else {
            piece_mask = boards[Piece(BLACK, kind).value()];
        }

        bool cond = (black_mask & piece_mask) == 0ull;
        // if ((black_mask & piece_mask) != 0ull) {
        if (cond == false) {
            board_compare(black_mask, piece_mask, "black_mask", "piece_mask");
        }

        // assert((black_mask & piece_mask) == 0ull);
        assert(cond == true);
        black_mask |= piece_mask;
    }
    u64 full_mask = white_mask | black_mask;
    assert(sidemask[WHITE] == white_mask);
    assert(sidemask[BLACK] == black_mask);
    assert((white_mask & black_mask) == 0ull); // no bits should overlap

//     for (int i = 0; i < sq2p.size(); ++i) {
//         Piece piece = sq2p[i];
//         Square square(i);
// 
// 
//     }
//     for (auto&& piece: sq2p) {
//         if (piece.empty()) {
//             assert(full_mask)
//         }
//     }


    // std::array<Color, 2> colors = { WHITE, BLACK };
    // std::array<PieceKind, 5> kinds = { PAWN, KNIGHT, BISHOP, ROOK, QUEEN };
    // for (auto&& color: colors) {
    //     for (auto&& kind: kinds) {
    //         Piece piece(color, kind);
    //         u64 board = boards[piece.value()];
    //         for (int i = 0; i < 64; ++i) {
    //             Square sq(static_cast<u8>(i));
    //             bool bitboard_has_piece = (sq.mask() & board) != 0;
    //             bool sq2p_has_piece = piece_on_square(sq) == piece;
    //             assert(bitboard_has_piece == sq2p_has_piece);
    //         }
    //     }
    // }

    // std::array<int, 13> counts;
    // counts.fill(0);
    // for (auto&& piece: sq2p) {
    //     ++counts[piece.value()];
    // }
    // assert(counts[Piece(WHITE, KING).value()] == 1);
    // assert(counts[Piece(BLACK, KING).value()] == 1);
    // assert(counts[Piece(WHITE, PAWN).value()] <= 8);
    // assert(counts[Piece(BLACK, PAWN).value()] <= 8);
#endif
}


