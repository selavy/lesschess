#include "search.h"
#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

static inline force_inline
int min(int a, int b) {
    return a < b ? a : b;
}

static inline force_inline
int max(int a, int b) {
    return a > b ? a : b;
}

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
    uint8_t depth;
    int score;
    uint8_t flags;
    move move;
};

//#define TTSZ (1 << 20)
// smallest prime above 1 << 20
#define TTSZ 1048583
struct tt_entry tt_tbl[TTSZ];
uint64_t transposition_table_hits;
uint64_t transposition_table_lookups;

void transposition_table_counters_init() {
    transposition_table_hits = 0ull;
    transposition_table_lookups = 0ull;
}

void transposition_table_counters_print() {
    fprintf(stderr, "Transposition Table Hits   : %" PRIu64 "\n", transposition_table_hits);
    fprintf(stderr, "Transposition Table Lookups: %" PRIu64 "\n", transposition_table_lookups);    
}

void transposition_table_init() {
    // static_assert(__builtin_popcount(TTSZ) == 0, "Transposition Table size
    // must be a power of 2!");
    for (int i = 0; i < TTSZ; ++i) {
        tt_tbl[i].hash = 0ull;
        tt_tbl[i].score = 0;
        tt_tbl[i].move = INVALID_MOVE;
        tt_tbl[i].depth = 0u;
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
    const int table_index = tt_index(zhash);
    ++transposition_table_lookups;
    if (tt_tbl[table_index].hash == zhash && tt_tbl[table_index].depth >= depth) {
        ++transposition_table_hits;
        return tt_tbl[table_index].score;
    }
    
    if (depth == 0) {
        const int score = evaluate(pos);
        #define USE_TRANSPOSITION_TABLE
        #ifdef USE_TRANSPOSITION_TABLE
        tt_tbl[table_index].hash = zhash;
        tt_tbl[table_index].depth = 0;
        tt_tbl[table_index].score = score;
        tt_tbl[table_index].flags = TT_EXACT;
        tt_tbl[table_index].move = INVALID_MOVE; // TODO(plesslie): fix
        #endif
        return score;
    }
    assert(zhash == zobrist_hash_from_position(pos));
    assert(color == WHITE || color == BLACK);

    uint64_t new_zhash;
    move moves[MAX_MOVES];
    const int nmoves = generate_legal_moves(pos, &moves[0]);
    if (nmoves == 0) {
        // TODO(plesslie): this could be stalemate
        return pos->wtm ? INF : NEG_INF;
    }

    // 50-move draw
    if (pos->halfmoves >= 100) {
        return 0;
    }

    int best;
    struct savepos sp;
    if (color == BLACK) {
        best = NEG_INF;
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, &sp, moves[i], zhash);
            const int value = alphabeta(pos, new_zhash, depth - 1, alpha, beta, WHITE);
            undo_move(pos, &sp, moves[i]);
            best = max(value, best);
            alpha = max(alpha, best);
            if (beta <= alpha) { // alpha cutoff
                #ifdef USE_TRANSPOSITION_TABLE
                if (tt_tbl[table_index].hash == zhash && tt_tbl[table_index].flags != TT_EXACT) {
                    tt_tbl[table_index].hash = zhash;
                    tt_tbl[table_index].depth = depth;
                    tt_tbl[table_index].score = best;
                    tt_tbl[table_index].flags = TT_ALPHA;
                    tt_tbl[table_index].move = moves[i];
                }
                #endif
                break;
            }
        }
    } else {
        best = INF;
        for (int i = 0; i < nmoves; ++i) {
            new_zhash = make_move(pos, &sp, moves[i], zhash);
            const int value = alphabeta(pos, new_zhash, depth - 1, alpha, beta, BLACK);
            undo_move(pos, &sp, moves[i]);
            best = min(best, value);
            beta = min(beta, best);
            if (beta <= alpha) { // beta cutoff
                #ifdef USE_TRANSPOSITION_TABLE
                if (tt_tbl[table_index].hash == zhash && tt_tbl[table_index].flags != TT_EXACT) {
                    tt_tbl[table_index].hash = zhash;
                    tt_tbl[table_index].depth = depth;
                    tt_tbl[table_index].score = best;
                    tt_tbl[table_index].flags = TT_BETA;
                    tt_tbl[table_index].move = moves[i];
                }
                #endif
                break;
            }
        }
    }

    return best;
}

move root_search(const struct search_node *n, int depth, int *score) {
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
        best_move = root_search(&node, depth, score);
        if (*score == INF || *score == NEG_INF) {
            break;
        }
    }

    return best_move;
}
