#include "catch.hpp"
#include "perft.h"
#include "position.h"
#include <string>
#include <cinttypes>

using namespace lesschess;
using Expected = std::vector<std::pair<int, u64>>;

// TODO: better workaround for including commas in a macro
#define P(a, b) { a, b }

#define PERFT_TEST(name, fen, ...) TEST_CASE(name, "[perft]") { \
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
}


// https://www.chessprogramming.org/Perft_Results#Initial_Position
//
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
    // P(7, 3195901860),
    // P(8, 84998978956),
    // P(9, 2439530234167),
    // P(10, 69352859712417),
)

// https://www.chessprogramming.org/Perft_Results#Position_2
//
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
    // P(6, 8031647685),
)

// https://www.chessprogramming.org/Perft_Results#Position_3
//
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
    // P(8, 3009794393),
)

// https://www.chessprogramming.org/Perft_Results#Position_4
//
// |r| | | |k| | |r|
// |P|p|p|p| |p|p|p|
// | |b| | | |n|b|N|
// |n|P| | | | | | |
// |B|B|P| |P| | | |
// |q| | | | |N| | |
// |P|p| |P| | |P|P|
// |R| | |Q| |R|K| |
// w kq - 0 1
PERFT_TEST(
    "position-4-white",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    P(1, 6),
    P(2, 264),
    P(3, 9467),
    P(4, 422333),
    P(5, 15833292),
    P(6, 706045033),
)


// https://www.chessprogramming.org/Perft_Results#Position_4
//
// |r| | |q| |r|k| |
// |p|P| |p| | |p|p|
// |Q| | | | |n| | |
// |b|b|p| |p| | | |
// |N|p| | | | | | |
// | |B| | | |N|B|n|
// |p|P|P|P| |P|P|P|
// |R| | | |K| | |R|
// b KQ - 0 1
PERFT_TEST(
    "position-4-black",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
    P(1, 6),
    P(2, 264),
    P(3, 9467),
    P(4, 422333),
    P(5, 15833292),
    P(6, 706045033),
)

// https://www.chessprogramming.org/Perft_Results#Position_5
//
// |r|n|b|q| |k| |r|
// |p|p| |P|b|p|p|p|
// | | |p| | | | | |
// | | | | | | | | |
// | | |B| | | | | |
// | | | | | | | | |
// |P|P|P| |N|n|P|P|
// |R|N|B|Q|K| | |R|
// w KQ - 1 8
PERFT_TEST(
    "position-5",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    P(1, 44),
    P(2, 1486),
    P(3, 62379),
    P(4, 2103487),
    P(5, 89941194),
)

// https://www.chessprogramming.org/Perft_Results#Position_6
//
// |r| | | | |r|k| |
// | |p|p| |q|p|p|p|
// |p| |n|p| |n| | |
// | | |b| |p| |B| |
// | | |B| |P| |b| |
// |P| |N|P| |N| | |
// | |P|P| |Q|P|P|P|
// |R| | | | |R|K| |
// w - - 0 10
PERFT_TEST(
    "position-6",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    P(0, 1),
    P(1, 46),
    P(2, 2079),
    P(3, 89890),
    P(4, 3894594),
    // P(5, 164075551),
    // P(6, 6923051137),
    // P(7, 287188994746),
    // P(8, 11923589843526),
    // P(9, 490154852788714),
)

// http://www.rocechess.ch/perft.html
//
// |n| |n| | | | | |
// |P|P|P|k| | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | |K|p|p|p|
// | | | | | |N| |N|
// b - - 0 1
PERFT_TEST(
    "promotions",
    "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
    P(1, 24),
    P(2, 496),
    P(3, 9483),
    P(4, 182838),
    P(5, 3605103),
    P(6, 71179139),
)

// Roce chess engine perft test suite
// |r| | | |k| | |r|
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// | | | | | | | | |
// |R| | | |K| | |R|
// w KQkq - 0 1
PERFT_TEST(
    "rooks",
    "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
    P(1, 26),
    P(2, 568),
    P(3, 13744),
    P(4, 314346),
    P(5, 7594526),
    P(6, 179862938),
)
