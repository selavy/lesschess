#include "catch.hpp"
#include "position.h"
#include <iostream>
#include <map>

using namespace lesschess;

TEST_CASE("Position from FEN")
{
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
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
        REQUIRE(position.enpassant_available() == false);
        REQUIRE(position.fifty_move_rule_moves() == 0);
        REQUIRE(position.move_number() == 1);

        // flip side to move a couple times to verify that...
        REQUIRE(position.white_to_move() == true);
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
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
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
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
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
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
        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
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
        REQUIRE(position.castle_flags() == Position::CASTLE_NONE);
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
        REQUIRE(position.castle_flags() == (Position::CASTLE_BLACK_KING_SIDE | Position::CASTLE_BLACK_QUEEN_SIDE));
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
        REQUIRE(position.castle_flags() == (Position::CASTLE_WHITE_KING_SIDE | Position::CASTLE_WHITE_QUEEN_SIDE));
        REQUIRE(position.enpassant_available() == false);
    }

}

TEST_CASE("Position::dump_fen")
{
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

TEST_CASE("Position::make_move")
{
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

        REQUIRE(position.castle_flags() == Position::CASTLE_ALL);
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
            { Move::make_castle(Move::CastleKind::WHITE_KING_SIDE), "r1bqkb1r/pp1p1ppp/2n1pn2/8/3NP3/8/PPP1BPPP/RNBQ1RK1 b kq - 3 6", },
            { Move(B7, B6), "r1bqkb1r/p2p1ppp/1pn1pn2/8/3NP3/8/PPP1BPPP/RNBQ1RK1 w kq - 0 7" },
            { Move(F1, E1), "r1bqkb1r/p2p1ppp/1pn1pn2/8/3NP3/8/PPP1BPPP/RNBQR1K1 b kq - 1 7" },
            { Move(C8, A6), "r2qkb1r/p2p1ppp/bpn1pn2/8/3NP3/8/PPP1BPPP/RNBQR1K1 w kq - 2 8" },
            { Move(B1, C3), "r2qkb1r/p2p1ppp/bpn1pn2/8/3NP3/2N5/PPP1BPPP/R1BQR1K1 b kq - 3 8" },
            { Move(D8, C7), "r3kb1r/p1qp1ppp/bpn1pn2/8/3NP3/2N5/PPP1BPPP/R1BQR1K1 w kq - 4 9" },
            { Move(C1, G5), "r3kb1r/p1qp1ppp/bpn1pn2/6B1/3NP3/2N5/PPP1BPPP/R2QR1K1 b kq - 5 9" },
            { Move::make_castle(Move::CastleKind::BLACK_QUEEN_SIDE), "2kr1b1r/p1qp1ppp/bpn1pn2/6B1/3NP3/2N5/PPP1BPPP/R2QR1K1 w - - 6 10" },
            { Move(E4, E5), "2kr1b1r/p1qp1ppp/bpn1pn2/4P1B1/3N4/2N5/PPP1BPPP/R2QR1K1 b - - 0 10" },
            { Move(D7, D5), "2kr1b1r/p1q2ppp/bpn1pn2/3pP1B1/3N4/2N5/PPP1BPPP/R2QR1K1 w - d6 0 11" },
            { Move(E5, D6, ep_capture_tag{}), "2kr1b1r/p1q2ppp/bpnPpn2/6B1/3N4/2N5/PPP1BPPP/R2QR1K1 b - - 0 11" },
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
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
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
        FEN starting_position = "4k3/7P/8/8/8/8/8/4K3 w KQkq - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;

        std::vector<TestCase> test_cases = {
            { Move(H7, H8, ROOK), "4k2R/8/8/8/8/8/8/4K3 b KQkq - 0 1" }, // h8=R
            { Move(E8, D7),       "7R/3k4/8/8/8/8/8/4K3 w KQ - 1 2"   }, // Kd7
        };

        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
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
		// b KQkq - 0 1
        FEN starting_position = "4k3/8/8/8/8/8/7p/4K3 b KQkq - 0 1";
        Position position = Position::from_fen(starting_position);
        Savepos save;

        std::vector<TestCase> test_cases = {
            { Move(H2, H1, KNIGHT), "4k3/8/8/8/8/8/8/4K2n w KQkq - 0 2" }, // h1=N
            { Move(E1, D2),         "4k3/8/8/8/8/8/3K4/7n b kq - 1 2"   }, // Kd2
        };

        for (auto&& test_case: test_cases) {
            auto&& move = std::get<0>(test_case);
            auto&& fen = std::get<1>(test_case);
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == fen);
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
        Move move(H7, G8, QUEEN);
        position.make_move(save, move);
        REQUIRE(position.dump_fen() == "4k1Q1/8/8/8/8/8/8/4K3 b - - 0 1");
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
        std::vector<std::pair<Move::CastleKind, FEN>> test_cases = {
            { Move::CastleKind::WHITE_KING_SIDE, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R4RK1 b kq - 1 1" },
            { Move::CastleKind::WHITE_QUEEN_SIDE, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/2KR3R b kq - 1 1"},
        };

        for (auto&& test_case: test_cases) {
            auto&& kind = std::get<0>(test_case);
            auto&& expected = std::get<1>(test_case);
            Position position = Position::from_fen(original_fen);
            Move move = Move::make_castle(kind);
            Savepos save;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == expected);
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
        std::vector<std::pair<Move::CastleKind, FEN>> test_cases = {
            { Move::CastleKind::BLACK_KING_SIDE, "r4rk1/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 1 2" },
            { Move::CastleKind::BLACK_QUEEN_SIDE, "2kr3r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQ - 1 2" },
        };

        for (auto&& test_case: test_cases) {
            auto&& kind = std::get<0>(test_case);
            auto&& expected = std::get<1>(test_case);
            Position position = Position::from_fen(original_fen);
            Move move = Move::make_castle(kind);
            Savepos save;
            position.make_move(save, move);
            REQUIRE(position.dump_fen() == expected);
        }

    }

}

TEST_CASE("Undo Move")
{
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

        std::array<Move::CastleKind, 2> kinds = {
            Move::CastleKind::WHITE_KING_SIDE,
            Move::CastleKind::WHITE_QUEEN_SIDE,
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

        std::array<Move::CastleKind, 2> kinds = {
            Move::CastleKind::BLACK_KING_SIDE,
            Move::CastleKind::BLACK_QUEEN_SIDE,
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
            Move move(H7, H8, kind);
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
            Move move(A2, A1, kind);
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
        Move move(H5, G6, ep_capture_tag{});
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

    SECTION("Undo black en passant") {
        std::string fen = "4k3/8/8/6pP/pP6/8/8/4K3 b - b3";
        Position position = Position::from_fen(fen);
        Position position_copy = position;
        Savepos save;
        Move move(A4, B3, ep_capture_tag{});
        position.make_move(save, move);
        position.undo_move(save, move);
        REQUIRE(position == position_copy);
    }

}


TEST_CASE("FEN to ASCII and back")
{
    std::vector<std::pair<std::string, std::string>> fens = {
        {
            "\n"                  \
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
            "\n"                  \
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
            "\n"                  \
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
            "\n"                  \
            "| | | | |k| | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | | |\n" \
            "| | | | | | | |p|\n" \
            "| | | | |K| | | |\n" \
            "b KQkq - 0 1",

            "4k3/8/8/8/8/8/7p/4K3 b KQkq - 0 1"
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

TEST_CASE("Square is Attacked?")
{
    SECTION("Immortal Game Position")
    {
        std::string position =
            "\n"                  \
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

TEST_CASE("Legal Move Check")
{
    using TestMove = std::pair<Move, bool>;
    std::vector<std::pair<std::string, std::vector<TestMove>>> tests = {
        {
            "\n"                  \
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
                { Move::make_castle(Move::CastleKind::WHITE_KING_SIDE),  false },
                { Move::make_castle(Move::CastleKind::WHITE_QUEEN_SIDE), false },
                { Move::make_castle(Move::CastleKind::BLACK_KING_SIDE),  false },
                { Move::make_castle(Move::CastleKind::BLACK_QUEEN_SIDE), false },
            }
        },

        {
            "\n"                  \
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
                { Move(E5, D6, ep_capture_tag{}), true  },
                { Move(E5, F6, ep_capture_tag{}), false },
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


    // std::vector<std::pair<std::string,


    // std::string desc =
    // Move move(E2, E4);

    // Position position = Position::from_ascii(desc);
    // REQUIRE(position.is_legal_move(move) == true);
}
