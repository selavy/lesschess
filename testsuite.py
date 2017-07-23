#!/usr/bin/env python

import subprocess
from collections import namedtuple
import os
import sys


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
    run_perft_test_suite("position6", fen, expected, max_depth,
            nodes_only=True)


if __name__ == '__main__':
    fast_mode = True

    start_max_depth = None
    kiwi_max_depth = None
    position3_max_depth = None
    position4_max_depth = None
    talkchess_max_depth = None
    position6_max_depth = None
    if fast_mode:
        start_max_depth = 4
        kiwi_max_depth = 3
        position3_max_depth = 5
        position4_max_depth = 4
        talkchess_max_depth = 4
        position6_max_depth = 4

    find_executable()
    starting_position_perft_test(start_max_depth)
    kiwipete_perft_test(kiwi_max_depth)
    position3_perft_test(position3_max_depth)
    position4_white_perft_test(position4_max_depth)
    position4_black_perft_test(position4_max_depth)
    talkchess_perft_test(talkchess_max_depth)
    position6_perft_test(position6_max_depth)
