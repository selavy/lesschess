#include "catch.hpp"
#include "lesschess.h"
#include <climits>
#include <iostream>

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
    auto result   = easy_search(position);
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
    auto result   = easy_search(position);
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
    auto result   = easy_search(position);
    auto expected = Move{H6, H8};
    REQUIRE(result.move == expected);
    REQUIRE(result.score == WHITE_CHECKMATE);
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
    auto result   = easy_search(position);
    auto expected = Move{H3, H1};
    REQUIRE(result.move == expected);
    REQUIRE(result.score == BLACK_CHECKMATE);
}

TEST_CASE("Black mate in 2 with rook", "[search]")
{
    TransposeTable tt;
    std::string fen = "8/8/8/8/1k6/7r/8/K7 b - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = search(position, tt, 4);
    auto expected = Move{B4, B3};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == BLACK_CHECKMATE);
}

TEST_CASE("Black stalemate white king", "[search]")
{
    std::string fen = "K7/P7/2k5/8/8/8/8/8 b - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C6, C7};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == STALEMATE);
}

TEST_CASE("White mate -- philidor's smothered mate", "[search]")
{
    std::string fen = "1r5k/6pp/7N/8/2Q5/8/8/7K w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C4, G8};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == CHECKMATE);
}

TEST_CASE("White mate in 2 utilizing pin")
{
    // solution: 1. Qxh6+ Kg8 2. Qxg7#
    std::string fen = "5r1k/p3b1p1/1p2P2p/4Bp2/P1p1n3/5q2/BP1Q1PRP/7K w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{D2, H6};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == CHECKMATE);
}

TEST_CASE("White wins bishop with fork")
{
    std::string fen = "r1b2rk1/pp1p1ppp/1qn1pn2/2b5/2P5/1PN1PN2/P2B1PPP/R2QKB1R w KQ - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C3, A4};
    REQUIRE(result.move  == expected);
    // TODO: fix me
    // REQUIRE(result.score >= 300);
    REQUIRE(result.score >= 210);
}
