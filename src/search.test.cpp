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
        { "3R4/4K3/5p2/5p2/8/3BkNQ1/8/8",                    Move{D3, E4} },
        { "3K4/4B3/3Rp3/8/4pk2/1Qp1Np2/2p2P2/2R5",           Move{D6, D7} },
        { "6r1/5Q2/1n1p2pB/4k2b/3b3r/8/1NBRp2N/1K2R3",       Move{D2, D1} },
        { "8/1Rp5/K3P3/2B2Q2/n1kP4/P3r3/P3PN2/1N2bB2",       Move{F2, D1} },
        { "3KN3/2B2R2/6n1/8/4k2n/4p3/4B3/3R4",               Move{C7, B8} },
        { "5B2/3p1n2/R2p4/1P1NRBQ1/1KPkrb2/1p6/2Pp1Pn1/4r3", Move{D5, C7} },
        { "rqr5/1B1N3p/2n1p2K/b2P4/RB1Nkn1R/1P6/2P2PP1/1Q6", Move{D4, E2} },
        { "2B5/8/4pN1K/R1B1qkP1/4p3/7p/5P1P/4Q3",            Move{C5, E3} },
        { "5Q2/8/8/8/8/5p2/2N1Np2/2K2k2",                    Move{F8, H6} },
        { "8/8/1R6/2p5/8/2Bk1NRN/3P4/K6B",                   Move{B6, B1} },
        { "8/8/8/p7/kPr4R/N1P5/2P5/1K5Q",                    Move{H1, D5} },
        { "3Rb3/B4NpB/4N1n1/3P1P1R/1r2kP2/1p1np3/8/3Q2K1",   Move{G1, H2} },
        { "7Q/3B4/1B3R2/3pk3/1Nr1P2r/2n5/5nq1/K2R4",         Move{B6, E3} },
    };

    for (auto&& p : white_checkmate_yacpdb_positions) {
        auto fen = p.first + " w - - 0 1";
        SECTION(fen) {
            auto position = Position::from_fen(fen);
            auto result   = easy_search(position);
            auto expected = p.second;
            REQUIRE(result.move  == expected);
            REQUIRE(result.score == WHITE_CHECKMATE);
        }
    }

    SECTION("promotion leading to mate tactic") {
        std::string fen = "8/3k1P2/1K3B2/3B4/8/8/8/8 w - - 0 1";
        auto position = Position::from_fen(fen);
        auto result   = easy_search(position);
        auto ex1 = Move::make_promotion(F7, F8, QUEEN);
        auto ex2 = Move::make_promotion(F7, F8, ROOK);
        REQUIRE((result.move  == ex1 || result.move == ex2));
        REQUIRE(result.score == WHITE_CHECKMATE);
    }
}
