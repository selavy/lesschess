#include "catch.hpp"
#include "perft.h"
#include "position.h"
#include <string>
#include <cinttypes>

using namespace lesschess;

// TODO: better workaround for including commas in a macro
#define P(a, b) { a, b }

#define PERFT_TEST(name, fen, ...) do { \
    Expected ts = Expected{__VA_ARGS__}; \
    Position position = Position::from_fen(fen); \
    for (auto& t : ts) { \
        auto depth = std::get<0>(t); \
        auto nodes = std::get<1>(t); \
        auto result = perft_speed(position, depth); \
        printf(name ": depth(%d) nodes(%" PRIu64 ")\n", depth, nodes); \
        REQUIRE(position == Position::from_fen(fen)); \
        REQUIRE(result == nodes); \
    } \
} while(0)

using Expected = std::vector<std::pair<int, u64>>;

TEST_CASE("Starting Position", "[perft]")
{

    // |r|n|b|q|k|b|n|r|
    // |p|p|p|p|p|p|p|p|
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // | | | | | | | | |
    // |P|P|P|P|P|P|P|P|
    // |R|N|B|Q|K|B|N|R|
    // w KQkq - 0 1

    PERFT_TEST(
        "starting-position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        P(0, 1),
        P(1, 20),
        P(2, 400),
        P(3, 8902),
        P(4, 197281),
        P(5, 4865609),
        P(6, 119060324),
        P(7, 3195901860),
        // P(8, 84998978956),
        // P(9, 2439530234167),
        // P(10, 69352859712417),
    );
}

TEST_CASE("Kiwipete", "[perft]")
{
    // |r| | | |k| | |r|
    // |p| |p|p|q|p|b| |
    // |b|n| | |p|n|p| |
    // | | | |P|N| | | |
    // | |p| | |P| | | |
    // | | |N| | |Q| |p|
    // |P|P|P|B|B|P|P|P|
    // |R| | | |K| | |R|
    // w KQkq - 0 1

    PERFT_TEST(
        "kiwi-pete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        P(1, 48),
        P(2, 2039),
        P(3, 97862),
        P(4, 4085603),
        P(5, 193690690),
        P(6, 8031647685),
    );
}

TEST_CASE("Position 3", "[perft]")
{
    // | | | | | | | | |
    // | | |p| | | | | |
    // | | | |p| | | | |
    // |K|P| | | | | |r|
    // | |R| | | |p| |k|
    // | | | | | | | | |
    // | | | | |P| |P| |
    // | | | | | | | | |
    // w - - 0 1

    PERFT_TEST(
        "position-3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        P(1, 14),
        P(2, 191),
        P(3, 2812),
        P(4, 43238),
        P(5, 674624),
        P(6, 11030083),
        P(7, 178633661),
        P(8, 3009794393),
    );
}
