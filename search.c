#include "search.h"
#include <assert.h>
#include <string.h>
#include "position.h"
#include "movegen.h"
#include "eval.h"

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

struct tt_entry {
    // TODO(plesslie): don't need to save entire hash, just
    // save bits not in the index
    uint64_t h;
    int      v;
    // TODO(plesslie): probably need to save depth to?  score
    // from a deeper position should overwrite shallower values?
};

#define TTSZ (1 << 20)
struct tt_entry tt_tbl[TTSZ];

void transposition_table_init() {
    //memset(&tt_tbl[0], 0, sizeof(tt_tbl) * sizeof(tt_tbl[0]));
    assert(__builtin_popcount(TTSZ) == 0);
    for (int i = 0; i < TTSZ; ++i) {
        tt_tbl[i].h = 0;
        tt_tbl[i].v = 0;
    }
}

size_t tt_index(uint64_t h) {
    return h & (TTSZ - 1);
}

int tt_occupied(size_t idx) {
    return !(tt_tbl[idx].h == 0 && tt_tbl[idx].v == 0);
}

int tt_value(size_t idx) {
    assert(tt_tbl[idx].h != 0 && tt_tbl[idx].v != 0);
    return tt_tbl[idx].v;
}

void tt_set_value(uint64_t h, int v) {
    const size_t idx = tt_index(h);
    tt_tbl[idx].h = h;
    tt_tbl[idx].v = v;
}

int alphabeta(struct position *restrict pos, uint64_t zhash, int depth, int alpha, int beta, int maximizing) {
    int best;
    int nmoves;
    int i;
    int value;
    move moves[MAX_MOVES];
    struct savepos sp;

    // TODO(plesslie): make sense to move this code into the loop _before_ calling alphabeta on a branch
    // that is going to exit back immediately to save to function call overhead.
    const size_t idx = tt_index(zhash);
    if (tt_occupied(idx)) {
        return tt_value(idx);
    }

    if (depth == 0) {
        value = eval(pos);
        return value;
    }

    nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        return pos->wtm ? WHITE_WIN : BLACK_WIN;
    }

    // TODO(plesslie): do undo_move() actually need to recalculate the zobrist hash?
    // could just save it off before calling make_move
    if (maximizing) {
        best = NEG_INFINITI;
        for (i = 0; i < nmoves; ++i) {
            zhash = make_move(pos, &sp, moves[i], zhash);
            value = alphabeta(pos, zhash, depth - 1, alpha, beta, 0);
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
            value = alphabeta(pos, zhash, depth - 1, alpha, beta, 1);
            zhash = undo_move(pos, &sp, moves[i], zhash);
            best = MIN(best, value);
            beta = MIN(beta, best);
            if (beta <= alpha) {
                break;
            }
        }
    }

    tt_set_value(zhash, best);

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
    uint64_t zhash;

    memcpy(&pos, position, sizeof(pos));
    nmoves = generate_legal_moves(&pos, &moves[0]);
    if (nmoves == 0) {
        return MATED;
    }
    zobrist_hash_from_position(&pos, &zhash);

    if (pos.wtm == WHITE) {
        for (i = 0; i < nmoves; ++i) {
            make_move(&pos, &sp, moves[i], 0);
            value = alphabeta(&pos, zhash, depth, NEG_INFINITI, INFINITI, 0);
            if (value > best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(&pos, &sp, moves[i], 0);
        }
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(&pos, &sp, moves[i], 0);
            value = alphabeta(&pos, zhash, depth, NEG_INFINITI, INFINITI, 1);
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

