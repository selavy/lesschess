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

enum {
    TT_NONE = 0,
    TT_EXACT,
    TT_ALPHA,
    TT_BETA,
};

struct tt_entry {
    // TODO(plesslie): don't need to save entire hash, just
    // save bits not in the index
    uint64_t hash;
    int score;
    move move;
    uint8_t depth;
    uint8_t flags;
    // TODO(plesslie): probably need to save depth to?  score
    // from a deeper position should overwrite shallower values?
};

//#define TTSZ (1 << 20)
// smallest prime above 1 << 20
#define TTSZ 1048583
struct tt_entry tt_tbl[TTSZ];

void transposition_table_init() {
    // static_assert(__builtin_popcount(TTSZ) == 0, "Transposition Table size
    // must be a power of 2!");
    for (int i = 0; i < TTSZ; ++i) {
        tt_tbl[i].hash = 0;
        tt_tbl[i].score = 0;
        tt_tbl[i].move = INVALID_MOVE;
        tt_tbl[i].depth = 0;
        tt_tbl[i].flags = TT_NONE;
    }
}

// int tt_index(uint64_t h) { return h & (TTSZ - 1); }
int tt_index(uint64_t h) { return h % TTSZ; }

int alphabeta(struct position *pos, uint64_t zhash, int depth, int alpha, int beta, int maximizing) {
    int nmoves;
    int i;
    int value;
    move moves[MAX_MOVES];
    struct savepos sp;
    int best;

    if (depth == 0) {
        return eval(pos);
    }

    nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        return pos->wtm ? WHITE_WIN : BLACK_WIN;
    }

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

    return best;
}

struct search_node {
    struct position *pos;
    struct savepos *sp;
    const move *moves;
    uint64_t zhash;
    int nmoves;
};

move alphabeta_search(struct search_node *n, int depth, int *score) {
    int value;
    int i;
    struct position *pos = n->pos;
    struct savepos *sp = n->sp;
    const move *moves = n->moves;
    uint64_t zhash = n->zhash;
    const int nmoves = n->nmoves;
    move bestmove = INVALID_MOVE;
    int bestscore = pos->wtm == WHITE ? NEG_INFINITI : INFINITI;

    assert(nmoves > 0);

    if (pos->wtm == WHITE) {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, sp, moves[i], 0);
            value = alphabeta(pos, zhash, depth, NEG_INFINITI, INFINITI, 0);
            undo_move(pos, sp, moves[i], 0);
            if (value > bestscore) {
                bestmove = moves[i];
                bestscore = value;
            }
        }
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, sp, moves[i], 0);
            value = alphabeta(pos, zhash, depth, NEG_INFINITI, INFINITI, 1);
            undo_move(pos, sp, moves[i], 0);
            if (value < bestscore) {
                bestmove = moves[i];
                bestscore = value;
            }
        }
    }

    assert(bestmove != INVALID_MOVE);
    *score = bestscore;
    return bestmove;
}

move search(const struct position *p, int *score, int *searched_depth) {
    const int max_depth = 5;
    struct position pos;
    struct savepos sp;
    move moves[MAX_MOVES];
    int depth;
    move best_move;
    struct search_node node;

    memcpy(&pos, p, sizeof(pos));
    node.nmoves = generate_legal_moves(&pos, &moves[0]);
    if (node.nmoves == 0) {
        return MATED;
    }
    zobrist_hash_from_position(&pos, &node.zhash);
    node.pos = &pos;
    node.sp = &sp;
    node.moves = &moves[0];

    for (depth = 2; depth <= max_depth; ++depth) {
        *searched_depth = depth;
        best_move = alphabeta_search(&node, depth, score);
        if (*score == INFINITI || *score == NEG_INFINITI) {
            break;
        }
    }

    return best_move;
}
