#include "catch.hpp"
#include "move.h"

TEST_CASE("Move", "move representation construction checks") {
    SECTION("Normal move") {
        constexpr Move move{A1, A2};
        REQUIRE(move.from() == A1);
        REQUIRE(move.to()   == A2);
        static_assert(move.from() == A1, "");
        static_assert(move.to()   == A2, "");
        REQUIRE(move.flags() == Move::Flags::NONE);
    }

    SECTION("En Passant move") {
        constexpr Move move{E5, D6, ep_capture_tag{}};
        REQUIRE(move.from() == E5);
        REQUIRE(move.to()   == D6);
        static_assert(move.from() == E5, "");
        static_assert(move.to()   == D6, "");
        REQUIRE(move.flags() == Move::Flags::EP);
    }

    SECTION("Castle move") {
        constexpr Move move{E1, G1, castle_tag{}};
        REQUIRE(move.from() == E1);
        REQUIRE(move.to()   == G1);
        static_assert(move.from() == E1, "");
        static_assert(move.to()   == G1, "");
        REQUIRE(move.flags() == Move::Flags::CASTLE);
    }
}
