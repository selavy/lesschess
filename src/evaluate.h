#pragma once

namespace lesschess {

// Must line up with PieceKind values from move.h
constexpr int BasePieceValues[6] = {
    300, // knight
    310, // bishop
    500, // rook
    800, // queen
    100, // pawn
    0,   // king
};

class Position;

int evaluate(const Position& position);

} // ~namespace lesschess
