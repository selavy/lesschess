#!/usr/bin/env python

import subprocess
from collections import namedtuple
import os
import sys
import chess


PerftResult = namedtuple("PerftResult", [
    "nodes",
    "captures",
    "enpassants",
    "castles",
    "promotions",
    "checks",
    "mates"])


start_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
engine = "lesschess"
target = ""


def iwrite(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def find_executable():
    global target
    locations = (".", "./build")
    for loc in locations:
        target = loc + "/" + engine
        if os.path.isfile(target):
            return
    raise Exception("Unable to find engine!")


def run_perft_test(fen, depth):
    cmd = '{} perft "{fen}" {depth}'.format(target, fen=fen, depth=depth)
    output = subprocess.check_output(cmd, shell=True)
    nodes, captures, eps, castles, promos, checks, mates = output.split()
    return PerftResult(nodes=int(nodes), captures=int(captures),
                       enpassants=int(eps), castles=int(castles),
                       promotions=int(promos), checks=int(checks),
                       mates=int(mates))


def run_perft_test_suite(name, fen, expected, max_depth=None,
        nodes_only=False):
    iwrite("Perft Test: {}".format(name))
    max_depth = max_depth or 1000
    for depth, e in expected:
        if depth > max_depth:
            break
        run_perft_test(fen, depth)
        res = run_perft_test(fen, depth)
        if (nodes_only and res.nodes != e) or (not nodes_only and res != e):
            print ""
            print "Failed perft test case!!"
            print "FEN: ", fen
            print "Depth: ", depth
            print "Expected: ", str(e)
            print "Actual  : ", str(res)
            return
        iwrite('.')
    print "Passed."

print_cmd = True
def run_tactics_test(name, fen, expected_move=None, expected_score=None):
    import datetime

    print "Tactics Test: {}".format(name)
    board = chess.Board(fen)
    cmd = '{exe} tactics "{fen}"'.format(exe=target, fen=fen)
    if False:
        print "{!s}".format(board)
        print "Running command '{}'".format(cmd)
    begin = datetime.datetime.now()
    output = subprocess.check_output(cmd, shell=True)
    end = datetime.datetime.now()
    print "Test took {}.".format(end - begin)
    actual_move, actual_score, actual_depth = output.split()
    actual_score = int(actual_score)
    actual_depth = int(actual_depth)
    if actual_move == "mated":
        print "Failed!"
        print "Engine thinks it is mated!"
    elif actual_move == "none":
        print "Failed!"
        print "Engine thinks it has no legal moves!"
    elif expected_move is not None or expected_score is not None:
        actual_move_san = board.san(chess.Move.from_uci(actual_move))
        if expected_move is None:
            expected_move = actual_move
        expected_move_san = board.san(chess.Move.from_uci(expected_move))
        if expected_score is None:
            expected_score = actual_score
        if actual_move != expected_move or expected_score != actual_score:
            print "Failed!"
            print "Expected Move : {} ({})".format(
                expected_move_san, expected_move)
            print "Actual Move   : {} ({})".format(
                actual_move_san, actual_move)
            print "Expected Score: {}".format(expected_score)
            print "Actual Score  : {}".format(actual_score)
            print "Actual Depth  : {}".format(actual_depth)
        else:
            print "Move : {} ({})".format(actual_move_san, actual_move)
            print "Score: {}".format(actual_score)
            print "Depth: {}".format(actual_depth)
            print "Passed."
    else:
        raise Exception("Must provide either expected move or score!")
    print ""


def starting_position_perft_test(max_depth=None):
    expected = (
            (0, PerftResult(1, 0, 0, 0, 0, 0, 0)),
            (1, PerftResult(20, 0, 0, 0, 0, 0, 0)),
            (2, PerftResult(400, 0, 0, 0, 0, 0, 0)),
            (3, PerftResult(8902, 34, 0, 0, 0, 12, 0)),
            (4, PerftResult(197281, 1576, 0, 0, 0, 469, 8)),
            (5, PerftResult(4865609, 82719, 258, 0, 0, 27351, 347)),
            (6, PerftResult(119060324, 2812008, 5248, 0, 0, 809099, 10828)),
            )
    run_perft_test_suite("start position", start_position_fen, expected,
                         max_depth)


def kiwipete_perft_test(max_depth=None):
    fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
    expected = (
            (1, PerftResult(48, 8, 0, 2, 0, 0, 0)),
            (2, PerftResult(2039, 351, 1, 91, 0, 3, 0)),
            (3, PerftResult(97862, 17102, 45, 3162, 0, 993, 1)),
            (4, PerftResult(4085603, 757163, 1929, 128013, 15172, 25523, 43)),
            (5, PerftResult(193690690, 35043416, 73365, 4993637, 8392, 3309887, 30171)),
            )
    run_perft_test_suite("kiwi pete", fen, expected, max_depth)


def position3_perft_test(max_depth=None):
    fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
    expected = (
            (1, PerftResult(14, 1, 0, 0, 0, 2, 0)),
            (2, PerftResult(191, 14, 0, 0, 0, 10, 0)),
            (3, PerftResult(2812, 209, 2, 0, 0, 267, 0)),
            (4, PerftResult(43238, 3348, 123, 0, 0, 1680, 17)),
            (5, PerftResult(674624, 52051, 1165, 0, 0, 52950, 0)),
            (6, PerftResult(11030083, 940350, 33325, 0, 7552, 452473, 2733)),
            (7, PerftResult(178633661, 14519036, 294874, 0, 140024, 12797406, 87)),
            )
    run_perft_test_suite("position3", fen, expected, max_depth)


def position4_white_perft_test(max_depth=None):
    fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    expected = (
            (1, PerftResult(6, 0, 0, 0, 0, 0, 0)),
            (2, PerftResult(264, 87, 0, 6, 48, 10, 0)),
            (3, PerftResult(9467, 1021, 4, 0, 120, 38, 22)),
            (4, PerftResult(422333, 131393, 0, 7795, 60032, 15492, 5)),
            (5, PerftResult(15833292, 2046173, 6512, 0, 329464, 200568, 50562)),
            (6, PerftResult(706045033, 210369132, 212, 10882006, 81102984, 26973664, 81076)),
            )
    run_perft_test_suite("position4 white", fen, expected, max_depth)


def position4_black_perft_test(max_depth=None):
    fen = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
    expected = (
            (1, PerftResult(6, 0, 0, 0, 0, 0, 0)),
            (2, PerftResult(264, 87, 0, 6, 48, 10, 0)),
            (3, PerftResult(9467, 1021, 4, 0, 120, 38, 22)),
            (4, PerftResult(422333, 131393, 0, 7795, 60032, 15492, 5)),
            (5, PerftResult(15833292, 2046173, 6512, 0, 329464, 200568, 50562)),
            (6, PerftResult(706045033, 210369132, 212, 10882006, 81102984, 26973664, 81076)),
            )
    run_perft_test_suite("position4 black", fen, expected, max_depth)


def talkchess_perft_test(max_depth=None):
    fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    expected = (
            (1, 44),
            (2, 1486),
            (3, 62379),
            (4, 2103487),
            (5, 89941194),
            )
    run_perft_test_suite("talkchess", fen, expected, max_depth,
            nodes_only=True)


def position6_perft_test(max_depth=None):
    fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    expected = (
            (0, 1),
            (1, 46),
            (2, 2079),
            (3, 89890),
            (4, 3894594),
            (5, 164075551),
            (6, 6923051137),
            (7, 287188994746),
            (8, 11923589843526),
            (9, 490154852788714),
            )
    run_perft_test_suite("position6", fen, expected, max_depth, nodes_only=True)


def tactics_froms_gambit_mate():
    fen = "rnbqk1nr/ppp2p1p/3b4/6p1/8/5N1P/PPPPP1P1/RNBQKB1R b KQkq - 0 1"
    run_tactics_test("From's Gambit Mate in 1", fen, "d6g3")


def tactics_knight_sack_mate():
    fen = "rn3r1k/6pp/1pN2p2/p3N3/1P5q/PQ2PPp1/5n2/3R2K1 w - - 0 1"
    run_tactics_test("Knight Sack Mate", fen, "e5f7")


def tactics_queen_sack_mate():
    fen = "rn3r1k/6pp/1pN2p2/p3N3/1P5q/PQ2PPp1/5n2/2R3K1 w - - 0 1"
    run_tactics_test("Queen Sack Mate", fen, "b3g8")


def tactics_double_check_mate():
    fen = "rn2r1k1/5Npp/1pN2p2/p7/1P5q/PQ2PPp1/5n2/3R2K1 w - - 0 1"
    run_tactics_test("Double Check For Mate", fen, "f7h6")

def tactics_queen_sack_smothered_mate():
    fen = "rn2r2k/6pp/1pN2p1N/p7/1P5q/PQ2PPp1/5n2/3R2K1 w - - 0 1"
    run_tactics_test("Queen Sack Smothered Mate", fen, "b3g8")


def tactics_caro_kann_mate():
    fen = "rnb1kb1r/pp3ppp/2p5/4q3/4n3/3Q4/PPPB1PPP/2KR1BNR w kq - 0 1"
    run_tactics_test("Caro Kann Mate", fen, "d3d8")

def tactics_win_queen():
    fen = "rnbqk2r/ppp2ppp/3b4/8/2P1n3/5NP1/PP2PP1P/RNBQKB1R b KQkq - 0 1"
    run_tactics_test("Early Game Win Queen", fen, "e4f2")

def tactics_50_move_rule_draw():
    fen = "8/8/8/8/3k4/3P4/3K4/8 w - - 98 1"
    run_tactics_test("50-move Rule Draw", fen, expected_score=0.)

epd_records = (
    '1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - bm Qd1+; id "BK.01";',
    '3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - bm d5; id "BK.02";',
    '2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - bm f5; id "BK.03";',
    'rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - bm e6; id "BK.04";',
    'r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - bm Nd5 a4; id "BK.05";',
    '2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - bm g6; id "BK.06";',
    '1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - bm Nf6; id "BK.07";',
    '4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - bm f5; id "BK.08";',
    '2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - bm f5; id "BK.09";',
    '3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - bm Ne5; id "BK.10";',
    '2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - bm f4; id "BK.11";',
    'r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - bm Bf5; id "BK.12";',
    'r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - bm b4; id "BK.13";',
    'rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - bm Qd2 Qe1; id "BK.14";', # TODO(plesslie): fix this test to accept either move
    '2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - bm Qxg7+; id "BK.15";',
    'r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - bm Ne4; id "BK.16";',
    'r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - bm h5; id "BK.17";',
    'r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - bm Nb3; id "BK.18";',
    '3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - bm Rxe4; id "BK.19";',
    'r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - bm g4; id "BK.20";',
    '3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - bm Nh6; id "BK.21";',
    '2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - bm Bxe4; id "BK.22";',
    'r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - bm f6; id "BK.23";',
    'r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - bm f4; id "BK.24";',
)

def run_epd_test(name, fen, expected_move):
    import datetime

    print "Tactics Test: {}".format(name)
    board = chess.Board(fen)
    print "{!s}".format(board)
    cmd = '{exe} tactics "{fen}"'.format(exe=target, fen=fen)
    print "Running command '{}'".format(cmd)
    begin = datetime.datetime.now()
    output = subprocess.check_output(cmd, shell=True)
    end = datetime.datetime.now()
    print "Test took {}.".format(end - begin)
    actual_move, actual_score, actual_depth = output.split()
    actual_score = int(actual_score)
    actual_depth = int(actual_depth)
    if actual_move == "mated":
        print "Failed!"
        print "Engine thinks it is mated!"
    elif actual_move == "none":
        print "Failed!"
        print "Engine thinks it has no legal moves!"
    else:
        actual_move_san = board.san(chess.Move.from_uci(actual_move))
        expected_move_san = expected_move
        if actual_move_san != expected_move_san:
            print "Failed!"
            print "Expected Move : {} ({})".format(
                expected_move_san, expected_move)
            print "Actual Move   : {} ({})".format(
                actual_move_san, actual_move)
            print "Actual Score  : {}".format(actual_score)
            print "Actual Depth  : {}".format(actual_depth)
        else:
        #     print "Move : {} ({})".format(actual_move_san, actual_move)
        #     print "Score: {}".format(actual_score)
        #     print "Depth: {}".format(actual_depth)
            print "Passed."
    print ""


def tactics_epd_records():
    for record in epd_records:
        fen, rest = record.split('bm', 1)
        expected_move, rest = rest.split(';', 1)
        expected_move = expected_move.strip()
        name = rest[5:-2]
        fen += "0 0"
        # print "Name=",name
        # print "FEN=",fen
        run_epd_test(name, fen, expected_move)
        break  # for now, just the first test
        

if __name__ == '__main__':
    fast_mode = True

    perft_suites = (
        ("start", starting_position_perft_test, 4),
        ("kiwi", kiwipete_perft_test, 3),
        ("position3", position3_perft_test, 5),
        ("white_position4", position4_white_perft_test, 4),
        ("black_position4", position4_black_perft_test, 4),
        ("talkchess", talkchess_perft_test, 4,),
        ("position6", position6_perft_test, 4,),
    )

    tactics_suites = (
        ("froms_gambit", tactics_froms_gambit_mate),
        ("knight_sack", tactics_knight_sack_mate),
        # Too hard right now
        # ("queen_sack", tactics_queen_sack_mate),
        ("double_check", tactics_double_check_mate),
        ("queen_sack_smothered", tactics_queen_sack_smothered_mate),
        ("caro_kann", tactics_caro_kann_mate),
	("win_queen", tactics_win_queen),
        ("50_move_rule", tactics_50_move_rule_draw),
        ("edp_tactics", tactics_epd_records),
    )

    available_suites = {}
    for name, suite, depth in perft_suites:
        available_suites[name] = (suite, (depth, ))

    for name, suite in tactics_suites:
        available_suites[name] = (suite, tuple())

    suites = set()
    fast_mode = True
    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            if arg == "slow":
                fast_mode = False
            elif arg == "tactics":
                for s in tactics_suites:
                    suites.add(available_suites[s[0]])
            elif arg == "perft":
                for s in perft_suites:
                    suites.add(available_suites[s[0]])
            else:
                suites.add(available_suites[arg])
    suites = tuple(sorted(suites))

    if not suites:
        suites = tuple(sorted(available_suites.itervalues()))

    find_executable()
    for func, args in suites:
        func(*args)
