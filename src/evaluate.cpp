#include "evaluate.h"
#include "position.h"

namespace lesschess {

// Must line up with PieceKind values from move.h
int BasePieceValues[6] = {
    300, // knight
    310, // bishop
    500, // rook
    800, // queen
    100, // pawn
    0,   // king
};

int evaluate(const Position& position) {
    int score = 0;

    for (int pc = KNIGHT; pc != KING; ++pc) {
        score += popcountll(position._bboard(WHITE, static_cast<PieceKind>(pc))) * BasePieceValues[pc];
    }

    for (int pc = KNIGHT; pc != KING; ++pc) {
        score -= popcountll(position._bboard(BLACK, static_cast<PieceKind>(pc))) * BasePieceValues[pc];
    }

    return score;
}

} // ~namespace lesschess
