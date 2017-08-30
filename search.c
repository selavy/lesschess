#include "search.h"
#include <assert.h>
#include <string.h>
#include "position.h"
#include "movegen.h"
#include "eval.h"

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

int alphabeta(struct position *restrict pos, int depth, int alpha, int beta, int maximizing, move last_move) {
    int best;
    int nmoves;
    int i;
    int value;
    move moves[MAX_MOVES];
    struct savepos sp;

    if (depth == 0) {
        value = eval(pos);
        return value;
    }

    nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        return pos->wtm ? WHITE_WIN : BLACK_WIN;
    }

    if (maximizing) {
        best = NEG_INFINITI;
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value = alphabeta(pos, depth - 1, alpha, beta, 0, moves[i]);
            undo_move(pos, &sp, moves[i], 0);
            best = MAX(best, value);
            alpha = MAX(alpha, best);
            if (beta <= alpha) {
                break;
            }
        }
    } else {
        best = INFINITI;
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value = alphabeta(pos, depth - 1, alpha, beta, 1, moves[i]);
            undo_move(pos, &sp, moves[i], 0);
            best = MIN(best, value);
            beta = MIN(beta, best);
            if (beta <= alpha) {
                break;
            }
        }
    }

    return best;
}

move search(const struct position *restrict const position) {
    struct savepos sp;
    struct position pos;
    move moves[MAX_MOVES];
    int nmoves;
    int i;
    int best = position->wtm == WHITE ? NEG_INFINITI - 1 : INFINITI + 1;
    int value;
    int bestmoveno = -1;
    const int depth = 5;

    memcpy(&pos, position, sizeof(pos));
    nmoves = generate_legal_moves(&pos, &moves[0]);
    if (nmoves == 0) {
        return MATED;
    }

    if (pos.wtm == WHITE) {
        for (i = 0; i < nmoves; ++i) {
            make_move(&pos, &sp, moves[i], 0);
            value = alphabeta(&pos, depth, NEG_INFINITI, INFINITI, 0, moves[i]);
            if (value > best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(&pos, &sp, moves[i], 0);
        }
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(&pos, &sp, moves[i], 0);
            value = alphabeta(&pos, depth, NEG_INFINITI, INFINITI, 1, moves[i]);
            if (value < best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(&pos, &sp, moves[i], 0);
        }
    }

    assert(bestmoveno > -1);
    return moves[bestmoveno];
}

