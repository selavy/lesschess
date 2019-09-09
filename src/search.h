#pragma once

#include "position.h"
#include <climits>

namespace lesschess {

// be careful to avoid overflow from INT_MIN == -INT_MIN
constexpr int MAX_SCORE = INT_MAX - 1;
constexpr int DRAW = 0;
constexpr int CHECKMATE = MAX_SCORE - 1; // TODO: does it matter if it is infinity or not?
constexpr int STALEMATE = DRAW;
constexpr int FIFTY_MOVE_RULE_DRAW = DRAW;
constexpr int WHITE_CHECKMATE = CHECKMATE;
constexpr int BLACK_CHECKMATE = -CHECKMATE;

struct SearchResult {
    constexpr SearchResult() noexcept : score{0}, move{} {}
    constexpr SearchResult(Move m, int s) noexcept : score{s}, move{m} {}

    int  score;
    Move move;
};

SearchResult search(Position& position, TransposeTable& tt);

SearchResult easy_search(Position& position);

} // namespace lesschess
