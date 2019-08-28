#include "search.h"
#include "evaluate.h"
#include <climits>
#include <array>
#include <cassert>
#include <functional>
#include <iostream> // TEMP

namespace lesschess {

using Moves = std::vector<Move>; // TODO: change this back to std::array<Move, 256>?

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

} // ~namespace lesschess
