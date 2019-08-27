#pragma once

#include "position.h"

namespace lesschess {

struct SearchResult {
    constexpr SearchResult() noexcept : score{0}, move{} {}
    constexpr SearchResult(Move m, int s) noexcept : score{s}, move{m} {}

    int  score;
    Move move;
};

SearchResult search(Position& position);

} // namespace lesschess
