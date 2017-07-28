#include "eval.h"
#include "move.h"
#include "position.h"
#include "movegen.h"

int eval(struct position *restrict const pos) {
    int white_score = 0;
    int black_score = 0;

    const int white_pawns = popcountll(pos->brd[PIECE(WHITE, PAWN)]);
    const int white_knights = popcountll(pos->brd[PIECE(WHITE, KNIGHT)]);
    const int white_bishops = popcountll(pos->brd[PIECE(WHITE, BISHOP)]);
    const int white_rooks = popcountll(pos->brd[PIECE(WHITE, ROOK)]);
    const int white_queens = popcountll(pos->brd[PIECE(WHITE, QUEEN)]);

    const int black_pawns = popcountll(pos->brd[PIECE(BLACK, PAWN)]);
    const int black_knights = popcountll(pos->brd[PIECE(BLACK, KNIGHT)]);
    const int black_bishops = popcountll(pos->brd[PIECE(BLACK, BISHOP)]);
    const int black_rooks = popcountll(pos->brd[PIECE(BLACK, ROOK)]);
    const int black_queens = popcountll(pos->brd[PIECE(BLACK, QUEEN)]);

    const uint64_t white_attacked = generate_attacked(pos, WHITE);
    const uint64_t black_attacked = generate_attacked(pos, BLACK);
    const int white_squares_attacked = popcountll(white_attacked);
    const int black_squares_attacked = popcountll(black_attacked);


    white_score =
          100*white_pawns
        + 295*white_knights
        + 300*white_bishops
        + 500*white_rooks
        + 800*white_queens
        + white_squares_attacked
        ;

    black_score =
          100*black_pawns
        + 295*black_knights
        + 300*black_bishops
        + 500*black_rooks
        + 800*black_queens
        + black_squares_attacked
        ;

    return white_score - black_score;
}

