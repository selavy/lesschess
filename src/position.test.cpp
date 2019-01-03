#include "catch.hpp"
#include "position.h"

TEST_CASE("Position from FEN") {
    SECTION("Starting position") {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(fen);

        REQUIRE(position.piece_on_square(A1) == ColorPiece{WHITE, ROOK});
        REQUIRE(position.piece_on_square(B1) == ColorPiece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(C1) == ColorPiece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(D1) == ColorPiece{WHITE, QUEEN});
        REQUIRE(position.piece_on_square(E1) == ColorPiece{WHITE, KING});
        REQUIRE(position.piece_on_square(F1) == ColorPiece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(G1) == ColorPiece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(H1) == ColorPiece{WHITE, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_2}.value()) == ColorPiece{WHITE, PAWN});
        }

        REQUIRE(position.piece_on_square(A8) == ColorPiece{BLACK, ROOK});
        REQUIRE(position.piece_on_square(B8) == ColorPiece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(C8) == ColorPiece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(D8) == ColorPiece{BLACK, QUEEN});
        REQUIRE(position.piece_on_square(E8) == ColorPiece{BLACK, KING});
        REQUIRE(position.piece_on_square(F8) == ColorPiece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(G8) == ColorPiece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(H8) == ColorPiece{BLACK, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == ColorPiece{BLACK, PAWN});
        }

        REQUIRE(position.wtm() == true);
        REQUIRE(position.castle() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
        REQUIRE(position.halfmoves_ == 0);
        REQUIRE(position.moves_ == 1);
    }
}
