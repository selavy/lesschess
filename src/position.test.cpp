#include "catch.hpp"
#include "position.h"
#include <iostream>

TEST_CASE("Position from FEN") {
    SECTION("Sizeof") {
        Position pos;
        std::cout << "sizeof(pos.bbrd_)    = " << sizeof(pos.bbrd_) << "\n";
        std::cout << "sizeof(pos.bbrd_[0]) = " << sizeof(pos.bbrd_[0]) << "\n";
    }

    SECTION("Starting position") {
        auto maybe_pos = Position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        REQUIRE(static_cast<bool>(maybe_pos) == true);
        const Position& pos = *maybe_pos;
        REQUIRE(pos.castle() == Position::CASTLE_ALL);
        REQUIRE(pos.epsq() > H6);
        REQUIRE(pos.piece_on_square(A1) == ColorPiece{WHITE, ROOK});
    }
}
