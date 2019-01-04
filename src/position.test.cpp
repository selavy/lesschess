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
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
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
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == Piece{BLACK, PAWN});
        }

        for (u8 rank = RANK_3; rank <= RANK_6; ++rank) {
            for (u8 file = FILE_A; file <= FILE_H; ++file) {
                REQUIRE(position.piece_on_square(Square{file, rank}.value()) == Piece{});
            }
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
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
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
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
            REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == Piece{BLACK, PAWN});
        }

        REQUIRE(position.white_to_move() == false);
        REQUIRE(position.castle == Position::CASTLE_ALL);
        REQUIRE(position.halfmoves == 0);
        REQUIRE(position.moves == 1);
    }

    SECTION("After 1.e4 c5") {
        std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
        Position position = Position::from_fen(fen);

        REQUIRE(position.piece_on_square(A1) == Piece{WHITE, ROOK});
        REQUIRE(position.piece_on_square(B1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(C1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(D1) == Piece{WHITE, QUEEN});
        REQUIRE(position.piece_on_square(E1) == Piece{WHITE, KING});
        REQUIRE(position.piece_on_square(F1) == Piece{WHITE, BISHOP});
        REQUIRE(position.piece_on_square(G1) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(H1) == Piece{WHITE, ROOK});
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
            if (file != FILE_E) {
                REQUIRE(position.piece_on_square(Square{file, RANK_2}.value()) == Piece{WHITE, PAWN});
            } else {
                REQUIRE(position.piece_on_square(Square{file, RANK_4}.value()) == Piece{WHITE, PAWN});
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
        for (u8 file = FILE_A; file <= FILE_H; ++file) {
            if (file != FILE_C) {
                REQUIRE(position.piece_on_square(Square{file, RANK_7}.value()) == Piece{BLACK, PAWN});
            } else {
                REQUIRE(position.piece_on_square(Square{file, RANK_5}.value()) == Piece{BLACK, PAWN});
            }
        }

        for (u8 rank = RANK_3; rank <= RANK_6; ++rank) {
            for (u8 file = FILE_A; file <= FILE_H; ++file) {
                // skip e4 and c5
                if ((file == FILE_E && rank == RANK_4) || (file == FILE_C && rank == RANK_5)) {
                    continue;
                }
                REQUIRE(position.piece_on_square(Square{file, rank}.value()) == Piece{});
            }
        }

        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle == Position::CASTLE_ALL);
        REQUIRE(position.halfmoves == 0);
        REQUIRE(position.moves == 2);
    }

    SECTION("Kiwipete position") {
        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
        Position position = Position::from_fen(fen);
    }
}
