#include "catch.hpp"
#include "perft.h"
#include "position.h"
#include <string>
#include <cinttypes>

using namespace lesschess;


#define PERFT_TEST(name, fen, ts) do { \
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

    Expected ts = {
        { 0, 1 },
        { 1, 20 },
        { 2, 400 },
        { 3, 8902 },
        { 4, 197281 },
        { 5, 4865609 },
        { 6, 119060324 },
        { 7, 3195901860 },
        // { 8, 84998978956 },
        // { 9, 2439530234167 },
        // { 10, 69352859712417 },
    };

    PERFT_TEST("starting-position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", ts);
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

    Expected ts = {
        { 1, 48 },
        { 2, 2039 },
        { 3, 97862 },
        { 4, 4085603 },
        { 5, 193690690 },
        { 6, 8031647685 },
    };

    PERFT_TEST("kiwi-pete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", ts);
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

    Expected ts = {
        { 1, 14 },
        { 2, 191 },
        { 3, 2812 },
        { 4, 43238 },
        { 5, 674624 },
        { 6, 11030083 },
        { 7, 178633661 },
        { 8, 3009794393 },
    };

    PERFT_TEST("position-3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", ts);
}
