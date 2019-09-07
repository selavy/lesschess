#include "catch.hpp"
#include "position.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>

using namespace lesschess;

TEST_CASE("Read long algebraic moves", "[position]")
{
    Zobrist::initialize();

    SECTION("White moves from starting position") {
        std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "e2e4", Move(E2, E4) },
            { "d2d4", Move(D2, D4) },
            { "c2c4", Move(C2, C4) },
            { "a2a3", Move(A2, A3) },
            { "a2a4", Move(A2, A4) },
            { "g1f3", Move(G1, F3) },
            { "g1h3", Move(G1, H3) },
            { "b1c3", Move(B1, C3) },
            { "b1a3", Move(B1, A3) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(startpos);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == false);
            REQUIRE(move.is_promotion() == false);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }

    SECTION("Black moves from starting position after 1.e4") {
        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | |P| | | |
        // | | | | | | | | |
        // |P|P|P|P| |P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // b KQkq e3 0 1
        std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "e7e5", Move(E7, E5) },
            { "d7d5", Move(D7, D5) },
            { "c7c5", Move(C7, C5) },
            { "a7a6", Move(A7, A6) },
            { "a7a5", Move(A7, A5) },
            { "g8f6", Move(G8, F6) },
            { "g8h6", Move(G8, H6) },
            { "b8c6", Move(B8, C6) },
            { "b8a6", Move(B8, A6) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == false);
            REQUIRE(move.is_promotion() == false);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }

    SECTION("White castle moves") {
        // |r| | | |k| | |r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // |P|P|P|P|P|P|P|P|
        // |R| | | |K| | |R|
        // w KQkq - 0 1
        std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "e1g1", Move::make_castle(Castle::WHITE_KING_SIDE) },
            { "e1c1", Move::make_castle(Castle::WHITE_QUEEN_SIDE) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == true);
            REQUIRE(move.is_promotion() == false);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }

    SECTION("Black castle moves") {
        // |r| | | |k| | |r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // |P|P|P|P|P|P|P|P|
        // |R| | | |K| | |R|
        // b KQkq - 0 1
        std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "e8g8", Move::make_castle(Castle::BLACK_KING_SIDE) },
            { "e8c8", Move::make_castle(Castle::BLACK_QUEEN_SIDE) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == true);
            REQUIRE(move.is_promotion() == false);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }

    SECTION("En Passant") {
        std::vector<std::tuple<std::string, Move, std::string>> tests = {
            // white
            { "e5d6", Move::make_enpassant(E5, D6), "4k3/ppp1pppp/8/3pP3/8/8/PPPP1PPP/4K3 w - d6 0 1" },
            { "b5a6", Move::make_enpassant(B5, A6), "4k3/1ppppppp/8/pP6/8/8/PPPP1PPP/4K3 w - a6 0 1" },
            { "h5g6", Move::make_enpassant(H5, G6), "4k3/pppppp1p/8/6pP/8/8/PPPP1PPP/4K3 w - g6 0 1" },
            // black
            { "d4e3", Move::make_enpassant(D4, E3), "4k3/ppp1pppp/8/8/3pP3/8/PPPP1PPP/4K3 b - e3 0 1" },
            { "g4h3", Move::make_enpassant(G4, H3), "4k3/ppp1pppp/8/8/6pP/8/PPPP1PPP/4K3 b - h3 0 1" },
            { "b4a3", Move::make_enpassant(B4, A3), "4k3/ppp1pppp/8/8/Pp6/8/1PPPPPPP/4K3 b - a3 0 1" },
        };

        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto& fen      = std::get<2>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == false);
            REQUIRE(move.is_promotion() == false);
            REQUIRE(move.is_enpassant() == true);
            REQUIRE(move == expected);
        }
    }

	SECTION("White Promotions") {
        std::string fen = "4k3/P7/8/8/8/8/7p/4K3 w - - 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "a7a8q", Move::make_promotion(A7, A8, QUEEN) },
            { "a7a8r", Move::make_promotion(A7, A8, ROOK) },
            { "a7a8b", Move::make_promotion(A7, A8, BISHOP) },
            { "a7a8n", Move::make_promotion(A7, A8, KNIGHT) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == false);
            REQUIRE(move.is_promotion() == true);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }

	SECTION("Black Promotions") {
        std::string fen = "4k3/P7/8/8/8/8/7p/4K3 b - - 0 1";
        std::vector<std::pair<std::string, Move>> tests = {
            { "h2h1q", Move::make_promotion(H2, H1, QUEEN) },
            { "h2h1r", Move::make_promotion(H2, H1, ROOK) },
            { "h2h1b", Move::make_promotion(H2, H1, BISHOP) },
            { "h2h1n", Move::make_promotion(H2, H1, KNIGHT) },
        };
        for (auto& t : tests) {
            auto& movespec = std::get<0>(t);
            auto& expected = std::get<1>(t);
            auto position = Position::from_fen(fen);
            auto move = position.move_from_long_algebraic(movespec);
            REQUIRE(move.to_long_algebraic_string() == movespec);
            REQUIRE(move.is_castle()    == false);
            REQUIRE(move.is_promotion() == true);
            REQUIRE(move.is_enpassant() == false);
            REQUIRE(move == expected);
        }
    }
}

TEST_CASE("Position from FEN", "[position]")
{
    Zobrist::initialize();

    SECTION("Starting position") {
        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // |P|P|P|P|P|P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // w KQkq - 0 1
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
        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 1);

        // flip side to move a couple times to verify that...
        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("After 1.e4") {
        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | |P| | | |
        // | | | | | | | | |
        // |P|P|P|P| |P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // b KQkq e3 0 1
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
        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 1);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == Square{E3});
    }

    SECTION("After 1.e4 c5") {
        // |r|n|b|q|k|b|n|r|
        // |p|p| |p|p|p|p|p|
        // | | | | | | | | |
        // | | |p| | | | | |
        // | | | | |P| | | |
        // | | | | | | | | |
        // |P|P|P|P| |P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // w KQkq c6 0 2
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
        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 2);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == Square{C6});
    }

    SECTION("Kiwipete position") {
        // |r| | | |k| | |r|
        // |p| |p|p|q|p|b| |
        // |b|n| | |p|n|p| |
        // | | | |P|N| | | |
        // | |p| | |P| | | |
        // | | |N| | |Q| |p|
        // |P|P|P|B|B|P|P|P|
        // |R| | | |K| | |R|
        // w KQkq -
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
        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #3") {
        // | | | | | | | | |
        // | | |p| | | | | |
        // | | | |p| | | | |
        // |K|P| | | | | |r|
        // | |R| | | |p| |k|
        // | | | | | | | | |
        // | | | | |P| |P| |
        // | | | | | | | | |
        // w - -
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
        REQUIRE(position.castle_flags() == CASTLE_NONE);
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #4") {
        // |r|n|b|q| |k| |r|
        // |p|p| |P|b|p|p|p|
        // | | |p| | | | | |
        // | | | | | | | | |
        // | | |B| | | | | |
        // | | | | | | | | |
        // |P|P|P| |N|n|P|P|
        // |R|N|B|Q|K| | |R|
        // w KQ - 1 8
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
        REQUIRE(!position.castle_allowed(Castle::WHITE_KING_SIDE));
        REQUIRE(!position.castle_allowed(Castle::WHITE_QUEEN_SIDE));
        REQUIRE(position.castle_allowed(Castle::BLACK_KING_SIDE));
        REQUIRE(position.castle_allowed(Castle::BLACK_QUEEN_SIDE));
        REQUIRE(position.enpassant_available() == false);
    }

    SECTION("Position #5") {
        // |r|n|b|q| |k| |r|
        // |p|p| |P|b|p|p|p|
        // | | |p| | | | | |
        // | | | | | | | | |
        // | | |B| | | | | |
        // | | | | | | | | |
        // |P|P|P| |N|n|P|P|
        // |R|N|B|Q|K| | |R|
        // w KQ - 1 8
        std::string fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
        Position position = Position::from_fen(fen);

        REQUIRE(position.move_number() == 8);
        REQUIRE(position.fifty_move_rule_moves() == 1);
        REQUIRE(position.castle_allowed(Castle::WHITE_KING_SIDE));
        REQUIRE(position.castle_allowed(Castle::WHITE_QUEEN_SIDE));
        REQUIRE(!position.castle_allowed(Castle::BLACK_KING_SIDE));
        REQUIRE(!position.castle_allowed(Castle::BLACK_QUEEN_SIDE));
        REQUIRE(position.enpassant_available() == false);
    }

}

TEST_CASE("Position::dump_fen", "[position]")
{
    Zobrist::initialize();

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

TEST_CASE("Position::make_move", "[position]")
{
    Zobrist::initialize();

    SECTION("1.e4") {
        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | | | | | |
        // |P|P|P|P|P|P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // w KQkq - 0 1
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

        REQUIRE(position.castle_flags() == CASTLE_ALL);
        REQUIRE(position.enpassant_available() == true);
        REQUIRE(position.enpassant_target_square() == E3);

        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | |P| | | |
        // | | | | | | | | |
        // |P|P|P|P| |P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // b KQkq e3 0 1
        std::string expect_fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        Position expected = Position::from_fen(expect_fen);
        REQUIRE(position == expected);

        position.undo_move(save, move);
        REQUIRE(position == Position::from_fen(fen));
    }

    SECTION("1.e4 c5") {
        // |r|n|b|q|k|b|n|r|
        // |p|p|p|p|p|p|p|p|
        // | | | | | | | | |
        // | | | | | | | | |
        // | | | | |P| | | |
        // | | | | | | | | |
        // |P|P|P|P| |P|P|P|
        // |R|N|B|Q|K|B|N|R|
        // b KQkq e3 0 1
        std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        Position position = Position::from_fen(fen);
        Move move(C7, C5);
        Savepos save;
        position.make_move(save, move);
        std::string expected_fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
        REQUIRE(position.dump_fen() == expected_fen);
        position.undo_move(save, move);
        REQUIRE(position == Position::from_fen(fen));
    }

    using FEN = std::string;
    using TestCase = std::pair<Move, FEN>;
    SECTION("Silician to start + en passant") {
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
            { Move::make_castle(Castle::WHITE_KING_SIDE), "r1bqkb1r/pp1p1ppp/2n1pn2/8/3NP3/8/PPP1BPPP/RNBQ1RK1 b kq - 3 6", },
            { Move(B7, B6), "r1bqkb1r/p2p1ppp/1pn1pn2/8/3NP3/8/PPP1BPPP/RNBQ1RK1 w kq - 0 7" },
            { Move(F1, E1), "r1bqkb1r/p2p1ppp/1pn1pn2/8/3NP3/8/PPP1BPPP/RNBQR1K1 b kq - 1 7" },
            { Move(C8, A6), "r2qkb1r/p2p1ppp/bpn1pn2/8/3NP3/8/PPP1BPPP/RNBQR1K1 w kq - 2 8" },
            { Move(B1, C3), "r2qkb1r/p2p1ppp/bpn1pn2/8/3NP3/2N5/PPP1BPPP/R1BQR1K1 b kq - 3 8" },
            { Move(D8, C7), "r3kb1r/p1qp1ppp/bpn1pn2/8/3NP3/2N5/PPP1BPPP/R1BQR1K1 w kq - 4 9" },
            { Move(C1, G5), "r3kb1r/p1qp1ppp/bpn1pn2/6B1/3NP3/2N5/PPP1BPPP/R2QR1K1 b kq - 5 9" },
            { Move::make_castle(Castle::BLACK_QUEEN_SIDE), "2kr1b1r/p1qp1ppp/bpn1pn2/6B1/3NP3/2N5/PPP1BPPP/R2QR1K1 w - - 6 10" },
            { Move(E4, E5), "2kr1b1r/p1qp1ppp/bpn1pn2/4P1B1/3N4/2N5/PPP1BPPP/R2QR1K1 b - - 0 10" },
            { Move(D7, D5), "2kr1b1r/p1q2ppp/bpn1pn2/3pP1B1/3N4/2N5/PPP1BPPP/R2QR1K1 w - d6 0 11" },
            { Move::make_enpassant(E5, D6), "2kr1b1r/p1q2ppp/bpnPpn2/6B1/3N4/2N5/PPP1BPPP/R2QR1K1 b - - 0 11" },
        };

		// |r|n|b|q|k|b|n|r|
		// |p|p|p|p|p|p|p|p|
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// |P|P|P|P|P|P|P|P|
		// |R|N|B|Q|K|B|N|R|
		// w KQkq - 0 1
        FEN starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;
        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            auto orig_position = position;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
            position.undo_move(save, move);
            REQUIRE(position == orig_position);
            position.make_move(save, move);
        }
    }

    SECTION("White promotion") {
		// | | | | |k| | | |
		// | | | | | | | |P|
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | |K| | | |
		// w KQkq - 0 1
        FEN starting_position = "4k3/7P/8/8/8/8/8/4K3 w - - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;

        std::vector<TestCase> test_cases = {
            { Move::make_promotion(H7, H8, ROOK), "4k2R/8/8/8/8/8/8/4K3 b - - 0 1" }, // h8=R
            { Move(E8, D7),                       "7R/3k4/8/8/8/8/8/4K3 w - - 1 2" }, // Kd7
        };

        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            auto orig_position = position;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
            position.undo_move(save, move);
            REQUIRE(position == orig_position);
            position.make_move(save, move);
        }
    }

    SECTION("Black promotion") {
		// | | | | |k| | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | |p|
		// | | | | |K| | | |
		// b - - 0 1
        FEN starting_position = "4k3/8/8/8/8/8/7p/4K3 b - - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;

        std::vector<TestCase> test_cases = {
            { Move::make_promotion(H2, H1, KNIGHT), "4k3/8/8/8/8/8/8/4K2n w - - 0 2" }, // h1=N
            { Move(E1, D2),                         "4k3/8/8/8/8/8/3K4/7n b - - 1 2"   }, // Kd2
        };

        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            auto orig_position = position;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
            position.undo_move(save, move);
            REQUIRE(position == orig_position);
            position.make_move(save, move);
        }
    }

    SECTION("White promotion with capture") {
		// | | | | |k| |n| |
		// | | | | | | | |P|
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | |K| | | |
		// w - - 0 1
        FEN starting_position = "4k1n1/7P/8/8/8/8/8/4K3 w - - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;
        auto move = Move::make_promotion(H7, G8, QUEEN);
        auto orig_position = position;
        position.make_move(save, move);
        REQUIRE(position.dump_fen() == "4k1Q1/8/8/8/8/8/8/4K3 b - - 0 1");
        position.undo_move(save, move);
        REQUIRE(position == orig_position);
    }

    SECTION("Make move white castle") {
		// |r| | | |k| | |r|
		// |p|p|p|p|p|p|p|p|
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// |P|P|P|P|P|P|P|P|
		// |R| | | |K| | |R|
		// w KQkq - 0 1
        std::string original_fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";

        using FEN = std::string;
        std::vector<std::pair<Castle, FEN>> test_cases = {
            { Castle::WHITE_KING_SIDE, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R4RK1 b kq - 1 1" },
            { Castle::WHITE_QUEEN_SIDE, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/2KR3R b kq - 1 1"},
        };

        for (auto&& test_case: test_cases) {
            auto&& kind = std::get<0>(test_case);
            auto&& expected = std::get<1>(test_case);
            Position position = Position::from_fen(original_fen);
            Move move = Move::make_castle(kind);
            Savepos save;
            auto orig_position = position;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == expected);
            position.undo_move(save, move);
            REQUIRE(position == orig_position);
        }
    }

    SECTION("Make move black castle") {
		// |r| | | |k| | |r|
		// |p|p|p|p|p|p|p|p|
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// | | | | | | | | |
		// |P|P|P|P|P|P|P|P|
		// |R| | | |K| | |R|
		// w KQkq - 0 1
        std::string original_fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1";

        using FEN = std::string;
        std::vector<std::pair<Castle, FEN>> test_cases = {
            { Castle::BLACK_KING_SIDE, "r4rk1/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 1 2" },
            { Castle::BLACK_QUEEN_SIDE, "2kr3r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 1 2" },
        };

        for (auto&& test_case: test_cases) {
            auto&& kind = std::get<0>(test_case);
            auto&& expected = std::get<1>(test_case);
            Position position = Position::from_fen(original_fen);
            auto orig_position = position;
            Move move = Move::make_castle(kind);
            Savepos save;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == expected);
            position.undo_move(save, move);
            REQUIRE(position == orig_position);
        }
    }
}

TEST_CASE("Undo Move", "[position]")
{
    Zobrist::initialize();

    SECTION("Undo Flags=NONE with no capture") {
        std::string starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position position = Position::from_fen(starting_position);
        Position position_copy = position;
        REQUIRE(position.dump_fen() == starting_position);
        Move move(E2, E4);
        Savepos save;
        position.make_move(save, move);
        std::string position_1_e4 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        REQUIRE(position.dump_fen() == position_1_e4);
        position.undo_move(save, move);
        REQUIRE(position.dump_fen() == starting_position);
        REQUIRE(position == position_copy);
    }

    SECTION("Undo Flags=NONE with no capture king move") {
        std::string original_fen = "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;
        REQUIRE(position.dump_fen() == original_fen);
        Move move(E1, E2);
        Savepos save;

        // should be able to make/undo move multiple times just fine
        for (int i = 0; i < 5; ++i) {
            position.make_move(save, move);
            std::string expected = "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPPKPPP/RNBQ1BNR b kq - 1 2";
            REQUIRE(position.dump_fen() == expected);
            position.undo_move(save, move);
            REQUIRE(position.dump_fen() == original_fen);
            REQUIRE(position == position_copy);
        }
    }

    SECTION("Undo Flags=NONE capture") {
        std::string original_fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPPKPPP/RNBQ1BNR w kq -";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;
        Move move(E4, D5); // exd5
        Savepos save;
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

    SECTION("Undo Flags=NONE capture with black") {
        std::string original_fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPPKPPP/RNBQ1BNR b kq - 0 1";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;
        Move move(D5, E4); // dxe4
        Savepos save;
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

    SECTION("Undo white castle move") {
        std::string original_fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;

        std::array<Castle, 2> kinds = {
            Castle::WHITE_KING_SIDE,
            Castle::WHITE_QUEEN_SIDE,
        };

        for (auto&& kind: kinds) {
            position = position_copy;
            Move move = Move::make_castle(kind);
            Savepos save;
            position.make_move(save, move);
            position.undo_move(save, move);
            REQUIRE(position == position_copy);
        }
    }

    SECTION("Undo black castle move") {
        std::string original_fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;

        std::array<Castle, 2> kinds = {
            Castle::BLACK_KING_SIDE,
            Castle::BLACK_QUEEN_SIDE,
        };

        for (auto&& kind: kinds) {
            position = position_copy;
            Move move = Move::make_castle(kind);
            Savepos save;
            position.make_move(save, move);
            position.undo_move(save, move);
            REQUIRE(position == position_copy);
        }
    }

    SECTION("Undo white promotion") {
        std::string original_fen = "4k3/7P/8/8/8/8/p7/4K3 w - -";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;
        Savepos save;

        std::array<PieceKind, 4> kinds = {
            QUEEN, ROOK, BISHOP, KNIGHT,
        };

        for (auto kind: kinds) {
            position = position_copy;
            auto move = Move::make_promotion(H7, H8, kind);
            position.make_move(save, move);
            position.undo_move(save, move);
            REQUIRE(position == position_copy);
        }
    }

    SECTION("Undo black promotion") {
        std::string original_fen = "4k3/7P/8/8/8/8/p7/4K3 b - -";
        Position position = Position::from_fen(original_fen);
        Position position_copy = position;
        Savepos save;

        std::array<PieceKind, 4> kinds = {
            QUEEN, ROOK, BISHOP, KNIGHT,
        };

        for (auto kind: kinds) {
            position = position_copy;
            auto move = Move::make_promotion(A2, A1, kind);
            position.make_move(save, move);
            position.undo_move(save, move);
            REQUIRE(position == position_copy);
        }
    }

    SECTION("Undo white en passant") {
        std::string fen = "4k3/8/8/6pP/pP6/8/8/4K3 w - g6";
        Position position = Position::from_fen(fen);
        Position position_copy = position;
        Savepos save;
        auto move = Move::make_enpassant(H5, G6);
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

    SECTION("Undo black en passant") {
        std::string fen = "4k3/8/8/6pP/pP6/8/8/4K3 b - b3";
        Position position = Position::from_fen(fen);
        Position position_copy = position;
        Savepos save;
        auto move = Move::make_enpassant(A4, B3);
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

}


TEST_CASE("FEN to ASCII and back", "[position]")
{
    Zobrist::initialize();

    std::vector<std::pair<std::string, std::string>> fens = {
        {
            "|r| |b|k| | | |r|\n" \
            "|p| | |p|B|p|N|p|\n" \
            "|n| | | | |n| | |\n" \
            "| |p| |N|P| | |P|\n" \
            "| | | | | | |P| |\n" \
            "| | | |P| | | | |\n" \
            "|P| |P| |K| | | |\n" \
            "|q| | | | | |b| |\n" \
            "b - - 0 1",

            "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 b - - 0 1"
        },

        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p|p|p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P|P|P|P|P|\n" \
            "|R|N|B|Q|K|B|N|R|\n" \
            "w KQkq - 0 1",

            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        },

        {
            "| | | | |k| |n| |\n" \
            "| | | | | | | |P|\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |K| | | |\n" \
            "w - - 0 1",

            "4k1n1/7P/8/8/8/8/8/4K3 w - - 0 1"
        },

        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | |p|\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 1",

            "4k3/8/8/8/8/8/7p/4K3 b - - 0 1"
        }

    };

    for (auto& p : fens) {
        auto& ascii = std::get<0>(p);
        auto& fen   = std::get<1>(p);
        REQUIRE(Position::from_ascii(ascii).dump_fen() == fen);
        REQUIRE(Position::from_fen(fen).dump_ascii()   == ascii);
        REQUIRE(Position::from_ascii(ascii) == Position::from_fen(fen));
    }
}

TEST_CASE("Square is Attacked?", "[position]")
{
    Zobrist::initialize();

    SECTION("Immortal Game Position")
    {
        std::string position =
            "|r| |b|k| | | |r|\n" \
            "|p| | |p|B|p|N|p|\n" \
            "|n| | | | |n| | |\n" \
            "| |p| |N|P| | |P|\n" \
            "| | | | | | |P| |\n" \
            "| | | |P| | | | |\n" \
            "|P| |P| |K| | | |\n" \
            "|q| | | | | |b| |\n" \
            "b - - 0 1";
        auto p = Position::from_ascii(position);

        REQUIRE(p.attacks(WHITE, F3) == true);
        REQUIRE(p.attacks(WHITE, H1) == false);
        REQUIRE(p.attacks(WHITE, E1) == true);
        REQUIRE(p.attacks(BLACK, B1) == true);
        REQUIRE(p.attacks(BLACK, H1) == false);
        REQUIRE(p.attacks(BLACK, C1) == true);
        REQUIRE(p.attacks(WHITE, B4) == true);
        REQUIRE(p.attacks(BLACK, B4) == true);
        REQUIRE(p.attacks(WHITE, A8) == false);
    }
}

TEST_CASE("Legal Move Check", "[position]")
{
    Zobrist::initialize();

    using TestMove = std::pair<Move, bool>;
    std::vector<std::pair<std::string, std::vector<TestMove>>> tests = {
        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p|p|p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P|P|P|P|P|\n" \
            "|R|N|B|Q|K|B|N|R|\n" \
            "w KQkq - 0 1",
            {
                { Move(E2, E4), true  },
                { Move(E8, F7), false },
                { Move(E8, F7), false },
                { Move(E4, E5), false },
                { Move::make_castle(Castle::WHITE_KING_SIDE),  false },
                { Move::make_castle(Castle::WHITE_QUEEN_SIDE), false },
                { Move::make_castle(Castle::BLACK_KING_SIDE),  false },
                { Move::make_castle(Castle::BLACK_QUEEN_SIDE), false },
            }
        },

        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p| |p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| | | |p|P| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P| |P|P|P|\n" \
            "|R|N|B|Q|K|B|N|R|\n" \
            "w KQkq d6 0 2",
            {
                { Move::make_enpassant(E5, D6), true  },
                { Move::make_enpassant(E5, F6), false },
            }
        }
    };

    for (auto& p : tests) {
        auto& desc = std::get<0>(p);
        auto& tcs = std::get<1>(p);
        auto  position = Position::from_ascii(desc);
        for (auto& tc : tcs) {
            auto& move = std::get<0>(tc);
            auto  exp  = std::get<1>(tc);
            REQUIRE(position.is_legal_move(move) == exp);
        }
    }
}

u64 BB(std::initializer_list<int> sqs)
{
    u64 result = 0;
    for (auto sq : sqs) {
        assert(sq >= 0 && sq < 64);
        result |= (1ull << sq);
    }
    return result;
}

std::set<std::string> BB2SQs(u64 bb)
{
    std::set<std::string> result;
    for (int i = 0; i < 64; ++i) {
        if ((bb & (1ull << i)) != 0) {
            result.insert(Square::names[i]);
        }
    }
    return result;
}

TEST_CASE("_generate_checkers", "[position]")
{
    Zobrist::initialize();

    using Squares = std::initializer_list<int>;
    std::vector<std::tuple<std::string, Color, Squares>> ts = {
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |Q| | | |\n" \
            "| | | | |K| | | |\n" \
            "w - - 0 2",
            BLACK,
            { E2 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |Q| | | |\n" \
            "| | | | |K| | | |\n" \
            "w - - 0 2",
            WHITE,
            {}
        },
        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p| |p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| | | |p|P| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P| |P|P|P|\n" \
            "|R|N|B|Q|K|B|N|R|\n" \
            "w KQkq d6 0 2",
            BLACK,
            {}
        },
        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p| |p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| |B| |p|P| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P| |P|P|P|\n" \
            "|R|N|B|Q|K| |N|R|\n" \
            "w KQkq - 0 2",
            BLACK,
            { B5 }
        },
        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p| |p|p|p|p|\n" \
            "| | | | | |N| | |\n" \
            "| |B| |p|P| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P|P|P|P| |P|P|P|\n" \
            "|R|N|B|Q|K| |N|R|\n" \
            "w KQkq - 0 2",
            BLACK,
            { B5, F6 }
        },
        {
            "|r|n|b|q|k|b|n|r|\n" \
            "|p|p|p| |p|p|p|p|\n" \
            "| | | | | | | | |\n" \
            "| | | |p|P| | | |\n" \
            "| |b| | | | | | |\n" \
            "| | | |n| | | | |\n" \
            "|P|P|P| | |P|P|P|\n" \
            "|R|N|B|Q|K| |N|R|\n" \
            "w KQkq - 0 2",
            WHITE,
            { B4, D3 }
        },
    };

    for (auto& t : ts) {
        auto position = Position::from_ascii(std::get<0>(t));
        auto side = std::get<1>(t);
        auto expect = BB2SQs(BB(std::get<2>(t)));
        auto result = BB2SQs(position._generate_checkers(side));
        REQUIRE(result == expect);
    }
}

// TODO: move to private and remove tests
TEST_CASE("_generate_attacked", "[position]")
{
    Zobrist::initialize();

    using Squares = std::initializer_list<int>;
    std::vector<std::tuple<std::string, Color, Squares>> tcs = {
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |K| | | |\n" \
            "w - - 0 2",
            WHITE,
            { D1, D2, E2, F2, F1 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            BLACK,
            { D8, D7, E7, F7, F8 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |P| | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, D3, E2, F1, F2, F3 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | |P|\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, G3 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "|P| | | | | | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, B3 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |B| | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, D3, C4, B5, A6, F3, G4, H5 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |N| | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, G1, G3, F4, D4, C3, C1 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |R| | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, A2, B2, C2, D2, F2, G2, H2, E1, E3, E4, E5, E6, E7, E8 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | |Q| | | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, E2, F1, F2, A2, B2, C2, D2, F2, G2, H2, E1, E3, E4, E5, E6, E7, E8,
              D1, F3, G4, H5, F1, D3, C4, B5, A6 }
        },
        {
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | |p|p|p| | |\n" \
            "| | | |P|Q|P| | |\n" \
            "| | | | |K| | | |\n" \
            "b - - 0 2",
            WHITE,
            { D1, D2, D3, E1, E2, E3, F1, F2, F3, C3, G3 }
        }
    };

    for (auto& tc : tcs) {
        auto position = Position::from_ascii(std::get<0>(tc));
        auto attacker = std::get<1>(tc);
        auto expect = BB2SQs(BB(std::get<2>(tc)));
        auto result = BB2SQs(position._generate_attacked(attacker));
        REQUIRE(result == expect);
    }
}

TEST_CASE("generate_legal_moves", "[position]")
{
    Zobrist::initialize();

    using MoveList = std::vector<Move>;

    auto gen_moves = [](const Position& position) -> MoveList
    {
        std::vector<Move> moves(256);
        int nmoves = position.generate_legal_moves(moves.data());
        moves.erase(moves.begin() + nmoves, moves.end());
        std::sort(moves.begin(), moves.end());
        return moves;
    };

    SECTION("evasions")
    {
        std::vector<std::pair<std::string, MoveList>> ts = {
            //
            // more than 1 checker
            //
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | | |\n" \
                "| | |B| | |N| | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | |P|P|P| | |\n" \
                "| | | | |K| | | |\n" \
                "b - - 0 2",
                {
                    Move(E8, D8),
                    Move(E8, E7),
                    Move(E8, F8),
                    Move(E8, F7),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | |p|p|p| | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| |b| |n| |n| | |\n" \
                "| | | |P|P|P| | |\n" \
                "| | | | |K| | | |\n" \
                "w - - 0 2",
                {
                    Move(E1, F1),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | |p|p|p| | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | |n| |n| |b|\n" \
                "| | | |P|P| | | |\n" \
                "| | | | |K| | | |\n" \
                "w - - 0 2",
                {
                    Move(E1, D1),
                }
            },

            //
            // evade with en passant capture
            //
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | |P|p| | | |\n" \
                "| | | | | |K| | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "w - e6 0 2",
                {
                    Move::make_enpassant(D5, E6),

                    // King Moves
                    Move(F4, E4),
                    Move(F4, E3),
                    Move(F4, F3),
                    Move(F4, G3),
                    Move(F4, G4),
                    Move(F4, F5),
                    Move(F4, G5),

                    // King captures pawn
                    Move(F4, E5),
                }
            },
            {
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | |R| |\n" \
                "| | | |k| | | | |\n" \
                "| | | |p|P| |N| |\n" \
                "| | | | | |P| | |\n" \
                "| | | | | | | | |\n" \
                "| | |R| |K| | | |\n" \
                "b - e3 0 2",
                {
                    Move::make_enpassant(D4, E3),
                }
            },
            {
                "| | | | | | | | |\n" \
                "| |N| | | | | | |\n" \
                "| | | | | | |R| |\n" \
                "| |k| | | | | | |\n" \
                "|P|p| |p|P| |N| |\n" \
                "| | |P| | |P| | |\n" \
                "| | | | | | | | |\n" \
                "|Q| |R| |K| | | |\n" \
                "b - a3 0 2",
                {
                    Move::make_enpassant(B4, A3),
                    Move(B5, C4),
                }
            },
            {
                "| | | | | | | |R|\n" \
                "| | |N| | | | | |\n" \
                "|R| | | | | | | |\n" \
                "| | | | | | |k| |\n" \
                "| | | |p|P| |p|P|\n" \
                "| | |P| | |P|P| |\n" \
                "| | | | | | | | |\n" \
                "| | |R| |K| | | |\n" \
                "b - h3 0 2",
                {
                    Move::make_enpassant(G4, H3),
                }
            },

            //
            // evade by blocking
            //
            {
                "|r|n|b|q|k|b|n|r|\n" \
                "|p|p|p|p|p|p|p|p|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| |b| | | | | | |\n" \
                "| | | | | | | | |\n" \
                "|P|P|P| |P|P|P|P|\n" \
                "|R|N|B|Q|K| | |R|\n" \
                "w KQkq - 0 2",
                {
                    Move(C2, C3),
                    Move(C1, D2),
                    Move(D1, D2),
                    Move(B1, D2),
                    Move(B1, C3),
                    Move(E1, F1),
                }
            },
            {
                "|r|n|b|q|k| | |r|\n" \
                "|p|p|p| |p|p|p|p|\n" \
                "| | | | | | | | |\n" \
                "| |B| | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "|P|P|P|P|P|P|P|P|\n" \
                "|R|N|B|Q|K| |N|R|\n" \
                "b KQkq - 0 2",
                {
                    Move(C7, C6),
                    Move(C8, D7),
                    Move(D8, D7),
                    Move(E8, F8),
                    Move(B8, C6),
                    Move(B8, D7),
                }
            },
            {
                "|r|n|b|q|k| | |r|\n" \
                "|p|p|p| |p|p|p|p|\n" \
                "| | | | | | | | |\n" \
                "| |Q| | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "|P|P|P|P|P|P|P|P|\n" \
                "|R|N|B|Q|K| |N|R|\n" \
                "b KQkq - 0 2",
                {
                    Move(C7, C6),
                    Move(C8, D7),
                    Move(D8, D7),
                    Move(E8, F8),
                    Move(B8, C6),
                    Move(B8, D7),
                }
            },
            {
                "|r|n|b|q|k| | |r|\n" \
                "|p|p|p| |p|p|p|p|\n" \
                "| | | |N| | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "|P|P|P|P|P|P|P|P|\n" \
                "|R|N|B|Q|K| |N|R|\n" \
                "b KQkq - 0 2",
                {
                    Move(C7, D6),
                    Move(E7, D6),
                    Move(D8, D6),
                    Move(E8, F8),
                    Move(E8, D7),
                }
            },
            {
                "| |K| | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | |p|p|\n" \
                "|R| | | | | | |k|\n" \
                "b - - 0 2",
                {
                    Move::make_promotion(G2, G1, KNIGHT),
                    Move::make_promotion(G2, G1, BISHOP),
                    Move::make_promotion(G2, G1, QUEEN),
                    Move::make_promotion(G2, G1, ROOK),
                }
            },
            {
                "|r| | | | | | |K|\n" \
                "| | | | | | |P|P|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| |k| | | | | | |\n" \
                "w - - 0 2",
                {
                    Move::make_promotion(G7, G8, KNIGHT),
                    Move::make_promotion(G7, G8, BISHOP),
                    Move::make_promotion(G7, G8, QUEEN),
                    Move::make_promotion(G7, G8, ROOK),
                }
            },
            {
                "|k| | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | |P|\n" \
                "|r| | | | | | |K|\n" \
                "|r| | | | | | |R|\n" \
                "| | | | | | |P| |\n" \
                "| | | | | | | | |\n" \
                "w - - 0 2",
                {
                    Move(G2, G4),
                    Move(H4, G5),
                }
            },
            {
                "| | | | | | | | |\n" \
                "| | | | | | |p| |\n" \
                "| | | | | | | |p|\n" \
                "|R| | | | | | |k|\n" \
                "| | | | | | | |p|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "|K| | | | | | | |\n" \
                "b - - 0 2",
                {
                    Move(G7, G5),
                    Move(H5, G4),
                    Move(H5, G6),
                }
            },
        };

        for (auto& t : ts) {
                auto position = Position::from_ascii(std::get<0>(t));
                auto expect = std::get<1>(t);
                std::sort(expect.begin(), expect.end());
                auto result = gen_moves(position);
                REQUIRE(result == expect);
        }

    } // evasions

    SECTION("non-evasions")
    {
        std::vector<std::pair<std::string, MoveList>> ts = {
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | |P| | | |\n" \
                "| | | | |K| | | |\n" \
                "w - - 0 2",
                {
                    Move(E1, D1),
                    Move(E1, F1),
                    Move(E1, D2),
                    Move(E1, F2),
                    Move(E2, E3),
                    Move(E2, E4),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | |N| | | |\n" \
                "| | | | |P| | | |\n" \
                "| | | | |K| | | |\n" \
                "w - - 0 2",
                {
                    Move(E1, D1),
                    Move(E1, F1),
                    Move(E1, D2),
                    Move(E1, F2),
                    Move(E3, D1),
                    Move(E3, C2),
                    Move(E3, C4),
                    Move(E3, D5),
                    Move(E3, F5),
                    Move(E3, G4),
                    Move(E3, G2),
                    Move(E3, F1),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | |p| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | |P| | | |\n" \
                "| | | | |K| | | |\n" \
                "b - - 0 2",
                {
                    Move(E8, D8),
                    Move(E8, F8),
                    Move(E8, D7),
                    Move(E8, F7),
                    Move(E7, E6),
                    Move(E7, E5),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | |p| | | |\n" \
                "| | | | |n| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | |P| | | |\n" \
                "| | | | |K| | | |\n" \
                "b - - 0 2",
                {
                    Move(E8, D8),
                    Move(E8, F8),
                    Move(E8, D7),
                    Move(E8, F7),
                    Move(E6, D8),
                    Move(E6, C7),
                    Move(E6, C5),
                    Move(E6, D4),
                    Move(E6, F4),
                    Move(E6, G5),
                    Move(E6, G7),
                    Move(E6, F8),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | |P|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | |p|\n" \
                "| | | | |K| | | |\n" \
                "w - - 0 2",
                {
                    Move(E1, D1),
                    Move(E1, D2),
                    Move(E1, E2),
                    Move(E1, F1),
                    Move(E1, F2),
                    Move::make_promotion(H7, H8, KNIGHT),
                    Move::make_promotion(H7, H8, BISHOP),
                    Move::make_promotion(H7, H8, ROOK),
                    Move::make_promotion(H7, H8, QUEEN),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | |P|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | |p|\n" \
                "| | | | |K| | | |\n" \
                "b - - 0 2",
                {
                    Move(E8, D8),
                    Move(E8, D7),
                    Move(E8, E7),
                    Move(E8, F8),
                    Move(E8, F7),
                    Move::make_promotion(H2, H1, KNIGHT),
                    Move::make_promotion(H2, H1, BISHOP),
                    Move::make_promotion(H2, H1, ROOK),
                    Move::make_promotion(H2, H1, QUEEN),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | |p|P| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | |p|\n" \
                "| | | | |K| | | |\n" \
                "w - d6 0 2",
                {
                    Move(E1, D1),
                    Move(E1, D2),
                    Move(E1, E2),
                    Move(E1, F1),
                    Move(E1, F2),
                    Move(E5, E6),
                    Move::make_enpassant(E5, D6),
                }
            },
            {
                "| | | | |k| | | |\n" \
                "| | | | |r| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | |p|P| | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | |p|\n" \
                "| | | | |K| | | |\n" \
                "w - d6 0 2",
                {
                    Move(E1, D1),
                    Move(E1, D2),
                    Move(E1, E2),
                    Move(E1, F1),
                    Move(E1, F2),
                    Move(E5, E6),
                }
            },
            {
                "| | | | |k| | |r|\n" \
                "| | | | |p|p|p|p|\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | | | | | |\n" \
                "| | | | |P|P|P|P|\n" \
                "| | | | |K| | |R|\n" \
                "w K - 0 2",
                {
                    Move(E1, D1),
                    Move(E1, D2),
                    Move(E1, F1),
                    Move(H1, F1),
                    Move(H1, G1),
                    Move(E2, E3),
                    Move(E2, E4),
                    Move(F2, F3),
                    Move(F2, F4),
                    Move(G2, G3),
                    Move(G2, G4),
                    Move(H2, H3),
                    Move(H2, H4),
                    Move::make_castle(Castle::WHITE_KING_SIDE),
                }
            },
        };

        for (auto& t : ts) {
            auto desc = std::get<0>(t);
            auto position = Position::from_ascii(desc);
            auto expect = std::get<1>(t);
            std::sort(expect.begin(), expect.end());
            auto result = gen_moves(position);
            REQUIRE(result == expect);
        }
    }
}

TEST_CASE("in_check", "[position]")
{
    Zobrist::initialize();

    std::vector<std::tuple<std::string, bool, bool>> ts = {
        { "k7/8/8/8/8/8/1K6/6Nr w - - 0 1", false, false, },
        { "k7/8/8/8/8/8/1K5r/6N1 w - - 0 1", true, false, },
        { "k7/8/5b2/8/8/8/1K6/6N1 w - - 0 1", true, false, },
        { "k7/8/8/8/2n5/8/1K6/6N1 w - - 0 1", true, false, },
        { "k7/8/8/4b3/3n4/8/1K6/6N1 w - - 0 1", false, false, },
        { "k6q/8/8/8/8/8/1K6/6N1 w - - 0 1", true, false, },
        { "k7/8/8/Q7/8/8/1K6/6N1 w - - 0 1", false, true, },
        { "k6R/8/8/8/8/8/1K6/6N1 w - - 0 1", false, true, },
        { "k7/8/8/8/8/8/1K4B1/6N1 w - - 0 1", false, true, }
    };

    for (auto& t : ts) {
        auto& fen = std::get<0>(t);
        auto white_expect = std::get<1>(t);
        auto black_expect = std::get<2>(t);
        auto position = Position::from_fen(fen);
        auto white_result = position.in_check(Color::WHITE);
        auto black_result = position.in_check(Color::BLACK);
        REQUIRE(white_result == white_expect);
        REQUIRE(black_result == black_expect);
    }
}

TEST_CASE("zobrist", "[position]")
{
    Zobrist::initialize();

    SECTION("1.e4 - pawn move that changes e.p. target square")
    {
        std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        std::string move = "e2e4";
        std::string fen2 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
        // position.undo_move(sp, m);
        // TODO: uncomment this
        // REQUIRE(position == original);
    }

    SECTION("Pawn capture -- does not change castle flags")
    {
        std::string fen1 = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1";
        std::string move = "e4d5";
        std::string fen2 = "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
    }

    SECTION("white king side castle")
    {
        std::string fen1 = "r1bqkb1r/1pp2ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
        std::string move = "e1g1"; // == 0-0
        std::string fen2 = "r1bqkb1r/1pp2ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        REQUIRE(m.is_castle());
        REQUIRE(m.castle_kind() == Castle::WHITE_KING_SIDE);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
    }

    SECTION("white queen side castle")
    {
        std::string fen1 = "r1bqkb1r/1pp2ppp/p1n2n2/4p3/B3P3/2NPBN2/PPPQ1PPP/R3K2R w KQkq - 0 1";
        std::string move = "e1c1";
        std::string fen2 = "r1bqkb1r/1pp2ppp/p1n2n2/4p3/B3P3/2NPBN2/PPPQ1PPP/2KR3R b kq - 1 1";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        REQUIRE(m.is_castle());
        REQUIRE(m.castle_kind() == Castle::WHITE_QUEEN_SIDE);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
    }

    SECTION("black king side castle")
    {
        std::string fen1 = "r1bqk2r/1pp1bppp/p1n2n2/4p3/B3P3/2NPBN2/PPPQ1PPP/R3K1R1 b Qkq - 0 1";
		std::string move = "e8g8";
		std::string fen2 = "r1bq1rk1/1pp1bppp/p1n2n2/4p3/B3P3/2NPBN2/PPPQ1PPP/R3K1R1 w Q - 1 2";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        REQUIRE(m.is_castle());
        REQUIRE(m.castle_kind() == Castle::BLACK_KING_SIDE);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
    }

    SECTION("black queen side castle")
    {
        std::string fen1 = "r3k2r/1ppqbppp/p1n1bn2/4p3/B3P3/2NPBN2/PPPQ1PPP/R3K1R1 b Qq - 0 1";
        std::string move = "e8c8";
        std::string fen2 = "2kr3r/1ppqbppp/p1n1bn2/4p3/B3P3/2NPBN2/PPPQ1PPP/R3K1R1 w Q - 1 2";
        Position position = Position::from_fen(fen1);
        REQUIRE(position.zobrist_hash() != 0u);
        Position original = position;
        Move m = position.move_from_long_algebraic(move);
        REQUIRE(m.is_castle());
        REQUIRE(m.castle_kind() == Castle::BLACK_QUEEN_SIDE);
        Savepos sp;
        position.make_move(sp, m);
        Position expected = Position::from_fen(fen2);
        REQUIRE(expected.zobrist_hash() != 0u);
        REQUIRE(position == expected);
    }

    // SECTION("white en passant capture")
    // {

    // }

    // black en passant capture
    // white promotion
    // black promotion
    // white capture of rook changing king side castle rights
    // black capture of rook changing king side castle rights
    // white capture of rook changing queen side castle rights
    // black capture of rook changing queen side castle rights
}
