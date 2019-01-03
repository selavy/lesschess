#include "position.h"
#include <cstring>
#include <cstdlib>

namespace {

ColorPiece translate_fen_piece(char c) {
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
        default: throw std::runtime_error{"Invalid piece"};
    }
    return ColorPiece{color, piece};
}

} // ~anonymous namespace

Position::Position() noexcept
    : bbrd_{0}
    , side_{0}
    , sq2p_{0}
    , ksq_{0}
    , moves_(1)
    , halfmoves_(0)
    , wtm_(WHITE)
    , castle_(Position::CASTLE_ALL)
    , epsq_(Position::ENPASSANT_NONE)
{
    // memset(&bbrd_[0], 0, sizeof(bbrd_));
    // memset(&sq2p_[0], 0, sizeof(sq2p_));
    // memset(&side_[0], 0, sizeof(side_));
    // memset(&ksq_[0] , 0, sizeof(ksq_));
    // for (int i = 0; i < sizeof(sq2p_); ++i) {
    //     sq2p_[i] = Square{};
    // }
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
                ColorPiece piece = translate_fen_piece(c);
                const Square sq{static_cast<u8>(file), static_cast<u8>(rank)};
                position.sq2p_[sq.value()] = piece.value();
                position.side_[piece.color()] |= sq.mask();
                if (piece.piece() == KING) {
                    position.ksq_[piece.color()] = sq.value();
                } else {
                    position.bbrd_[piece.value()] |= sq.mask();
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
            position.wtm_ = WHITE;
            break;
        case 'B':
        case 'b':
            position.wtm_ = BLACK;
            break;
        default:
            throw std::runtime_error("Invalid character in color specification");
    }
    it = expect(it, ' ');

    // Castling Availability
    if (it == last) {
        throw std::runtime_error("Expected castling availability specification");
    } else if (*it == '-') {
        position.castle_ = Position::CASTLE_NONE;
        ++it;
    } else {
        while (it < last && *it != ' ') {
            char c = *it++;
            switch (c) {
            case 'K':
                position.castle_ |= Position::CASTLE_WHITE_KING_SIDE;
                break;
            case 'k':
                position.castle_ |= Position::CASTLE_BLACK_KING_SIDE;
                break;
            case 'Q':
                position.castle_ |= Position::CASTLE_WHITE_QUEEN_SIDE;
                break;
            case 'q':
                position.castle_ |= Position::CASTLE_BLACK_QUEEN_SIDE;
                break;
            default:
                throw std::runtime_error("Invalid character in castling specification");
            }
        }
    }
    it = expect(it, ' ');

    // En passant Target Square
    if (it == last) {
        throw std::runtime_error("Expected en passant target square");
    } else if (*it == '-') {
        position.epsq_ = Position::ENPASSANT_NONE;
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
        if (!(rank == 2 || rank == 5)) {
            throw std::runtime_error("Invalid rank for enpassant target square");
        }
        position.epsq_ = Square{file, rank}.value();
    }
    it = expect(it, ' ');

    // may or may not have halfmove and move specifications
    if (it == last) {
        // Halfmove spec (50-move rule)
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid halfmove specification");
            }
            position.halfmoves_ *= 10;
            position.halfmoves_ += *it - '0';
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
        }
    }

    return position;
}

void make_move(Savepos& sp, Move move) {

}

void undo_move(const Savepos& sp, Move move) {

}
