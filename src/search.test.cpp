#include "catch.hpp"
#include "lesschess.h"
#include <climits>

using namespace lesschess;

// TODO: remove? how to make this work when eval function changes?
// maybe can make it just a lower bound of what the score should be
TEST_CASE("Basic white eval", "[search]")
{
    // |k| | | | | |n| |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | |K| | | | |
    // | | | | | | | |R|
    // w - - 0 1
    std::string fen = "k5n1/8/8/8/8/8/3K4/7R w - - 0 1";
    auto position = Position::from_fen(fen);
    int score = evaluate(position);
    int expected = 500 - 300;
    REQUIRE(score == expected);
}

TEST_CASE("Basic black eval", "[search]")
{
    // | | | | |k| | | |
    // | | | | | | | |n|
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | |K| | | |
    // w - - 0 1
    std::string fen = "4k3/7n/8/8/8/8/8/4K3 w - - 0 1";
    auto position = Position::from_fen(fen);
    int score = evaluate(position);
    int expected = -300;
    REQUIRE(score == expected);
}

TEST_CASE("Win knight - wtm", "[search]")
{
    // Wanting to see white play Rh8, which pins the knight to the king, then capture
    // on the next move.
    //
    // |k| | | | | |n| |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | |K| | | | |
    // | | | | | | | |R|
    // w - - 0 1
    std::string fen = "k5n1/8/8/8/8/8/3K4/7R w - - 0 1";

    auto position = Position::from_fen(fen);
    auto result   = search(position);
    auto expected = Move{H1, H8};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == 500); // TODO: make into lower bound
}

TEST_CASE("Win knight - btm", "[search]")
{
    // Wanting to see black play Rh1, which pins the knight to the king, then capture
    // on the next move.
    //
    // |k| | | | | | |r|
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // |K| | | | | |N| |
    // b - - 0 1
    std::string fen = "k6r/8/8/8/8/8/8/K5N1 b - - 0 1";

    auto position = Position::from_fen(fen);
    auto result   = search(position);
    auto expected = Move{H8, H1};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == -500); // TODO: make into upper bound
}

TEST_CASE("White mate in 1 with rook", "[search]")
{
    // |k| | | | | | | |
    // | | | | | | | | |
    // |K| | | | | | |R|
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // w - - 0 1
    std::string fen = "k7/8/K6R/8/8/8/8/8 w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = search(position);
    auto expected = Move{H6, H8};
    REQUIRE(result.move == expected);
}

TEST_CASE("Black mate in 1 with rook", "[search]")
{
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // |k| | | | | | |r|
    // | | | | | | | | |
    // |K| | | | | | | |
    // b - - 0 1
    std::string fen = "8/8/8/8/8/k6r/8/K7 b - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = search(position);
    auto expected = Move{H3, H1};
    REQUIRE(result.move == expected);
}
