#include "catch.hpp"
#include "lesschess.h"
#include <climits>
#include <iostream>

using namespace lesschess;

// TODO: remove? how to make this work when eval function changes?
// maybe can make it just a lower bound of what the score should be
TEST_CASE("Basic white eval", "[search]")
{
    Zobrist::initialize();
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
    Zobrist::initialize();
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
    Zobrist::initialize();
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
    Zobrist::initialize();
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
    Zobrist::initialize();
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
    Zobrist::initialize();
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
    Zobrist::initialize();
    std::string fen = "8/8/8/8/1k6/7r/8/K7 b - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{B4, B3};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == BLACK_CHECKMATE);
}

TEST_CASE("Black stalemate white king", "[search]")
{
    Zobrist::initialize();
    std::string fen = "K7/P7/2k5/8/8/8/8/8 b - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C6, C7};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == STALEMATE);
}

TEST_CASE("White mate -- philidor's smothered mate", "[search]")
{
    Zobrist::initialize();
    std::string fen = "1r5k/6pp/7N/8/2Q5/8/8/7K w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C4, G8};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == CHECKMATE);
}

TEST_CASE("White mate in 2 utilizing pin")
{
    Zobrist::initialize();
    // solution: 1. Qxh6+ Kg8 2. Qxg7#
    std::string fen = "5r1k/p3b1p1/1p2P2p/4Bp2/P1p1n3/5q2/BP1Q1PRP/7K w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{D2, H6};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score == CHECKMATE);
}

TEST_CASE("Knight fork reduced")
{
    Zobrist::initialize();
    std::string fen = "4k3/p3ppp1/1qp5/p1bp4/1pP5/1PN1P2P/P2P1PP1/3QK3 w - - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C3, A4};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score >= 300);
}

TEST_CASE("White wins bishop with fork")
{
    Zobrist::initialize();
    std::string fen = "r1b2rk1/pp1p1ppp/1qn1pn2/2b5/2P5/1PN1PN2/P2B1PPP/R2QKB1R w KQ - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C3, A4};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score >= 300);
}

TEST_CASE("Black wins bishop with fork")
{
    Zobrist::initialize();
    std::string fen = "r2qkb1r/p2b1ppp/1pn1pn2/2p5/2B5/1QN1PN2/PP1P1PPP/R1B2RK1 b kq - 0 1";
    auto position = Position::from_fen(fen);
    auto result   = easy_search(position);
    auto expected = Move{C6, A5};
    REQUIRE(result.move  == expected);
    REQUIRE(result.score <= -300);
}

TEST_CASE("Tactics")
{
    Zobrist::initialize();

    SECTION("White knight fork of king and rook")
    {
        std::string fen = "2r3k1/R7/8/5N2/8/8/8/6K1 w - - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto expected = Move{F5, E7};
        REQUIRE(result.move  == expected);
        REQUIRE(result.score >= 800);
    }

    SECTION("White mate in 2 with knights")
    {
        std::string fen = "2r4k/R7/8/4NN2/8/8/8/6K1 w - - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto expected = Move{E5, G6};
        REQUIRE(result.move  == expected);
        REQUIRE(result.score == WHITE_CHECKMATE);
    }

    SECTION("Black mate in 2 with knights")
    {
        std::string fen = "7k/R7/5N2/4N3/4nn2/8/r7/7K b - - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto expected = Move{E4, G3};
        REQUIRE(result.move  == expected);
        REQUIRE(result.score == BLACK_CHECKMATE);
    }

    SECTION("White mate with bishop")
    {
        std::string fen = "r1b2bkr/ppp3pp/2n5/3qp3/2B5/8/PPPP1PPP/RNB1K2R w KQ - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto expected = Move{C4, D5};
        REQUIRE(result.move  == expected);
        REQUIRE(result.score == WHITE_CHECKMATE);
    }

    SECTION("White mate after queen sac")
    {
        std::string fen = "7n/3NR3/1P3p2/1p1kbN1B/1p6/1K6/6b1/1Q6 w - - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto expected = Move{B1, F1};
        REQUIRE(result.move  == expected);
        REQUIRE(result.score == WHITE_CHECKMATE);
    }

    std::vector<std::pair<std::string, Move>> white_checkmate_yacpdb_positions = {
        { "3R4/4K3/5p2/5p2/8/3BkNQ1/8/8 w - - 0 1",              Move{D3, E4} },
        { "3K4/4B3/3Rp3/8/4pk2/1Qp1Np2/2p2P2/2R5 w - - 0 1",     Move{D6, D7} },
        { "6r1/5Q2/1n1p2pB/4k2b/3b3r/8/1NBRp2N/1K2R3 w - - 0 1", Move{D2, D1} },
        { "8/1Rp5/K3P3/2B2Q2/n1kP4/P3r3/P3PN2/1N2bB2 w - - 0 1", Move{F2, D1} },
    };

    for (auto&& p : white_checkmate_yacpdb_positions) {
        auto&& fen = p.first;
        SECTION(fen) {
            auto position = Position::from_fen(fen);
            auto result   = easy_search(position);
            auto expected = p.second;
            REQUIRE(result.move  == expected);
            REQUIRE(result.score == WHITE_CHECKMATE);
        }
    }
}
