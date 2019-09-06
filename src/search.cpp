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

void dump_pv(Moves& pv) {
    for (auto m : pv) {
        std::cout << m.to_long_algebraic_string() << " ";
    }
    std::cout << "\n";
}

void print_tabs(int depth_left) {
    for (int i = 0; i < (MAX_DEPTH - depth_left); ++i) {
        std::cout << '\t';
    }
}

int alpha_beta(Position& position, int alpha, int beta, int depth, Move move, Moves& pv)
{
    // TODO: check if terminal node (mate, stalemate, etc)
    //       not sure if the fastest way to do that is to check if `nmoves == 0`?
    if (depth == 0) {
        int score = evaluate(position);
        for (int i = 0; i <= MAX_DEPTH; ++i)
            std::cout << '\t';
        std::cout << "evaluate for move=" << move.to_long_algebraic_string() << " "
            << "score=" << score << " -- ";
            dump_pv(pv);
        return position.white_to_move() ? score : -score;
    }

    print_tabs(depth);
    std::cout << "alpha_beta, "
        << "move=" << move.to_long_algebraic_string() << " "
        << "wtm=" << position.white_to_move() << " "
        << "alpha=" << alpha << " "
        << "beta=" << beta << " "
        << "depthLeft=" << depth << " "
        << "\n";

    int value = -INFINITY;
    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push_back(moves[i]);
        value = std::max(value, -alpha_beta(position, /*alpha*/-beta, /*beta*/-alpha, depth - 1, moves[i], pv));
        pv.pop_back();
        position.undo_move(sp, moves[i]);
        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }

    print_tabs(depth);
    std::cout << "leaving alpha_beta, value=" << value << " ";
    dump_pv(pv);

    return value;
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

        std::cout << "evaluating root " << moves[i].to_long_algebraic_string() << " "
            << "bestScore=" << bestscore << "\n";

        // TODO: don't think I should be negating here
        int score = -alpha_beta(position, /*alpha*/-INFINITY, /*beta*/INFINITY, /*depth*/MAX_DEPTH, moves[i], pv);

        std::cout << "leaving root " << moves[i].to_long_algebraic_string() << ": "
            << "score=" << score << " "
            << "bestScore=" << bestscore << " "
            << "\n";

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
