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

void dump_pv(Moves& pv) {
    for (auto m : pv) {
        std::cout << m.to_long_algebraic_string() << " ";
    }
    std::cout << "\n";
}

void print_tabs(int depth_left) {
    for (int i = 0; i < (3 - depth_left); ++i) {
        std::cout << '\t';
    }
}

int alpha_beta(Position& position, int alpha, int beta, int depth_left, Move move, Moves& pv)
{
    if (depth_left == 0) {
        int score = evaluate(position);
        std::cout << "\t\t\t\tevaluate for move=" << move.to_long_algebraic_string() << " "
            << "score=" << score << " -- ";
            dump_pv(pv);
        // return evaluate(position);
        return score;
    }

    print_tabs(depth_left);
    std::cout << "alpha_beta, "
        << "move=" << move.to_long_algebraic_string() << " "
        << "wtm=" << position.white_to_move() << " "
        << "alpha=" << alpha << " "
        << "beta=" << beta << " "
        << "depthLeft=" << depth_left << " "
        << "\n";

    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push_back(moves[i]);
        int score = -alpha_beta(position, /*alpha*/-beta, /*beta*/-alpha, depth_left - 1, moves[i], pv);
        pv.pop_back();
        position.undo_move(sp, moves[i]);
        if (score >= beta) {
            // return beta;
            alpha = beta;
            break;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    print_tabs(depth_left);
    std::cout << "leaving alpha_beta, alpha=" << alpha << " ";
    dump_pv(pv);
    std::cout << "\n";

    return alpha;
}

SearchResult search(Position& position)
{
    int bestmove = -1;
    int bestscore = position.white_to_move() ? -INFINITY : INFINITY;
    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    Moves pv;
    for (int i = 0;i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push_back(moves[i]);

        std::cout << "evaluating root " << moves[i].to_long_algebraic_string() << "\n";

        // TODO: don't think I should be negating here
        int score = -alpha_beta(position, /*alpha*/-INFINITY, /*beta*/INFINITY, /*depth*/ 3, moves[i], pv);

        std::cout << "score for " << moves[i].to_long_algebraic_string() << ": "
            << "score=" << score << " "
            << "bestScore=" << bestscore << " "
            << "\n";

        position.undo_move(sp, moves[i]);
        pv.pop_back();

        if (position.white_to_move() && score > bestscore) {
            bestscore = score;
            bestmove = i;
        }
        if (!position.white_to_move() && score < bestscore) {
            bestscore = score;
            bestmove = i;
        }
    }
    assert(bestmove != -1);
    return {moves[bestmove], bestscore};
}

#if 0
template <Color color>
constexpr bool _is_better_score(int lhs, int rhs) noexcept {
    return color == WHITE ? lhs > rhs : lhs < rhs;
}

int alphabeta_max(Position& position, int alpha, int beta, int depth);
int alphabeta_min(Position& position, int alpha, int beta, int depth);

int alphabeta_max(Position& position, int alpha, int beta, int depth) {
    if (depth == 0) {
        return evaluate(position);
    }

    Moves   moves{256};
    Savepos sp;
    int     nmoves;
    int     score;

    nmoves = position.generate_legal_moves(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        score = alphabeta_min(position, alpha, beta, depth - 1);
        position.undo_move(sp, moves[i]);
        if (score >= beta)
            return beta;
        if (score >  alpha)
            alpha = score;
    }
    return alpha;
}

int alphabeta_min(Position& position, int alpha, int beta, int depth) {
    if (depth == 0) {
        return evaluate(position);
    }

    Moves   moves{256};
    Savepos sp;
    int     nmoves;
    int     score;

    nmoves = position.generate_legal_moves(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        score = alphabeta_max(position, alpha, beta, depth - 1);
        position.undo_move(sp, moves[i]);
        if (score <= alpha)
            return alpha;
        if (score <  beta)
            beta = score;
    }
    return beta;
}

SearchResult search(Position& position) {
    auto&& is_better_score = position.white_to_move()
        ? _is_better_score<WHITE> : _is_better_score<BLACK>;
    auto&&  alphabeta = position.white_to_move() ? alphabeta_max : alphabeta_min;
    Savepos sp;
    Moves   moves{256};
    int     bestmove = -1;
    int     bestscore = position.white_to_move() ? INT_MIN : INT_MAX;
    int     nmoves;
    int     score;

    nmoves = position.generate_legal_moves(&moves[0]);

    // DEBUG
    std::cout << "info string found " << nmoves << " moves" << std::endl;

    for (int i = 0; i < nmoves; ++i) {
        // DEBUG
        std::cout << "info "
            << "currmove " << moves[i].to_long_algebraic_string() << " "
            << "currmovenumber " << i + 1
            << std::endl;

        position.make_move(sp, moves[i]);
        // int score = evaluate(position);
        score = alphabeta(position, INT_MIN, INT_MAX, 4);
        position.undo_move(sp, moves[i]);

        // DEBUG
        std::cout << "info score cp " << score << std::endl;

        if (is_better_score(score, bestscore)) {
            bestscore = score;
            bestmove  = i;
        }
        assert(bestmove != -1);

        // DEBUG
        std::cout << "info string "
            << "bestmove index = " << bestmove << " "
            << "bestmove = " << moves[bestmove].to_long_algebraic_string() << " "
            << "score = " << bestscore
            << std::endl;
    }

    return SearchResult{moves[bestmove], bestscore};
}
#endif

} // ~namespace lesschess
