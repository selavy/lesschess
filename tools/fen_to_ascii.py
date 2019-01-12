#!/usr/bin/env python

import sys


def print_board(o):
    """Taken from https://codegolf.stackexchange.com/questions/78326/convert-a-forsyth-edwards-notation-string-to-ascii-art
    R. Kap's solution
    """
    print(''.join(['| '*8if h=='8'else'| '*int(h)if h.isdigit()else'|\n'if h=='/'else'|'+h for h in o])+'|')



def fen_to_ascii(fen):
    parts = fen.split(' ')
    print_board(parts[0])
    parts.pop(0)
    parts = [x for x in parts if x]
    if len(parts) == 0:  # no side to move
        parts.append('w')
    if len(parts) == 1:  # no castle rights
        parts.append('-')
    if len(parts) == 2:  # no e.p. square
        parts.append('-')
    if len(parts) == 3:  # no halfmoves number
        parts.append('0')
    if len(parts) == 4:  # no move number
        parts.append('1')


    print(' '.join(parts))


# fen_to_ascii("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
if len(sys.argv) == 2:
    fen_to_ascii(sys.argv[1])
else:
    print(f"usage: {sys.argv[0]} <fen>")
    print('    example: {sys.argv[0]} "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"')

