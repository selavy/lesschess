#include "move.h"

std::ostream& operator<<(std::ostream& os, Square sq) noexcept {
    os << sq.name();
    return os;
}

std::ostream& operator<<(std::ostream& os, Piece pc) noexcept {
    os << pc.name();
    return os;
}
