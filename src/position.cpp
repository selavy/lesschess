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
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            if (first == last) {
                return last;
            }
            char c = *first++;
            if (c >= '1' && c <= '8') {
                file += c - '0' - 1;
            } else {
                auto maybe_piece = translate_fen_piece(c);
                if (!maybe_piece) {
                    return last;
                }
                ColorPiece piece = *maybe_piece;
                const Square sq{static_cast<u8>(file), static_cast<u8>(rank)};
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
        if (first < last && *first == '/') {
            ++first;
        }
    }
    return first;
}

std::string_view::iterator
consume_spaces(std::string_view::iterator first,
               std::string_view::iterator last) noexcept
{
    while (first < last && *first == ' ') {
        ++first;
    }
    return first;
}

std::string_view::iterator
parse_fen_color(std::string_view::iterator first,
                std::string_view::iterator last,
                Position& pos) noexcept
{
    if (first < last) {
        char c = *first++;
        switch (c) {
            case 'W':
            case 'w':
                pos.wtm_ = WHITE;
                break;
            case 'B':
            case 'b':
                pos.wtm_ = BLACK;
                break;
            default:
                return last;
        }
    }
    return first;
}

std::string_view::iterator
parse_fen_castling(std::string_view::iterator first,
                   std::string_view::iterator last,
                   Position& pos) noexcept
{
    if (!(first < last)) {
        return last;
    }
    if (*first == '-') {
        pos.castle_ = Position::CASTLE_NONE;
        return first + 1;
    }
    while (first < last) {
        const char c = *first++;
        switch (c) {
            case 'K':
                pos.castle_ |= Position::CASTLE_WHITE_KING_SIDE;
                break;
            case 'k':
                pos.castle_ |= Position::CASTLE_BLACK_KING_SIDE;
                break;
            case 'Q':
                pos.castle_ |= Position::CASTLE_WHITE_QUEEN_SIDE;
                break;
            case 'q':
                pos.castle_ |= Position::CASTLE_BLACK_QUEEN_SIDE;
                break;
            default:
                return last;
        }
    }
    return first;
}

std::string_view::iterator
parse_fen_epsq(std::string_view::iterator first,
               std::string_view::iterator last,
               Position& pos)
{
    if (!(first < last)) {
        return last;
    }
    char c = *first++;
    if (c == '-') {
        pos.epsq_ = Position::ENPASSANT_NONE;
        return first;
    }
    if (c >= 'a' && c <= 'h') {
        u8 file = c - 'a';
        u8 rank = *first++ - '1';
        assert(file >= 0 && file <= 7);
        assert(rank >= 0 && rank <= 7);
        if (rank == 2 || rank == 5) {
            pos.epsq_ = Square{file, rank}.value();
            return first;
        }
    }
    return last;
}

template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>::type>
std::string_view::iterator
parse_fen_number(std::string_view::iterator first,
                 std::string_view::iterator last,
                 T& out)
{
    if (!(first < last)) {
        return first;
    }
    out = 0;
    while (first < last) {
        char c = *first++;
        if (c >= '0' && c <= '9') {
            out *= 10;
            out += c - '0';
        } else {
            break;
        }
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
    auto it = fen.begin();
    auto last = fen.end();
    it = parse_fen_board(it, last, pos);
    if (it == last) return std::nullopt;
    it = consume_spaces(it, last);
    it = parse_fen_color(it, last, pos);
    if (it == last) return std::nullopt;
    it = consume_spaces(it, last);
    it = parse_fen_castling(it, last, pos);
    if (it == last) return std::nullopt;
    it = consume_spaces(it, last);
    it = parse_fen_epsq(it, last, pos);
    if (it == last) return std::nullopt;
    it = consume_spaces(it, last);
    it = parse_fen_number(it, last, pos.halfmoves_);
    it = consume_spaces(it, last);
    it = parse_fen_number(it, last, pos.moves_);
    return pos;
}

void make_move(Savepos& sp, Move move) {

}

void undo_move(const Savepos& sp, Move move) {

}
