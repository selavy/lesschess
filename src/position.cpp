#include "position.h"
#include <cstring>
#include <cstdlib>

static std::optional<ColorPiece> translate_fen_piece(char c) noexcept {
    Color color = std::isupper(c) ? WHITE : BLACK;
    c = std::tolower(c);
    Piece piece;
    switch (c) {
        case 'p': piece = PAWN; break;
        case 'n': piece = KNIGHT; break;
        case 'b': piece = BISHOP; break;
        case 'r': piece = ROOK; break;
        case 'q': piece = QUEEN; break;
        case 'k': piece = KING; break;
        default: return std::nullopt;
    }
    return ColorPiece{color, piece};
}

std::string_view::iterator
parse_fen_board(std::string_view::iterator first,
                std::string_view::iterator last,
                Position& pos) noexcept
{
    memset(&pos.bbrd_[0], 0, sizeof(pos.bbrd_));
    memset(&pos.sq2p_[0], 0, sizeof(pos.sq2p_));
    memset(&pos.side_[0], 0, sizeof(pos.side_));
    for (u8 rank = 7; rank >= 0; --rank) {
        for (u8 file = 0; file < 8; ++file) {
            if (first == last) {
                printf("breaking first loop :(\n");
                return last;
            }
            char c = *first++;
            if (c >= '1' && c <= '8') {
                file += c - '0' - 1;
            } else {
                auto maybe_piece = translate_fen_piece(c);
                if (!maybe_piece) {
                    printf("breaking on character: %c\n", c);
                    return last;
                }
                ColorPiece piece = *maybe_piece;
                const Square sq{file, rank};
                pos.sq2p_[sq.value()] = piece.value();
                pos.side_[piece.color()] |= sq.mask();
                if (piece.piece() == KING) {
                    pos.ksq_[piece.color()] = sq.value();
                } else {
                    pos.bbrd_[piece.value()] |= sq.mask();
                }
                // TEMP TEMP
                printf("Placed %s at %s(%d)\n", piece.name(), sq.name(), sq.value());
            }
        }
        ++first;
    }
    return first;
}

/* static */ std::optional<Position> Position::from_fen(std::string_view fen) {
    Position pos;
    pos.moves_ = 1;
    pos.wtm_ = WHITE;
    pos.halfmoves_ = 0;
    pos.castle_ = Position::CASTLE_NONE;
    pos.epsq_ = Position::ENPASSANT_NONE;

    auto first = fen.begin();
    auto last = fen.end();
    first = parse_fen_board(first, last, pos);
    auto it = first;

    printf("it = %c\n", *it);
    // Active color
    switch (*it++) {
        case 'w':
        case 'W':
            pos.wtm_ = WHITE;
            break;
        case 'b':
        case 'B':
            pos.wtm_ = BLACK;
            break;
        default:
            return std::nullopt;
    }
    // TEMP TEMP
    printf("To Move: %s\n", pos.wtm_ == WHITE ? "WHITE" : "BLACK");
    ++it; // space

    // Castling availability
    if (*it == '-') {
        pos.castle_ = Position::CASTLE_NONE;
        ++it;
    } else {
        while (it != fen.end() && *it != ' ') {
            switch (*it) {
                case 'K': pos.castle_ |= Position::CASTLE_WHITE_KING_SIDE; break;
                case 'Q': pos.castle_ |= Position::CASTLE_WHITE_QUEEN_SIDE; break;
                case 'k': pos.castle_ |= Position::CASTLE_BLACK_KING_SIDE; break;
                case 'q': pos.castle_ |= Position::CASTLE_BLACK_QUEEN_SIDE; break;
                default: return std::nullopt;
            }
            ++it;
        }
    }
    // TEMP TEMP
    printf("Castling availability: %u\n", pos.castle_);
    ++it;

    // En passant target square
    if (it == fen.end()) {
        return std::nullopt;
    } else if (*it == '-') {
        pos.epsq_ = Position::ENPASSANT_NONE;
    } else if (*it >= 'a' && *it <= 'h') {
        const u8 file = *it++ - 'a';
        const u8 rank = *it++ - '1';
        if (rank == 2 || rank == 5) {
            pos.epsq_ = Square{file, rank}.value();
        }
    } else {
        return std::nullopt;
    }

    // Halfmove clock
    while (it != fen.end() && *it != ' ') {
        char c = *it++;
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        pos.halfmoves_ *= 10;
        pos.halfmoves_ += c - '0';
    }
    ++it;

    while (it != fen.end()) {
        char c = *it;
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        pos.moves_ *= 10;
        pos.moves_ += c - '0';
    }

    return pos;
}

void make_move(Savepos& sp, Move move) {

}

void undo_move(const Savepos& sp, Move move) {

}
