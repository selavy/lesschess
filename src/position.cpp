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

/* static */ std::optional<Position> Position::from_fen(std::string_view fen) {
    Position pos;
    pos.moves_ = 1;
    pos.wtm_ = WHITE;
    pos.halfmoves_ = 0;
    pos.castle_ = Position::CASTLE_NONE;
    pos.epsq_ = Position::ENPASSANT_NONE;
    memset(&pos.bbrd_[0], 0, sizeof(pos.bbrd_));
    memset(&pos.sq2p_[0], 0, sizeof(pos.sq2p_));
    memset(&pos.side_[0], 0, sizeof(pos.side_));

    auto it = fen.begin();
    u8 sq = 63;
    while (it != fen.end() && sq >= 0) {
        const char c = *it++;
        if (c >= '1' && c <= '8') {
            sq -= c - '0';
        } else if (c == '/') {
            // ++sq;
            // assert(sq % 8 == 0);
        } else {
            auto maybe_piece = translate_fen_piece(c);
            if (!maybe_piece) {
                return std::nullopt;
            }
            ColorPiece piece = *maybe_piece;
            const Square square{sq};
            auto mask = square.mask();
            pos.sq2p_[sq] = piece.value();
            pos.side_[piece.color()] |= mask;
            if (piece.piece() == KING) {
                pos.ksq_[piece.color()] = sq;
            } else {
                pos.bbrd_[piece.value()] |= mask;
            }
            // TEMP TEMP
            printf("Placed %s at %s(%d)\n", piece.name(), square.name(), sq);
            --sq;
        }
    }

    // Active color
    if (it == fen.end() || *it++ != ' ') {
        return std::nullopt;
    }
    if (it == fen.end()) {
        return std::nullopt;
    }

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

    // Castling availability
    if (it == fen.end()) {
        return std::nullopt;
    }
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
