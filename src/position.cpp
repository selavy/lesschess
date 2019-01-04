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
    : bbrd{0}
    , side{0}
    , ksqs{0}
    , moves(1)
    , halfmoves(0)
    , wtm(WHITE)
    , castle(Position::CASTLE_ALL)
    , epsq(Position::ENPASSANT_NONE)
{
    memset(sq2p, EMPTY_SQUARE, sizeof(sq2p));
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
                position.sq2p[sq.value()] = piece.value();
                position.side[piece.color()] |= sq.mask();
                if (piece.kind() == KING) {
                    position.ksqs[piece.color()] = sq.value();
                } else {
                    position.bbrd[piece.value()] |= sq.mask();
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
            position.wtm = WHITE;
            break;
        case 'B':
        case 'b':
            position.wtm = BLACK;
            break;
        default:
            throw std::runtime_error("Invalid character in color specification");
    }
    it = expect(it, ' ');

    // Castling Availability
    if (it == last) {
        throw std::runtime_error("Expected castling availability specification");
    } else if (*it == '-') {
        position.castle = Position::CASTLE_NONE;
        ++it;
    } else {
        while (it < last && *it != ' ') {
            char c = *it++;
            switch (c) {
            case 'K':
                position.castle |= Position::CASTLE_WHITE_KING_SIDE;
                break;
            case 'k':
                position.castle |= Position::CASTLE_BLACK_KING_SIDE;
                break;
            case 'Q':
                position.castle |= Position::CASTLE_WHITE_QUEEN_SIDE;
                break;
            case 'q':
                position.castle |= Position::CASTLE_BLACK_QUEEN_SIDE;
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
        position.epsq = Position::ENPASSANT_NONE;
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
        position.epsq = Square{file, rank}.value();
    }
    it = expect(it, ' ');

    // may or may not have halfmove and move specifications
    if (it == last) {
        // Halfmove spec (50-move rule)
        while (it != last && *it != ' ') {
            if (*it < '0' && *it > '9') {
                throw std::runtime_error("Invalid halfmove specification");
            }
            position.halfmoves *= 10;
            position.halfmoves += *it - '0';
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
        }
    }

    return position;
}

void make_move(Savepos& sp, Move move) {

}

void undo_move(const Savepos& sp, Move move) {

}
