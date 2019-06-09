#include "catch.hpp"
#include "perft.h"
#include "position.h"
#include <string>

using namespace lesschess;

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
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position position = Position::from_fen(fen);

    std::vector<std::pair<int, u64>> ts = {
        { 0, 1 },
        { 1, 20 },
        { 2, 400 },
        { 3, 8902 },
        { 4, 197281 },
    };

    for (auto& t : ts) {
        auto depth = std::get<0>(t);
        auto nodes = std::get<1>(t);
        auto result = perft_speed(position, depth);
        REQUIRE(position == Position::from_fen(fen));
        REQUIRE(result == nodes);
    }
}
