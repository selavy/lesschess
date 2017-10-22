#include "search.h"
#include "evaluate.h"
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
int tt_index(uint64_t h) {
    return h % TTSZ;
}

struct search_node {
    struct position *pos;
    struct savepos *sp;
    const move *moves;
    uint64_t zhash;
    int nmoves;
};

int alphabeta(struct position *pos, uint64_t zhash, int depth, int alpha, int beta, int color) {
    if (depth == 0) {
        const int score = evaluate(pos);
        return score;
    }
    assert(zhash == zobrist_hash_from_position(pos));
    assert(color == WHITE || color == BLACK);

    uint64_t new_zhash;
    move moves[MAX_MOVES];
    const int nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        return pos->wtm ? INF : NEG_INF;
    }

    int best;
    struct savepos sp;
    if (color == BLACK) {
        best = NEG_INF;
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, &sp, moves[i], zhash);
            const int value = alphabeta(pos, new_zhash, depth - 1, alpha, beta, WHITE);
            undo_move(pos, &sp, moves[i]);
            best = MAX(value, best);
            alpha = MAX(alpha, best);
            if (beta <= alpha) {
                break;
            }
        }
    } else {
        best = INF;
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, &sp, moves[i], zhash);
            const int value = alphabeta(pos, new_zhash, depth - 1, alpha, beta, BLACK);
            undo_move(pos, &sp, moves[i]);
            best = MIN(best, value);
            beta = MIN(beta, best);
            if (beta <= alpha) {
                break;
            }
        }
    }

    return best;
}

move alphabeta_search(const struct search_node *n, int depth, int *score) {
    struct position *pos = n->pos;
    struct savepos *sp = n->sp;
    const move *moves = n->moves;
    const uint64_t zhash = n->zhash;
    const int nmoves = n->nmoves;
    move bestmove = INVALID_MOVE;
    uint64_t new_zhash;

    assert(zhash == zobrist_hash_from_position(pos));
    assert(nmoves > 0);
    int bestscore = pos->wtm == WHITE ? NEG_INF : INF;
    if (pos->wtm == WHITE) {
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, sp, moves[i], zhash);
            assert(new_zhash == zobrist_hash_from_position(pos));
            const int value = alphabeta(pos, new_zhash, depth, NEG_INF, INF, WHITE);
            undo_move(pos, sp, moves[i]);
            if (value > bestscore) {
                bestmove = moves[i];
                bestscore = value;
            }
        }
    } else {
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, sp, moves[i], zhash);
            assert(new_zhash == zobrist_hash_from_position(pos));
            const int value = alphabeta(pos, new_zhash, depth, NEG_INF, INF, 1);
            undo_move(pos, sp, moves[i]);
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

move search(struct position *pos, int *score, int *searched_depth) {
    const int max_depth = 5;
    struct savepos sp;
    move moves[MAX_MOVES];
    move best_move;

    const int nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        return MATED;
    }
    const uint64_t zhash = zobrist_hash_from_position(pos);
    const struct search_node node = {.pos = pos, .sp = &sp, .moves = &moves[0], .zhash = zhash, .nmoves = nmoves};
    assert(node.zhash == zobrist_hash_from_position(pos));

    for (int depth = 2; depth <= max_depth; ++depth) {
        *searched_depth = depth;
        best_move = alphabeta_search(&node, depth, score);
        if (*score == INF || *score == NEG_INF) {
            break;
        }
    }

    return best_move;
}
