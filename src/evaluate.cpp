#include "evaluate.h"
#include "position.h"
#include <initializer_list>

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

    // Basic Material Imbalance:
    std::initializer_list<PieceKind> kinds = {
        PieceKind::PAWN,
        PieceKind::KNIGHT,
        PieceKind::BISHOP,
        PieceKind::ROOK,
        PieceKind::QUEEN,
    };
    for (auto kind : kinds) {
        score += BasePieceValues[kind] * (position.piece_count(WHITE, kind) - position.piece_count(BLACK, kind));
    }

    // Space:
    // number of squares attacked by either side
    // TODO: weighted by "importance" aka closer to center or not?

    // King Safety:

    // Pawn Structure:

    return score;
}

} // ~namespace lesschess
