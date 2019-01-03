#include "catch.hpp"
#include "position.h"

TEST_CASE("Position from FEN") {
    SECTION("Parse FEN board") {
        Position position;
        std::string fen_ = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        std::string_view fen{fen_};
        auto it = parse_fen_board(fen.begin(), fen.end(), position);

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

        REQUIRE(it != fen.end());
        REQUIRE(*it == ' ');
    }
    // SECTION("Starting position") {
    //     auto maybe_pos = Position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //     REQUIRE(static_cast<bool>(maybe_pos) == true);
    //     const Position& pos = *maybe_pos;
    //     REQUIRE(pos.castle() == Position::CASTLE_ALL);
    //     REQUIRE(pos.epsq() > H6);
    //     REQUIRE(pos.piece_on_square(A1) == ColorPiece{WHITE, ROOK});
    // }
}
