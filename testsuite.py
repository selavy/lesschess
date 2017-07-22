#!/usr/bin/env python

import subprocess
from collections import namedtuple
import os


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


def starting_position_perft_test():
    print "Test: starting position perft...",
    expected = (
            (0, PerftResult(1, 0, 0, 0, 0, 0, 0)),
            (1, PerftResult(20, 0, 0, 0, 0, 0, 0)),
            (2, PerftResult(400, 0, 0, 0, 0, 0, 0)),
            (3, PerftResult(8902, 34, 0, 0, 0, 12, 0)),
            (4, PerftResult(197281, 1576, 0, 0, 0, 469, 8)),
            (5, PerftResult(4865609, 82719, 258, 0, 0, 27351, 347)),
            )
    fen = start_position_fen
    for depth, e in expected:
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
    print "Passed."


if __name__ == '__main__':
    find_executable()
    starting_position_perft_test()
