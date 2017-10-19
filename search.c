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
    uint64_t hash;
    int value;
    int depth;
    // TODO(plesslie): probably need to save depth to?  score
    // from a deeper position should overwrite shallower values?
};

//#define TTSZ (1 << 20)
// smallest prime above 1 << 20
#define TTSZ 1048583
struct tt_entry tt_tbl[TTSZ];

void transposition_table_init() {
    // static_assert(__builtin_popcount(TTSZ) == 0, "Transposition Table size must be a power of 2!");
    for (int i = 0; i < TTSZ; ++i) {
        tt_tbl[i].hash = 0;
        tt_tbl[i].value = 0;
        tt_tbl[i].depth = 0;
    }
}

// size_t tt_index(uint64_t h) { return h & (TTSZ - 1); }
size_t tt_index(uint64_t h) { return h % TTSZ; }

// 01 function alphabeta(node, depth, α, β, maximizingPlayer)
// 02      if depth = 0 or node is a terminal node
// 03          return the heuristic value of node
// 04      if maximizingPlayer
// 05          v := -∞
// 06          for each child of node
// 07              v := max(v, alphabeta(child, depth – 1, α, β, FALSE))
// 08              α := max(α, v)
// 09              if β ≤ α
// 10                  break (* β cut-off *)
// 11          return v
// 12      else
// 13          v := +∞
// 14          for each child of node
// 15              v := min(v, alphabeta(child, depth – 1, α, β, TRUE))
// 16              β := min(β, v)
// 17              if β ≤ α
// 18                  break (* α cut-off *)
// 19          return v

int alphabeta(struct position *restrict pos, uint64_t zhash, int depth,
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

int alphabeta_search(struct position *restrict pos, const move *restrict moves,
                     int nmoves, uint64_t zhash, int depth, int *score) {
    struct savepos sp;
    int best = pos->wtm == WHITE ? NEG_INFINITI : INFINITI;
    int bestmoveno = -1;
    int value;
    int i;

    if (pos->wtm == WHITE) {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value = alphabeta(pos, zhash, depth, NEG_INFINITI, INFINITI, 0);
            if (value > best) {
                bestmoveno = i;
                best = value;
            }
            undo_move(pos, &sp, moves[i], 0);
        }
    } else {
        for (i = 0; i < nmoves; ++i) {
            make_move(pos, &sp, moves[i], 0);
            value = alphabeta(pos, zhash, depth, NEG_INFINITI, INFINITI, 1);
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

move search(const struct position *restrict const p, int *score,
            int *searched_depth) {
    const int max_depth = 5;
    struct position pos;
    move moves[MAX_MOVES];
    int nmoves;
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
        moveno = alphabeta_search(&pos, &moves[0], nmoves, zhash, depth, score);
        if (*score == INFINITI || *score == NEG_INFINITI) {
            break;
        }
    }

    assert(moveno >= 0 && moveno < nmoves);
    return moves[moveno];
}
