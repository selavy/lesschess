#include "search.h"
#include "evaluate.h"
#include <climits>
#include <array>
#include <cassert>
#include <functional>
#include <iostream> // TEMP

namespace lesschess {

template <Color color>
constexpr bool _is_better_score(int lhs, int rhs) noexcept {
    return color == WHITE ? lhs > rhs : lhs < rhs;
}

SearchResult search(Position& position) {
    auto&& is_better_score = position.white_to_move() ? _is_better_score<WHITE> : _is_better_score<BLACK>;
    Savepos sp;
    // TODO: how to ensure below stack size?
    // std::array<Move, 256> moves;
    std::vector<Move> moves{256};
    int bestmove = -1;
    int bestscore = position.white_to_move() ? INT_MIN : INT_MAX;
    int nmoves = position.generate_legal_moves(&moves[0]);

    // DEBUG
    std::cout << "info string found " << nmoves << " moves" << std::endl;

    for (int i = 0; i < nmoves; ++i) {
        // DEBUG
        std::cout << "info "
            << "currmove " << moves[i].to_long_algebraic_string() << " "
            << "currmovenumber " << i + 1
            << std::endl;

        position.make_move(sp, moves[i]);
        int score = evaluate(position);
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
