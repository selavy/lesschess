[[PLAN]]
1. Install libfmt [DONE: see tests/test_libfmt.cpp]
2. Read position from FEN [DONE]
    * TODO(peter): go back and harden this code.  Works well enough for now
3. Move generation + perft test harness [DONE]
4. Basic UCI protocol playing random move game in Arena against itself [DONE]
  * Load engine, Ctrl-D makes it start playing itself
5. Alpha-Beta search with fixed piece values to fixed depth [DONE]
  * Fix not checking for checkmate, stalemate, etc [DONE]
      * Need a test for 50-move rule
  * Fix not checking for 3-move repetition
6. Transposition Table [DONE]
  * Implement Zobrist hashing [DONE]
  * Benchmark some hashtables -- khash, abseil, google dense hashmap
7. Implement a quiescence search
8. Switch to iterative deepening search
9. Move search to a separate thread -- started and stopped by UCI thread
10. Make search iterative (instead of recursive) with explicit stack

[[NOTES]]
+Bitboard move generation
    +Maybe implement this in python and have frontend translate to c++ backend?
        +https://github.com/niklasf/python-chess
+Test framework:
    +Perft
        +Generate legal moves in certain positions (en passant, castling shenanigans, etc)
    +Tactics positions
+UCI support
    +Hook up to xboard with Polyglot
        +https://github.com/ddugovic/polyglot
        +http://wbec-ridderkerk.nl/html/details1/PolyGlot.html
+MiniMax tree pruning
	+https://www.chessprogramming.org/Principal_Variation_Search
+Quiescence Search
    +https://www.chessprogramming.org/Quiescence_Search
+Zobrist Hashing + TT table
    +https://www.chessprogramming.org/Zobrist_Hashing
    +https://www.chessprogramming.org/Transposition_Table
+Iterative Deepening search
    +https://www.chessprogramming.org/Iterative_Deepening
+Parallel Search
+Hook up to lichess:
    +https://lichess.org/api#tag/Chess-Bot
    +https://lichess.org/blog/WvDNticAAMu_mHKP/welcome-lichess-bots
+Syzygy tablebase support
    +https://github.com/syzygy1/tb
+SEE
    +https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
+Neural Net + Reinforcement Learning training for tree pruning/eval tuning
+Windows support? (probably not at this point)


OLD TODO:
Non-Engine Improvements
=======================
+syzygy tablebases
+opening book

Engine Improvements
===================
+Iterative Deepening Search + Time scheduling
+Move Ordering  - Killer Move Heuristic
+Transposition Table
+Incorporate piece placement into eval function
+Parallelize search
+3-move repetition draw (use TT table?)
+Aspiration Window

Search Algorithms
====================
+Principal Variation Search (PVS/NegaScout) - https://en.wikipedia.org/wiki/Principal_variation_search
+MTD(f)

Testing
========
+test harness for 1-3 move mate tactics [DONE]
+test harness for perft [DONE]

Misc
====
+Cleanup xboard uci code

