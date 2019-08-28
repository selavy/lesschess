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

    // Basic Material Imbalance:
    score += position.piece_count(WHITE, PieceKind::PAWN)   - position.piece_count(BLACK, PieceKind::PAWN);
    score += position.piece_count(WHITE, PieceKind::KNIGHT) - position.piece_count(BLACK, PieceKind::KNIGHT);
    score += position.piece_count(WHITE, PieceKind::BISHOP) - position.piece_count(BLACK, PieceKind::BISHOP);
    score += position.piece_count(WHITE, PieceKind::ROOK)   - position.piece_count(BLACK, PieceKind::ROOK);
    score += position.piece_count(WHITE, PieceKind::QUEEN)  - position.piece_count(BLACK, PieceKind::QUEEN);

    // Space:
    // number of squares attacked by either side
    // TODO: weighted by "importance" aka closer to center or not?

    // King Safety:

    // Pawn Structure:

    return score;
}

} // ~namespace lesschess
