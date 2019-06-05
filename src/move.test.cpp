#include "catch.hpp"
#include "move.h"
#include <cstring>

using namespace lesschess;

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

     SECTION("White King side castle move") {
         constexpr Move move = Move::make_castle(Move::CastleKind::WHITE_KING_SIDE);
         REQUIRE(move.from() == E1);
         REQUIRE(move.to()   == H1);
         static_assert(move.from() == E1, "");
         static_assert(move.to()   == H1, "");
         REQUIRE(move.flags() == Move::Flags::CASTLE);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == true);
         REQUIRE(move.is_promotion() == false);
     }

     SECTION("White Queen side castle move") {
         constexpr Move move = Move::make_castle(Move::CastleKind::WHITE_QUEEN_SIDE);
         REQUIRE(move.from() == E1);
         REQUIRE(move.to()   == A1);
         static_assert(move.from() == E1, "");
         static_assert(move.to()   == A1, "");
         REQUIRE(move.flags() == Move::Flags::CASTLE);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == true);
         REQUIRE(move.is_promotion() == false);
     }


     SECTION("Black King side castle move") {
         constexpr Move move = Move::make_castle(Move::CastleKind::BLACK_KING_SIDE);
         REQUIRE(move.from() == E8);
         REQUIRE(move.to()   == H8);
         static_assert(move.from() == E8, "");
         static_assert(move.to()   == H8, "");
         REQUIRE(move.flags() == Move::Flags::CASTLE);
         REQUIRE(move.is_enpassant() == false);
         REQUIRE(move.is_castle()    == true);
         REQUIRE(move.is_promotion() == false);
     }

     SECTION("Black Queen side castle move") {
         constexpr Move move = Move::make_castle(Move::CastleKind::BLACK_QUEEN_SIDE);
         REQUIRE(move.from() == E8);
         REQUIRE(move.to()   == A8);
         static_assert(move.from() == E8, "");
         static_assert(move.to()   == A8, "");
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

TEST_CASE("Piece", "piece and color") {
    SECTION("Can recover piece kind") {
        REQUIRE(Piece{BLACK, BISHOP}.kind() == BISHOP);
        REQUIRE(Piece{BLACK, KNIGHT}.kind() == KNIGHT);
        REQUIRE(Piece{BLACK, PAWN}  .kind() == PAWN);
        REQUIRE(Piece{WHITE, PAWN}  .kind() == PAWN);
        REQUIRE(Piece{WHITE, KING}  .kind() == KING);
    }

    SECTION("Can recover color") {
        REQUIRE(Piece{BLACK, BISHOP}.color() == BLACK);
        REQUIRE(Piece{BLACK, KNIGHT}.color() == BLACK);
        REQUIRE(Piece{BLACK, PAWN}  .color() == BLACK);
        REQUIRE(Piece{WHITE, PAWN}  .color() == WHITE);
        REQUIRE(Piece{WHITE, KING}  .color() == WHITE);
    }

    SECTION("Piece name") {
        REQUIRE(strcmp(Piece{WHITE, PAWN}  .name(), "white pawn") == 0);
        REQUIRE(strcmp(Piece{WHITE, KNIGHT}.name(), "white knight") == 0);
        REQUIRE(strcmp(Piece{WHITE, BISHOP}.name(), "white bishop") == 0);
        REQUIRE(strcmp(Piece{WHITE, ROOK}  .name(), "white rook") == 0);
        REQUIRE(strcmp(Piece{WHITE, QUEEN} .name(), "white queen") == 0);
        REQUIRE(strcmp(Piece{WHITE, KING}  .name(), "white king") == 0);

        REQUIRE(strcmp(Piece{BLACK, PAWN}  .name(), "black pawn") == 0);
        REQUIRE(strcmp(Piece{BLACK, KNIGHT}.name(), "black knight") == 0);
        REQUIRE(strcmp(Piece{BLACK, BISHOP}.name(), "black bishop") == 0);
        REQUIRE(strcmp(Piece{BLACK, ROOK}  .name(), "black rook") == 0);
        REQUIRE(strcmp(Piece{BLACK, QUEEN} .name(), "black queen") == 0);
        REQUIRE(strcmp(Piece{BLACK, KING}  .name(), "black king") == 0);

        REQUIRE(strcmp(Piece{}.name(), "empty") == 0);
    }
}
