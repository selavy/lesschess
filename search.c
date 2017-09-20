#include "search.h"
#include "eval.h"
#include "movegen.h"
#include "position.h"
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct tt_entry {
    // TODO(plesslie): don't need to save entire hash, just
    // save bits not in the index
    uint64_t h;
    int v;
    // TODO(plesslie): probably need to save depth to?  score
    // from a deeper position should overwrite shallower values?
};

//#define TTSZ (1 << 20)
// smallest prime above 1 << 20
#define TTSZ 1048583
struct tt_entry tt_tbl[TTSZ];

void transposition_table_init() {
    assert(__builtin_popcount(TTSZ) == 0);
    for (int i = 0; i < TTSZ; ++i) {
        tt_tbl[i].h = 0;
        tt_tbl[i].v = 0;
    }
}

size_t tt_index(uint64_t h) { return h & (TTSZ - 1); }

int tt_occupied(size_t idx) { return tt_tbl[idx].h != 0 && tt_tbl[idx].v != 0; }

int tt_value(size_t idx) {
    assert(tt_tbl[idx].h != 0 && tt_tbl[idx].v != 0);
    return tt_tbl[idx].v;
}

void tt_set_value(size_t idx, uint64_t h, int v) {
    // const size_t idx = tt_index(h);
    tt_tbl[idx].h = h;
    tt_tbl[idx].v = v;
}

int alphabeta_helper(struct position *restrict pos, uint64_t zhash, int depth,
                     int alpha, int beta, int maximizing) {
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

    // TODO(plesslie): does undo_move() actually need to recalculate the zobrist
    // hash?
    // could just save it off before calling make_move
    if (maximizing) {
        best = NEG_INFINITI;
        for (i = 0; i < nmoves; ++i) {
            zhash = make_move(pos, &sp, moves[i], zhash);
            value = alphabeta_helper(pos, zhash, depth - 1, alpha, beta, 0);
            zhash = undo_move(pos, &sp, moves[i], zhash);
            best = MAX(best, value);
            alpha = MAX(alpha, best);
            if (beta <= alpha) {
                break;
            }
        }
    } else {
        best = INFINITI;
        for (i = 0; i < nmoves; ++i) {
            zhash = make_move(pos, &sp, moves[i], zhash);
            value = alphabeta_helper(pos, zhash, depth - 1, alpha, beta, 1);
            zhash = undo_move(pos, &sp, moves[i], zhash);
            best = MIN(best, value);
            beta = MIN(beta, best);
            if (beta <= alpha) {
                break;
            }
        }
    }

    return best;
}

int alphabeta(struct position *restrict pos, const move *restrict moves,
              int nmoves, uint64_t zhash, int depth, int *score) {
    struct savepos sp;
    int best = pos->wtm == WHITE ? NEG_INFINITI - 1 : INFINITI + 1;
    int bestmoveno = -1;
    int value;
    int i;

    if (pos->wtm == WHITE) {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value =
                alphabeta_helper(pos, zhash, depth, NEG_INFINITI, INFINITI, 0);
            if (value > best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(pos, &sp, moves[i], 0);
        }
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value =
                alphabeta_helper(pos, zhash, depth, NEG_INFINITI, INFINITI, 1);
            if (value < best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(pos, &sp, moves[i], 0);
        }
    }

    assert(bestmoveno > -1);
    *score = best;
    return bestmoveno;
}

move search(const struct position *restrict const p, int *score, int *searched_depth) {
    struct position pos;
    move moves[MAX_MOVES];
    int nmoves;
    const int max_depth = 5;
    uint64_t zhash;
    int moveno;
    int depth;

    memcpy(&pos, p, sizeof(pos));
    nmoves = generate_legal_moves(&pos, &moves[0]);
    if (nmoves == 0) {
        return MATED;
    }
    zobrist_hash_from_position(&pos, &zhash);

    for (depth = 2; depth <= max_depth; ++depth) {
        *searched_depth = depth;
        moveno = alphabeta(&pos, &moves[0], nmoves, zhash, depth, score);
        if (*score == INFINITI || *score == NEG_INFINITI) {
            break;
        }
    }

    assert(moveno >= 0 && moveno < nmoves);
    return moves[moveno];
}
