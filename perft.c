#include "perft.h"
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include "movegen.h"

#define COUNT_CHECKS_AND_MATES

static uint64_t perft(int depth,
        struct position *restrict pos,
        uint64_t *captures,
        uint64_t *eps,
        uint64_t *castles,
        uint64_t *promos,
        uint64_t *checks,
        uint64_t *mates) {
    int i;
    int nmoves;
    uint64_t nodes = 0;
    move moves[MAX_MOVES];
#ifndef NDEBUG
    struct position tmp;
#endif
    uint64_t orig_z;
    uint64_t new_z;
    uint64_t z;
    struct savepos sp;
    uint32_t flags;

    if (depth == 0) {
#ifdef COUNT_CHECKS_AND_MATES
        nmoves = generate_legal_moves(pos, &moves[0]);
        if (nmoves == 0) {
            ++(*mates);
        }
        if (in_check(pos, pos->wtm)) {
            ++(*checks);
        }
#endif
        return 1;
    }

    assert(in_check(pos, FLIP(pos->wtm)) == 0);
    assert(validate_position(pos) == 0);
#ifndef NDEBUG
    memcpy(&tmp, pos, sizeof(tmp));
#endif
    zobrist_hash_from_position(pos, &orig_z);
    new_z = orig_z;
    assert(new_z != 0);
    nmoves = generate_legal_moves(pos, &moves[0]);

    if (depth > 1) {
        for (i = 0; i < nmoves; ++i) {
            new_z = make_move(pos, &sp, moves[i], new_z);
            zobrist_hash_from_position(pos, &z);
            assert(new_z == z);
            assert(new_z != orig_z);
            nodes += perft(depth - 1, pos, captures, eps, castles, promos, checks, mates);
            new_z = undo_move(pos, &sp, moves[i], new_z);
            assert(new_z == orig_z);

#ifndef NDEBUG
            assert(memcmp(pos, &tmp, sizeof(tmp)) == 0);
#endif
        }
    } else {
        nodes += nmoves;
        for (i = 0; i < nmoves; ++i) {
            flags = FLAGS(moves[i]);
            switch (flags) {
                case FLG_NONE:
                    if (pos->sqtopc[TO(moves[i])] != EMPTY) {
                        ++(*captures);
                    }
                    break;
                case FLG_EP: ++(*eps); ++(*captures); break;
                case FLG_PROMO:
                    ++(*promos);
                    if (pos->sqtopc[TO(moves[i])] != EMPTY) {
                        ++(*captures);
                    }
                    break;
                case FLG_CASTLE: ++(*castles); break;
                default: break;
            }

#ifdef COUNT_CHECKS_AND_MATES
            make_move(pos, &sp, moves[i], 0);
            perft(depth - 1, pos, captures, eps, castles, promos, checks, mates);
            undo_move(pos, &sp, moves[i], 0);
            #ifndef NDEBUG
            assert(memcmp(pos, &tmp, sizeof(tmp)) == 0);
            #endif
#endif
        }
    }
    return nodes;
}

int perft_test(const struct position *restrict position,
        int depth,
        uint64_t *nodes,
        uint64_t *captures,
        uint64_t *eps,
        uint64_t *castles,
        uint64_t *promos,
        uint64_t *checks,
        uint64_t *mates) {
    *nodes = *captures = *eps = *castles = *promos = *checks = *mates = 0;
    if (depth < 0) {
        return 1;
    }
    static struct position pos;
    memcpy(&pos, position, sizeof(pos));
    *nodes = perft(depth, &pos, captures, eps, castles, promos, checks, mates);
    return 0;
}

uint64_t perft_speed(struct position *restrict pos, int depth) {
    int i;
    int nmoves;
    uint64_t nodes = 0;    
    move moves[MAX_MOVES];
    struct savepos sp;
    if (depth == 0) {
        return 1;
    }
    nmoves = generate_legal_moves(pos, &moves[0]);
    if (depth == 1) {
        nodes = nmoves;
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            nodes += perft_speed(pos, depth - 1);
            undo_move(pos, &sp, moves[i], 0);
        }
    }
    return nodes;
}

static uint64_t perft_text_tree_helper(struct position *restrict pos, int depth) {
    int i;
    int nmoves;
    move moves[MAX_MOVES];
    uint64_t nodes = 0;
    struct savepos sp;
    if (depth == 0) {
        return 1;
    }
    nmoves = generate_legal_moves(pos, &moves[0]);
    if (depth == 1) {
        nodes = nmoves;
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            nodes += perft_text_tree_helper(pos, depth - 1);
            undo_move(pos, &sp, moves[i], 0);
        }
    }
    return nodes;
}

void perft_text_tree(struct position *restrict pos, int depth) {
    int i;
    int nmoves;
    move moves[MAX_MOVES];
    struct savepos sp;
    uint64_t total_nodes = 0;
    uint64_t nodes;
    depth = depth > 0 ? depth : 1;
    nmoves = generate_legal_moves(pos, &moves[0]);
    for (i = 0; i < nmoves; ++i) {
        make_move(pos, &sp, moves[i], 0);
        nodes = perft_text_tree_helper(pos, depth - 1);
        total_nodes += nodes;
        undo_move(pos, &sp, moves[i], 0);
        move_print_short(moves[i]); printf(": %" PRIu64 "\n", nodes);
    }
    printf("Total nodes at depth %d: %" PRIu64 "\n", depth, total_nodes);
}
