#include "catch.hpp"
#include "position.h"

TEST_CASE("Position from FEN") {
    SECTION("Starting position") {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(fen);

        REQUIRE(position.piece_on_square(A1) == Piece{WHITE, ROOK});
        REQUIRE(position.piece_on_square(B1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(C1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(D1) == Piece{WHITE, QUEEN});
        REQUIRE(position.piece_on_square(E1) == Piece{WHITE, KING});
        REQUIRE(position.piece_on_square(F1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(G1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(H1) == Piece{WHITE, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_2}.value()) == Piece{WHITE, PAWN});
        }

        REQUIRE(position.piece_on_square(A8) == Piece{BLACK, ROOK});
        REQUIRE(position.piece_on_square(B8) == Piece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(C8) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(D8) == Piece{BLACK, QUEEN});
        REQUIRE(position.piece_on_square(E8) == Piece{BLACK, KING});
        REQUIRE(position.piece_on_square(F8) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(G8) == Piece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(H8) == Piece{BLACK, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == Piece{BLACK, PAWN});
        }

        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
        REQUIRE(position.halfmoves == 0);
        REQUIRE(position.moves == 1);
    }

    SECTION("After 1.e4") {
        std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
		Position position = Position::from_fen(fen);

        REQUIRE(position.piece_on_square(A1) == Piece{WHITE, ROOK});
        REQUIRE(position.piece_on_square(B1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(C1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(D1) == Piece{WHITE, QUEEN});
        REQUIRE(position.piece_on_square(E1) == Piece{WHITE, KING});
        REQUIRE(position.piece_on_square(F1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(G1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(H1) == Piece{WHITE, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            if (file != FILE_E) {
                REQUIRE(position.piece_on_square(Square{file, RANK_2}.value()) == Piece{WHITE, PAWN});
            }
        }

        REQUIRE(position.piece_on_square(A8) == Piece{BLACK, ROOK});
        REQUIRE(position.piece_on_square(B8) == Piece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(C8) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(D8) == Piece{BLACK, QUEEN});
        REQUIRE(position.piece_on_square(E8) == Piece{BLACK, KING});
        REQUIRE(position.piece_on_square(F8) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(G8) == Piece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(H8) == Piece{BLACK, ROOK});
        for (u8 file = FILE_A; file < FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == Piece{BLACK, PAWN});
        }

    }
}
