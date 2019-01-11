#include "catch.hpp"
#include "position.h"
#include <iostream>
#include <map>

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
            REQUIRE(position.piece_on_square(Square{file, RANK_2}) == Piece{WHITE, PAWN});
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
            REQUIRE(position.piece_on_square(Square{file, RANK_7}) == Piece{BLACK, PAWN});
        }

        for (u8 rank = RANK_3; rank <= RANK_6; ++rank) {
            for (u8 file = FILE_A; file <= FILE_H; ++file) {
                REQUIRE(position.piece_on_square(Square{file, rank}) == Piece{});
            }
        }

        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 1);

        // flip side to move a couple times to verify that...
        REQUIRE(position.white_to_move() == true);
        position.flip_to_move();
        REQUIRE(position.white_to_move() == false);
        position.flip_to_move();
        REQUIRE(position.white_to_move() == true);
        position.flip_to_move();
        REQUIRE(position.white_to_move() == false);
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
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
                REQUIRE(position.piece_on_square(Square{file, RANK_2}) == Piece{WHITE, PAWN});
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
            REQUIRE(position.piece_on_square(Square{file, RANK_7}) == Piece{BLACK, PAWN});
        }

        REQUIRE(position.white_to_move() == false);
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 1);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == Square{E3});
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
                REQUIRE(position.piece_on_square(Square{file, RANK_2}) == Piece{WHITE, PAWN});
            } else {
                REQUIRE(position.piece_on_square(Square{file, RANK_4}) == Piece{WHITE, PAWN});
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
                REQUIRE(position.piece_on_square(Square{file, RANK_7}) == Piece{BLACK, PAWN});
            } else {
                REQUIRE(position.piece_on_square(Square{file, RANK_5}) == Piece{BLACK, PAWN});
            }
        }

        for (u8 rank = RANK_3; rank <= RANK_6; ++rank) {
            for (u8 file = FILE_A; file <= FILE_H; ++file) {
                // skip e4 and c5
                if ((file == FILE_E && rank == RANK_4) || (file == FILE_C && rank == RANK_5)) {
                    continue;
                }
                REQUIRE(position.piece_on_square(Square{file, rank}) == Piece{});
            }
        }

        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 2);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == Square{C6});
    }

    SECTION("Kiwipete position") {
        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
        Position position = Position::from_fen(fen);

        REQUIRE(position.piece_on_square(A1) == Piece{WHITE, ROOK});
        REQUIRE(position.piece_on_square(E1) == Piece{WHITE, KING});
        REQUIRE(position.piece_on_square(F3) == Piece{WHITE, QUEEN});
        REQUIRE(position.piece_on_square(E5) == Piece{WHITE, KNIGHT});
        REQUIRE(position.piece_on_square(D5) == Piece{WHITE, PAWN});
        REQUIRE(position.piece_on_square(C3) == Piece{WHITE, KNIGHT});

        REQUIRE(position.piece_on_square(E8) == Piece{BLACK, KING});
        REQUIRE(position.piece_on_square(A6) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(G7) == Piece{BLACK, BISHOP});
        REQUIRE(position.piece_on_square(E7) == Piece{BLACK, QUEEN});
        REQUIRE(position.piece_on_square(B6) == Piece{BLACK, KNIGHT});
        REQUIRE(position.piece_on_square(B4) == Piece{BLACK, PAWN});

        REQUIRE(position.piece_on_square(B5) == Piece{});
        REQUIRE(position.piece_on_square(H5) == Piece{});

        REQUIRE(position.move_number() == 1); // default value when missing from FEN
        REQUIRE(position.fifty_move_rule_moves() == 0); // default value when missing from FEN
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #3") {
        std::string fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
        Position position = Position::from_fen(fen);

        std::map<u8, Piece> expected = {
            { A5, {WHITE, KING} },
            { B4, {WHITE, ROOK} },
            { B5, {WHITE, PAWN} },
            { E2, {WHITE, PAWN} },
            { G2, {WHITE, PAWN} },
            { C7, {BLACK, PAWN} },
            { D6, {BLACK, PAWN} },
            { F4, {BLACK, PAWN} },
            { H4, {BLACK, KING} },
            { H5, {BLACK, ROOK} },
        };

        for (u8 sq = 0; sq < 64; ++sq) {
            auto it  = expected.find(sq);
            Piece pc = it != expected.end() ? it->second : Piece{};
            REQUIRE(position.piece_on_square(sq) == pc);
        }

        REQUIRE(position.move_number() == 1);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.castle_flags() == Position::CASTLE_NONE);
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #4") {
        std::string fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
        Position position = Position::from_fen(fen);

        std::map<u8, Piece> expected = {
            { A1, { WHITE, ROOK } },
            { D1, { WHITE, QUEEN } },
            { F1, { WHITE, ROOK } },
            { G1, { WHITE, KING } },
            { A2, { WHITE, PAWN } },
            { B2, { BLACK, PAWN } },
            { D2, { WHITE, PAWN } },
            { G2, { WHITE, PAWN } },
            { H2, { WHITE, PAWN } },
            { A3, { BLACK, QUEEN } },
            { F3, { WHITE, KNIGHT } },
            { A4, { WHITE, BISHOP } },
            { B4, { WHITE, BISHOP } },
            { C4, { WHITE, PAWN } },
            { E4, { WHITE, PAWN } },
            { A5, { BLACK, KNIGHT } },
            { B5, { WHITE, PAWN } },
            { B6, { BLACK, BISHOP } },
            { F6, { BLACK, KNIGHT } },
            { G6, { BLACK, BISHOP } },
            { H6, { WHITE, KNIGHT } },
            { A7, { WHITE, PAWN } },
            { B7, { BLACK, PAWN } },
            { C7, { BLACK, PAWN } },
            { D7, { BLACK, PAWN } },
            { F7, { BLACK, PAWN } },
            { G7, { BLACK, PAWN } },
            { H7, { BLACK, PAWN } },
            { A8, { BLACK, ROOK } },
            { E8, { BLACK, KING } },
            { H8, { BLACK, ROOK } },
        };

        for (u8 sq = 0; sq < 64; ++sq) {
            auto it  = expected.find(sq);
            Piece pc = it != expected.end() ? it->second : Piece{};
            REQUIRE(position.piece_on_square(sq) == pc);
        }

        REQUIRE(position.move_number() == 1);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.castle_flags() == (Position::CASTLE_BLACK_KING_SIDE | Position::CASTLE_BLACK_QUEEN_SIDE));
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #5") {
        std::string fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
        Position position = Position::from_fen(fen);

        REQUIRE(position.move_number() == 8);
        REQUIRE(position.fifty_move_rule_moves() == 1);
        REQUIRE(position.castle_flags() == (Position::CASTLE_WHITE_KING_SIDE | Position::CASTLE_WHITE_QUEEN_SIDE));
        REQUIRE(position.enpassant_available() == false);
    }

}

TEST_CASE("Position::dump_fen") {
    using FEN = std::string;
    std::vector<FEN> test_cases = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    };

    for (auto&& fen: test_cases) {
        Position position = Position::from_fen(fen);
        std::string my_fen = position.dump_fen();
        REQUIRE(my_fen == fen);
    }
}

TEST_CASE("Position::make_move") {
    SECTION("1.e4") {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(fen);
        Move move(E2, E4); // 1. e4
        REQUIRE(move.from() == E2);
        REQUIRE(move.to()   == E4);
        REQUIRE(move.is_promotion() == false);
        REQUIRE(move.is_enpassant() == false);
        REQUIRE(move.is_castle() == false);
        REQUIRE(position.move_number() == 1);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        Savepos save;
        position.make_move(save, move);
        REQUIRE(position.move_number() == 1);
        REQUIRE(position.fifty_move_rule_moves() == 0);

        REQUIRE(position.piece_on_square(E2) == NO_PIECE);
        REQUIRE(position.piece_on_square(E4) == Piece(WHITE, PAWN));

        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == E3);

        std::string expect_fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        Position expected = Position::from_fen(expect_fen);
        REQUIRE(position == expected);
    }

    SECTION("1.e4 c5") {
        std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        Position position = Position::from_fen(fen);
        Move move(C7, C5);
        Savepos save;
        position.make_move(save, move);
        std::string expected_fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
        REQUIRE(position.dump_fen() == expected_fen);
    }

    SECTION("Silician") {
        using FEN = std::string;
        using TestCase = std::pair<Move, FEN>;
        std::vector<TestCase> test_cases = {
			{ Move(E2, E4), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" },
			{ Move(C7, C5), "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2" },
			{ Move(G1, F3), "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2" },
			{ Move(B8, C6), "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3" },
			{ Move(D2, D4), "r1bqkbnr/pp1ppppp/2n5/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3" },
            { Move(C5, D4), "r1bqkbnr/pp1ppppp/2n5/8/3pP3/5N2/PPP2PPP/RNBQKB1R w KQkq - 0 4" },
            { Move(F3, D4), "r1bqkbnr/pp1ppppp/2n5/8/3NP3/8/PPP2PPP/RNBQKB1R b KQkq - 0 4"},
            { Move(E7, E6), "r1bqkbnr/pp1p1ppp/2n1p3/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5" },
            { Move(F1, E2), "r1bqkbnr/pp1p1ppp/2n1p3/8/3NP3/8/PPP1BPPP/RNBQK2R b KQkq - 1 5" },
            { Move(G8, F6), "r1bqkb1r/pp1p1ppp/2n1pn2/8/3NP3/8/PPP1BPPP/RNBQK2R w KQkq - 2 6" },
            { Move::make_castle_move(Move::CastleKind::WHITE_KING_SIDE), "r1bqkb1r/pp1p1ppp/2n1pn2/8/3NP3/8/PPP1BPPP/RNBQ1RK1 b kq - 3 6", },
        };

        FEN starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;
        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
        }
    }
}
