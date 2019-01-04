#include "catch.hpp"
#include "move.h"
#include <cstring>

TEST_CASE("Move", "move representation construction checks") {
     SECTION("Normal move") {
         constexpr Move move{A1, A2};
         REQUIRE(move.from() == A1);
         REQUIRE(move.to()   == A2);
         static_assert(move.from() == A1, "");
         static_assert(move.to()   == A2, "");
         REQUIRE(move.flags() == Move::Flags::NONE);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == false);
         REQUIRE(move.is_promotion() == false);
     }

     SECTION("En Passant move") {
         constexpr Move move{E5, D6, ep_capture_tag{}};
         REQUIRE(move.from() == E5);
         REQUIRE(move.to()   == D6);
         static_assert(move.from() == E5, "");
         static_assert(move.to()   == D6, "");
         REQUIRE(move.flags() == Move::Flags::ENPASSANT);
         REQUIRE(move.is_enpassant() == true);
         REQUIRE(move.is_castle()    == false);
         REQUIRE(move.is_promotion() == false);
     }

     SECTION("Castle move") {
         constexpr Move move{E1, G1, castle_tag{}};
         REQUIRE(move.from() == E1);
         REQUIRE(move.to()   == G1);
         static_assert(move.from() == E1, "");
         static_assert(move.to()   == G1, "");
         REQUIRE(move.flags() == Move::Flags::CASTLE);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == true);
         REQUIRE(move.is_promotion() == false);
     }

     SECTION("Promotion move") {
         constexpr Move move{E2, E1, ROOK};
         REQUIRE(move.from() == E2);
         REQUIRE(move.to()   == E1);
         static_assert(move.from() == E2, "");
         static_assert(move.to()   == E1, "");
         REQUIRE(move.flags() == Move::Flags::PROMOTION);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == false);
         REQUIRE(move.is_promotion() == true);
         REQUIRE(move.promotion() == ROOK);
     }
}

TEST_CASE("Square", "square from file, rank or value, etc") {
    SECTION("Convert from file and rank") {
        REQUIRE(strcmp(Square{FILE_A, RANK_1}.name(), "a1") == 0);
        REQUIRE(strcmp(Square{FILE_E, RANK_1}.name(), "e1") == 0);
        REQUIRE(strcmp(Square{FILE_F, RANK_8}.name(), "f8") == 0);
        REQUIRE(strcmp(Square{FILE_B, RANK_7}.name(), "b7") == 0);
        REQUIRE(strcmp(Square{FILE_C, RANK_5}.name(), "c5") == 0);

        REQUIRE(Square{FILE_A, RANK_1}.value() == 0);
        REQUIRE(Square{FILE_H, RANK_8}.value() == 63);
    }

    SECTION("Can recover file") {
        REQUIRE(Square{FILE_A, RANK_1}.file() == FILE_A);
        REQUIRE(Square{FILE_E, RANK_1}.file() == FILE_E);
        REQUIRE(Square{FILE_E, RANK_4}.file() == FILE_E);
        REQUIRE(Square{FILE_E, RANK_8}.file() == FILE_E);
        REQUIRE(Square{FILE_B, RANK_7}.file() == FILE_B);
    }

    SECTION("Can recover rank") {
        REQUIRE(Square{FILE_A, RANK_1}.rank() == RANK_1);
        REQUIRE(Square{FILE_E, RANK_1}.rank() == RANK_1);
        REQUIRE(Square{FILE_E, RANK_4}.rank() == RANK_4);
        REQUIRE(Square{FILE_E, RANK_8}.rank() == RANK_8);
        REQUIRE(Square{FILE_B, RANK_7}.rank() == RANK_7);
    }
}
