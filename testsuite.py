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


def run_perft_test_suite(name, fen, expected, max_depth):
    iwrite("Test: starting position perft")
    for depth, e in expected:
        if depth > max_depth:
            break
        run_perft_test(fen, depth)
        res = run_perft_test(fen, depth)
        if res != e:
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
    max_depth = max_depth or 6
    expected = (
            (0, PerftResult(1, 0, 0, 0, 0, 0, 0)),
            (1, PerftResult(20, 0, 0, 0, 0, 0, 0)),
            (2, PerftResult(400, 0, 0, 0, 0, 0, 0)),
            (3, PerftResult(8902, 34, 0, 0, 0, 12, 0)),
            (4, PerftResult(197281, 1576, 0, 0, 0, 469, 8)),
            (5, PerftResult(4865609, 82719, 258, 0, 0, 27351, 347)),
            (6, PerftResult(119060324, 2812008, 5248, 0, 0, 809099, 10828)),
            )
    run_perft_test_suite("start position perft", start_position_fen, expected,
                         max_depth)


def kiwipete_perft_test():
    kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
    expected = (
            (1, PerftResult(48, 8, 0, 2, 0, 0, 0)),
            (2, PerftResult(2039, 351, 1, 91, 0, 3, 0)),
            )
    run_perft_test_suite("kiwi pete perft", kiwipete_fen, expected, 100)


if __name__ == '__main__':
    find_executable()
    starting_position_perft_test(5)
    kiwipete_perft_test()
