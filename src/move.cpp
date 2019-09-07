#include "move.h"

namespace lesschess {

std::ostream& operator<<(std::ostream& os, Square sq) noexcept {
    os << sq.name();
    return os;
}

std::ostream& operator<<(std::ostream& os, Piece pc) noexcept {
    os << pc.name();
    return os;
}

std::ostream& operator<<(std::ostream& os, Move move) noexcept {
    os << "(";

    if (move.is_castle()) {
        switch (move.castle_kind()) {
            case Castle::WHITE_KING_SIDE:  os << "white 0-0 "; break;
            case Castle::BLACK_KING_SIDE:  os << "black 0-0 "; break;
            case Castle::WHITE_QUEEN_SIDE: os << "white 0-0-0 "; break;
            case Castle::BLACK_QUEEN_SIDE: os << "black 0-0-0 "; break;
            default:                                 os << "bad castle move "; break;
        }
        return os;
    }

    if (move.is_enpassant()) {
        os << "e.p. ";
    }
    os << move.from().name() << ", " << move.to().name();
    if (move.is_promotion()) {
        os << "=" << PieceKindNames[move.promotion()];
    }
    os << ")";
    return os;
}

} // ~namespace lesschess
