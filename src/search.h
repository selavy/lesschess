#pragma once

#include "position.h"
#include <climits>
#include <array>

namespace lesschess {

// be careful to avoid overflow from INT_MIN == -INT_MIN
constexpr int MAX_SCORE = INT_MAX - 1;
constexpr int DRAW = 0;
constexpr int CHECKMATE = MAX_SCORE - 1; // TODO: does it matter if it is infinity or not?
constexpr int STALEMATE = DRAW;
constexpr int FIFTY_MOVE_RULE_DRAW = DRAW;
constexpr int WHITE_CHECKMATE = CHECKMATE;
constexpr int BLACK_CHECKMATE = -CHECKMATE;
constexpr int MAX_DEPTH = 32;

template <int N>
struct PrimaryVariation {
    void push(Move m) noexcept { assert(count < moves.size()); moves[count++] = m; }
    void pop() noexcept { --count; }
    Move* begin() noexcept { return &moves[0]; }
    Move* end() noexcept { return &moves[count]; }
    Move& back() noexcept { assert(count > 0); return moves[count-1]; }
    const Move* begin() const noexcept { return begin(); }
    const Move* end() const noexcept { return end(); }
    void dump() const;

    constexpr PrimaryVariation() noexcept : moves{} {}
    PrimaryVariation(const PrimaryVariation& other) noexcept
        : moves{other.moves}, count{count} {}
    PrimaryVariation(PrimaryVariation&& other) noexcept
        : moves{std::move(other.moves)}, count{count} {}

    // TODO: figure out making it work for any M, M >= N, makes it think the copy assignment
    //       is deleted
    // template <int M>
    constexpr PrimaryVariation<N>& operator=(const PrimaryVariation<N>& other) noexcept {
        // static_assert(N >= M, "not enough room in destination primary variation");
        count = other.count;
        for (int i = 0; i < other.count; ++i) {
            moves[i] = other.moves[i];
        }
        return *this;
    }

    std::array<Move, N> moves;
    int                 count = 0;
};

using PV = PrimaryVariation<MAX_DEPTH>;

struct SearchResult {
    constexpr SearchResult() noexcept : score{0}, move{} {}
    constexpr SearchResult(Move m, int s) noexcept : score{s}, move{m} {}
    int  score;
    Move move;
};

SearchResult search(Position& position, TransposeTable& tt, int max_depth, bool useTT);

SearchResult easy_search(Position& position, bool useTT = true);

} // namespace lesschess
