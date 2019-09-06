#include "search.h"
#include "evaluate.h"
#include <climits>
#include <array>
#include <cassert>
#include <functional>
#include <iostream> // TEMP

namespace lesschess {

// be careful to avoid overflow from INT_MIN == -INT_MIN
constexpr int INFINITY = INT_MAX - 1;
constexpr int DRAW = 0;
constexpr int CHECKMATE = INFINITY - 1; // TODO: does it matter if it is infinity or not?
constexpr int STALEMATE = DRAW;
constexpr int FIFTY_MOVE_RULE_DRAW = DRAW;

// Number of moves upper bound:
// K -> 8 + 2 castles
// Q -> 27
// R -> 14 x 2 = 28
// N -> 8 x 2 = 16
// P -> (2 moves + 2 captures) x 8 = 32 (same as if all 8 pawns could promote)
// --> 139
//
// Worst worst case is 7 promoted queens => 7 x 27 + 8 x 2 = 394
// 128 is probably safe? 256 is definitely safe since willing to bet the the 7 Qs
// position isn't going to happen -- will either be checkmate or stalemate.

// TODO: what is the maximum number of possible moves in a position?
using Moves = std::vector<Move>; // TODO: change this back to std::array<Move, 256>?

constexpr int MAX_DEPTH = 2;

int negamax(Position& position, int alpha, int beta, int depth, Moves& pv)
{
    // TODO: check if terminal node (mate, stalemate, etc)
    //       not sure if the fastest way to do that is to check if `nmoves == 0`?
    if (depth == 0) {
        int score = evaluate(position);
        return position.white_to_move() ? -score : score;
    }

    // TODO: check for 3-move repetition
    if (position.fifty_move_rule_moves() >= 50) {
        return FIFTY_MOVE_RULE_DRAW;
    }

    int value = -INFINITY;
    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    if (nmoves == 0) {
        // TODO: cache `checkers` from generate_legal_moves() so we can check if mate or stalemate
        if (position.in_check(position.color_to_move())) {
            return position.white_to_move() ? CHECKMATE : -CHECKMATE;
        } else {
            return STALEMATE;
        }
    }
    assert(nmoves > 0);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push_back(moves[i]);
        value = std::max(value, negamax(position, /*alpha*/-beta, /*beta*/-alpha, depth - 1, pv));
        pv.pop_back();
        position.undo_move(sp, moves[i]);
        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }
    return -value;
}

SearchResult search(Position& position)
{
    int bestmove = -1;
    int bestscore = -INFINITY;
    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    Moves pv;
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push_back(moves[i]);
        int score = negamax(position, /*alpha*/-INFINITY, /*beta*/INFINITY, /*depth*/MAX_DEPTH, pv);
        position.undo_move(sp, moves[i]);
        pv.pop_back();
        if (score > bestscore) {
            bestscore = score;
            bestmove = i;
        }
    }
    assert(bestmove != -1);
    bestscore = position.white_to_move() ? bestscore : -bestscore;
    return {moves[bestmove], bestscore};
}

} // ~namespace lesschess
