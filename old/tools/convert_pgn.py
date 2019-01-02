#!/usr/bin/env python

import sys
import chess.pgn


if __name__ == '__main__':
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        filename = "silly_games.txt"

    with open(filename) as pgn:
        game = chess.pgn.read_game(pgn)

    node = game
    while not node.is_end():
        next_node = node.variations[0]
        print node.board().uci(next_node.move)
        node = next_node
