#include "catch.hpp"
#include "lesschess.h"

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

#if 0
TEST_CASE("Win knight", "[search]")
{
    // Wanting to see white play Rh8, which pins the knight to the pin, then captures
    // on the next more.
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
    REQUIRE(result.move == expected);
    REQUIRE(result.score >= 500);

}
#endif
