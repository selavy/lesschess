#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include "lesschess.h"

using namespace lesschess;

std::string VERSION = "0.0.1";

std::string engine_info() {
    std::string ss;
    ss += "LessChess ";
    ss += VERSION;
    ss += " by Peter Lesslie";
    return ss;
}

// 2019-08-14 16:10:50.878-->1:position startpos moves d2d4 g8f6 c2c4
// 2019-08-14 16:10:50.878-->1:go wtime 292121 btime 300000 winc 0 binc 0
// 2019-08-14 16:10:50.878<--1:bestmove g8f6 ponder c2c4

// Example from Stockfish:
// 2019-08-14 16:11:53.317*1*----------------Starting engine 1 Stockfish 8 64 Bit Modern-----------------
// 2019-08-14 16:11:53.328*1*Configured Engine 1 Type:   UCI
// 2019-08-14 16:11:53.328*1*enginedebug:CommandLine: >>nice -1 /home/peter/arena/Engines/Stockfish/stockfish_8_x64_modern <<
// 2019-08-14 16:11:53.328*1*enginedebug:-- executing engine--
// 2019-08-14 16:11:53.330*1*enginedebug:EngineReadThread create...
// 2019-08-14 16:11:53.330*1*enginedebug:EngineReadThread created suspended
// 2019-08-14 16:11:53.330*1*enginedebug:-- Thread now starting --
// 2019-08-14 16:11:53.330*1*enginedebug:-- Tengine.Redirect finished --
// 2019-08-14 16:11:53.351<--1:-- starting engine[slotnr].EngineProcess --
// 2019-08-14 16:11:53.355<--1:-- engine[slotnr].EngineProcess.Running --
// 2019-08-14 16:11:53.355<--1:Stockfish 8 64 POPCNT by T. Romstad, M. Costalba, J. Kiiski, G. Linscott
// 2019-08-14 16:11:53.356-->1:uci
// 2019-08-14 16:11:53.520<--1:id name Stockfish 8 64 POPCNT
// 2019-08-14 16:11:53.520<--1:id author T. Romstad, M. Costalba, J. Kiiski, G. Linscott
// 2019-08-14 16:11:53.520<--1:option name Debug Log File type string default 
// 2019-08-14 16:11:53.520<--1:option name Contempt type spin default 0 min -100 max 100
// 2019-08-14 16:11:53.520<--1:option name Threads type spin default 1 min 1 max 128
// 2019-08-14 16:11:53.520<--1:option name Hash type spin default 16 min 1 max 1048576
// 2019-08-14 16:11:53.520<--1:option name Clear Hash type button
// 2019-08-14 16:11:53.521<--1:option name Ponder type check default false
// 2019-08-14 16:11:53.521<--1:option name MultiPV type spin default 1 min 1 max 500
// 2019-08-14 16:11:53.521<--1:option name Skill Level type spin default 20 min 0 max 20
// 2019-08-14 16:11:53.521<--1:option name Move Overhead type spin default 30 min 0 max 5000
// 2019-08-14 16:11:53.521<--1:option name Minimum Thinking Time type spin default 20 min 0 max 5000
// 2019-08-14 16:11:53.521<--1:option name Slow Mover type spin default 89 min 10 max 1000
// 2019-08-14 16:11:53.521<--1:option name nodestime type spin default 0 min 0 max 10000
// 2019-08-14 16:11:53.521<--1:option name UCI_Chess960 type check default false
// 2019-08-14 16:11:53.521<--1:option name SyzygyPath type string default <empty>
// 2019-08-14 16:11:53.521<--1:option name SyzygyProbeDepth type spin default 1 min 1 max 100
// 2019-08-14 16:11:53.521<--1:option name Syzygy50MoveRule type check default true
// 2019-08-14 16:11:53.521<--1:option name SyzygyProbeLimit type spin default 6 min 0 max 6
// 2019-08-14 16:11:53.522<--1:uciok
// 2019-08-14 16:11:53.541-->1:setoption name Debug Log File value 
// 2019-08-14 16:11:53.541-->1:setoption name Contempt value 0
// 2019-08-14 16:11:53.548-->1:setoption name Threads value 1
// 2019-08-14 16:11:53.548-->1:setoption name Hash value 12
// 2019-08-14 16:11:53.548-->1:setoption name Ponder value false
// 2019-08-14 16:11:53.548-->1:setoption name MultiPV value 1
// 2019-08-14 16:11:53.548-->1:setoption name Skill Level value 20
// 2019-08-14 16:11:53.548-->1:setoption name Move Overhead value 30
// 2019-08-14 16:11:53.548-->1:setoption name Minimum Thinking Time value 20
// 2019-08-14 16:11:53.548-->1:setoption name Slow Mover value 89
// 2019-08-14 16:11:53.548-->1:setoption name nodestime value 0
// 2019-08-14 16:11:53.549-->1:setoption name UCI_Chess960 value false
// 2019-08-14 16:11:53.549-->1:setoption name SyzygyPath value <empty>
// 2019-08-14 16:11:53.549-->1:setoption name SyzygyProbeDepth value 1
// 2019-08-14 16:11:53.549-->1:setoption name Syzygy50MoveRule value true
// 2019-08-14 16:11:53.549-->1:setoption name SyzygyProbeLimit value 6
// 2019-08-14 16:11:53.562-->1:isready
// 2019-08-14 16:11:53.601<--1:readyok
// 2019-08-14 16:11:58.077**----------New game---2019-08-14 16:11:58,077 Wed -------------
// 2019-08-14 16:12:10.401*1*Start calc, move no: 1
// 2019-08-14 16:12:10.405-->1:ucinewgame
// 2019-08-14 16:12:10.405-->1:isready
// 2019-08-14 16:12:10.409<--1:readyok
// 2019-08-14 16:12:10.425-->1:position startpos moves d2d4
// 2019-08-14 16:12:10.428-->1:go wtime 300000 btime 300000 winc 0 binc 0
// 2019-08-14 16:12:10.429<--1:info depth 1 seldepth 1 multipv 1 score cp -8 nodes 20 nps 20000 tbhits 0 time 1 pv d7d5
// 2019-08-14 16:12:10.433<--1:info depth 2 seldepth 2 multipv 1 score cp -47 nodes 72 nps 72000 tbhits 0 time 1 pv d7d5 e2e3
// 2019-08-14 16:12:10.433<--1:info depth 3 seldepth 3 multipv 1 score cp -8 nodes 143 nps 71500 tbhits 0 time 2 pv d7d5 e2e3 e7e6
// 2019-08-14 16:12:10.433<--1:info depth 4 seldepth 4 multipv 1 score cp -42 nodes 474 nps 237000 tbhits 0 time 2 pv d7d5 b1c3 e7e6 e2e3
// 2019-08-14 16:12:10.434<--1:info depth 5 seldepth 7 multipv 1 score cp -12 nodes 1334 nps 667000 tbhits 0 time 2 pv g8f6 c1f4 f6d5 e2e3 e7e6
// 2019-08-14 16:12:10.434<--1:info depth 6 seldepth 7 multipv 1 score cp -42 nodes 2907 nps 969000 tbhits 0 time 3 pv d7d5 g1f3 g8f6 e2e3 e7e6 b1c3
// 2019-08-14 16:12:10.434<--1:info depth 7 seldepth 8 multipv 1 score cp -8 nodes 4835 nps 1208750 tbhits 0 time 4 pv d7d5 g1f3 g8f6 e2e3 b8c6 b1c3 e7e6
// 2019-08-14 16:12:10.438<--1:info depth 8 seldepth 8 multipv 1 score cp -26 nodes 8771 nps 877100 tbhits 0 time 10 pv d7d5 e2e3 e7e6 g1f3 b8c6 c2c4 f8b4 b1c3
// 2019-08-14 16:12:10.454<--1:info depth 9 seldepth 11 multipv 1 score cp -32 nodes 16136 nps 537866 tbhits 0 time 30 pv d7d5 g1f3 b8c6 c2c4 e7e5 f3e5 d5c4 e5c6 b7c6
// 2019-08-14 16:12:10.548<--1:info depth 10 seldepth 14 multipv 1 score cp -25 nodes 53557 nps 431911 tbhits 0 time 124 pv g8f6 e2e3 e7e6 g1f3 d7d5 f1d3 f8d6 e1g1 e8g8 c2c4 d5c4 d3c4
// 2019-08-14 16:12:10.630<--1:info depth 11 seldepth 16 multipv 1 score cp -37 nodes 81576 nps 396000 tbhits 0 time 206 pv g8f6 e2e3 e7e6 g1f3 d7d5 f1d3 f8d6 e1g1 e8g8 c2c4 d5c4 d3c4
// 2019-08-14 16:12:10.729<--1:info depth 12 seldepth 16 multipv 1 score cp -28 nodes 126940 nps 416196 tbhits 0 time 305 pv d7d5 g1f3 e7e6 e2e3 c7c5 f1e2 b8c6 e1g1 c5c4 b1c3 g8f6 g1h1
// 2019-08-14 16:12:10.807<--1:info depth 13 seldepth 17 multipv 1 score cp -27 nodes 192974 nps 503848 tbhits 0 time 383 pv d7d5 g1f3 e7e6 e2e3 c7c5 f1e2 g8f6 e1g1 f8d6 d4c5 d6c5 c2c4 e8g8 c1d2 d5c4 b1c3 b8c6
// 2019-08-14 16:12:10.917<--1:info depth 14 seldepth 17 multipv 1 score cp -8 nodes 286999 nps 582148 tbhits 0 time 493 pv g8f6 e2e3 e7e6 g1f3 f8e7 f1d3 d7d5 e1g1 e8g8 b1d2 c7c5 d4c5 b8c6 e3e4 d5e4 d2e4 f6e4 d3e4
// 2019-08-14 16:12:11.066<--1:info depth 15 seldepth 21 multipv 1 score cp -16 nodes 466812 nps 727121 tbhits 0 time 642 pv g8f6 g1f3 e7e6 c1f4 d7d5 e2e3 f8d6 c2c3 b8c6 f1d3 e8g8 e1g1 c8d7 f4d6 c7d6
// 2019-08-14 16:12:11.350<--1:info depth 16 seldepth 23 multipv 1 score cp -15 nodes 950677 nps 1026649 tbhits 0 time 926 pv g8f6 g1f3 d7d5 c1f4 c8f5 e2e3 e7e6 f1e2 f8e7 e1g1 e8g8 f3e5 b8d7 b1d2 c7c5 c2c3
// 2019-08-14 16:12:11.748<--1:info depth 17 seldepth 24 multipv 1 score cp -20 nodes 1636518 nps 1236040 hashfull 906 tbhits 0 time 1324 pv g8f6 g1f3 d7d5 c1f4 c8f5 e2e3 e7e6 f1d3 f5d3 d1d3 f8e7 b1d2 e8g8 e1g1 b8c6 c2c3 f6h5 a1d1 h5f4 e3f4
// 2019-08-14 16:12:12.205<--1:info depth 18 seldepth 25 multipv 1 score cp -27 nodes 2475412 nps 1389900 hashfull 986 tbhits 0 time 1781 pv g8f6 g1f3 d7d5 c2c4 e7e6 b1c3 f8e7 c4d5 e6d5 e2e3 e8g8 f1d3 b7b6 e1g1 c7c5 f3e5 c8a6 c3b5 a6c8 d4c5 b6c5
// 2019-08-14 16:12:12.544<--1:info depth 19 seldepth 27 multipv 1 score cp -14 nodes 3069254 nps 1447761 hashfull 993 tbhits 0 time 2120 pv g8f6 g1f3 d7d5 c2c4 e7e6 b1c3 f8e7 c4d5 e6d5 e2e3 e8g8 f1d3 b7b6 e1g1 c7c5 d4c5 b6c5 e3e4 d5e4 c3e4 b8c6 c1f4 c6b4
// 2019-08-14 16:12:12.995<--1:info depth 20 seldepth 28 multipv 1 score cp -17 nodes 3803256 nps 1479290 hashfull 996 tbhits 0 time 2571 pv g8f6 g1f3 d7d5 c2c4 e7e6 b1c3 f8e7 e2e3 e8g8 c1d2 c7c5 d4c5 e7c5 f1e2 d5c4 e1g1 c8d7 f3e5 b8c6 e5d7 d8d7 e2c4
// 2019-08-14 16:12:13.691<--1:info depth 21 currmove d7d5 currmovenumber 2
// 2019-08-14 16:12:13.943<--1:info depth 21 currmove e7e5 currmovenumber 3
// 2019-08-14 16:12:13.960<--1:info depth 21 currmove e7e6 currmovenumber 4
// 2019-08-14 16:12:13.998<--1:info depth 21 currmove b8c6 currmovenumber 5
// 2019-08-14 16:12:14.008<--1:info depth 21 currmove c7c6 currmovenumber 6
// 2019-08-14 16:12:14.032<--1:info depth 21 currmove d7d6 currmovenumber 7
// 2019-08-14 16:12:14.054<--1:info depth 21 currmove f7f6 currmovenumber 8
// 2019-08-14 16:12:14.058<--1:info depth 21 currmove a7a6 currmovenumber 9
// 2019-08-14 16:12:14.072<--1:info depth 21 currmove c7c5 currmovenumber 10
// 2019-08-14 16:12:14.078<--1:info depth 21 currmove b7b5 currmovenumber 11
// 2019-08-14 16:12:14.085<--1:info depth 21 currmove g7g6 currmovenumber 12
// 2019-08-14 16:12:14.090<--1:info depth 21 currmove a7a5 currmovenumber 13
// 2019-08-14 16:12:14.097<--1:info depth 21 currmove h7h6 currmovenumber 14
// 2019-08-14 16:12:14.102<--1:info depth 21 currmove b7b6 currmovenumber 15
// 2019-08-14 16:12:14.105<--1:info depth 21 currmove b8a6 currmovenumber 16
// 2019-08-14 16:12:14.110<--1:info depth 21 currmove f7f5 currmovenumber 17
// 2019-08-14 16:12:14.113<--1:info depth 21 currmove g8h6 currmovenumber 18
// 2019-08-14 16:12:14.117<--1:info depth 21 currmove h7h5 currmovenumber 19
// 2019-08-14 16:12:14.117<--1:info depth 21 currmove g7g5 currmovenumber 20
// 2019-08-14 16:12:14.117<--1:info depth 21 seldepth 28 multipv 1 score cp -22 nodes 5751509 nps 1557830 hashfull 999 tbhits 0 time 3692 pv g8f6 c2c4 e7e6 b1c3 d7d5 e2e3 c7c5 g1f3 b8c6 c4d5 e6d5 f1e2 f8d6 d4c5 d6c5 a2a3 e8g8 e1g1 c5b6 e2d3 b6c7
// 2019-08-14 16:12:14.117<--1:info depth 22 currmove g8f6 currmovenumber 1
// 2019-08-14 16:12:14.712<--1:info depth 22 currmove d7d5 currmovenumber 2
// 2019-08-14 16:12:15.233<--1:info depth 22 currmove e7e6 currmovenumber 3
// 2019-08-14 16:12:15.277<--1:info depth 22 currmove b7b5 currmovenumber 4
// 2019-08-14 16:12:15.284<--1:info depth 22 currmove e7e5 currmovenumber 5
// 2019-08-14 16:12:15.329<--1:info depth 22 currmove c7c6 currmovenumber 6
// 2019-08-14 16:12:15.358<--1:info depth 22 currmove b8c6 currmovenumber 7
// 2019-08-14 16:12:15.370<--1:info depth 22 currmove c7c5 currmovenumber 8
// 2019-08-14 16:12:15.383<--1:info depth 22 currmove a7a6 currmovenumber 9
// 2019-08-14 16:12:15.388<--1:info depth 22 currmove d7d6 currmovenumber 10
// 2019-08-14 16:12:15.400<--1:info depth 22 currmove b8a6 currmovenumber 11
// 2019-08-14 16:12:15.410<--1:info depth 22 currmove g7g6 currmovenumber 12
// 2019-08-14 16:12:15.414<--1:info depth 22 currmove f7f6 currmovenumber 13
// 2019-08-14 16:12:15.420<--1:info depth 22 currmove a7a5 currmovenumber 14
// 2019-08-14 16:12:15.425<--1:info depth 22 currmove h7h6 currmovenumber 15
// 2019-08-14 16:12:15.431<--1:info depth 22 currmove f7f5 currmovenumber 16
// 2019-08-14 16:12:15.443<--1:info depth 22 currmove g7g5 currmovenumber 17
// 2019-08-14 16:12:15.446<--1:info depth 22 currmove b7b6 currmovenumber 18
// 2019-08-14 16:12:15.449<--1:info depth 22 currmove h7h5 currmovenumber 19
// 2019-08-14 16:12:15.454<--1:info depth 22 currmove g8h6 currmovenumber 20
// 2019-08-14 16:12:15.456<--1:info depth 22 seldepth 28 multipv 1 score cp -30 upperbound nodes 8045479 nps 1598863 hashfull 999 tbhits 0 time 5032 pv g8f6 c2c4
// 2019-08-14 16:12:15.473<--1:info depth 22 currmove g8f6 currmovenumber 1
// 2019-08-14 16:12:15.742<--1:info depth 22 currmove d7d5 currmovenumber 2
// 2019-08-14 16:12:15.840<--1:info depth 22 currmove e7e6 currmovenumber 3
// 2019-08-14 16:12:15.872<--1:info depth 22 currmove b8c6 currmovenumber 4
// 2019-08-14 16:12:15.895<--1:info depth 22 currmove c7c6 currmovenumber 5
// 2019-08-14 16:12:15.932<--1:info depth 22 currmove e7e5 currmovenumber 6
// 2019-08-14 16:12:15.946<--1:info depth 22 currmove c7c5 currmovenumber 7
// 2019-08-14 16:12:15.967<--1:info depth 22 currmove b7b5 currmovenumber 8
// 2019-08-14 16:12:15.972<--1:info depth 22 currmove a7a6 currmovenumber 9
// 2019-08-14 16:12:15.992<--1:info depth 22 currmove f7f6 currmovenumber 10
// 2019-08-14 16:12:15.995<--1:info depth 22 currmove d7d6 currmovenumber 11
// 2019-08-14 16:12:16.037<--1:info depth 22 currmove a7a5 currmovenumber 12
// 2019-08-14 16:12:16.044<--1:info depth 22 currmove g7g6 currmovenumber 13
// 2019-08-14 16:12:16.056<--1:info depth 22 currmove g7g5 currmovenumber 14
// 2019-08-14 16:12:16.061<--1:info depth 22 currmove h7h6 currmovenumber 15
// 2019-08-14 16:12:16.068<--1:info depth 22 currmove b7b6 currmovenumber 16
// 2019-08-14 16:12:16.077<--1:info depth 22 currmove f7f5 currmovenumber 17
// 2019-08-14 16:12:16.080<--1:info depth 22 currmove b8a6 currmovenumber 18
// 2019-08-14 16:12:16.083<--1:info depth 22 currmove h7h5 currmovenumber 19
// 2019-08-14 16:12:16.086<--1:info depth 22 currmove g8h6 currmovenumber 20
// 2019-08-14 16:12:16.089<--1:info depth 22 seldepth 28 multipv 1 score cp -37 upperbound nodes 8840865 nps 1560611 hashfull 999 tbhits 0 time 5665 pv g8f6 c2c4
// 2019-08-14 16:12:16.106<--1:info depth 22 currmove g8f6 currmovenumber 1
// 2019-08-14 16:12:18.009<--1:info depth 22 currmove d7d5 currmovenumber 2
// 2019-08-14 16:12:18.021<--1:info depth 22 currmove h7h6 currmovenumber 3
// 2019-08-14 16:12:18.094<--1:info depth 22 currmove e7e6 currmovenumber 4
// 2019-08-14 16:12:18.100<--1:info depth 22 currmove d7d6 currmovenumber 5
// 2019-08-14 16:12:18.181<--1:info depth 22 currmove b8c6 currmovenumber 6
// 2019-08-14 16:12:18.187<--1:info depth 22 currmove e7e5 currmovenumber 7
// 2019-08-14 16:12:18.189<--1:info depth 22 currmove c7c5 currmovenumber 8
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove g7g5 currmovenumber 9
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove g7g6 currmovenumber 10
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove a7a5 currmovenumber 11
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove f7f6 currmovenumber 12
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove c7c6 currmovenumber 13
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove b7b6 currmovenumber 14
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove b7b5 currmovenumber 15
// 2019-08-14 16:12:18.197<--1:info depth 22 currmove f7f5 currmovenumber 16
// 2019-08-14 16:12:18.207<--1:info depth 22 currmove a7a6 currmovenumber 17
// 2019-08-14 16:12:18.212<--1:info depth 22 currmove b8a6 currmovenumber 18
// 2019-08-14 16:12:18.215<--1:info depth 22 currmove h7h5 currmovenumber 19
// 2019-08-14 16:12:18.215<--1:info depth 22 currmove g8h6 currmovenumber 20
// 2019-08-14 16:12:18.215<--1:info depth 22 seldepth 33 multipv 1 score cp -34 nodes 12953759 nps 1662656 hashfull 999 tbhits 0 time 7791 pv g8f6 c2c4 e7e6 b1c3 d7d5 g1f3 f8e7 c1f4 e8g8 e2e3 b7b6 f1d3 d5c4 d3c4 c8b7 e1g1 c7c5 c3b5 b8c6 d4c5 c6a5 c4e2 e7c5 f4g3 d8d1 a1d1
// 2019-08-14 16:12:18.215<--1:bestmove g8f6 ponder c2c4
// 2019-08-14 16:12:18.215*1*Found move:Ng8-f6

int main(int argc, char** argv)
{
    const char* const startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Position position; // TODO: move to separate thread
    std::cout << engine_info() << std::endl;
    std::string line, token;
    while (std::getline(std::cin, line)) {
        std::stringstream ss{line};
        ss >> token;
        if (token == "uci") {
            // * uci
            // 	tell engine to use the uci (universal chess interface),
            // 	this will be sent once as a first command after program boot
            // 	to tell the engine to switch to uci mode.
            // 	After receiving the uci command the engine must identify itself with the "id" command
            // 	and send the "option" commands to tell the GUI which engine settings the engine supports if any.
            // 	After that the engine should send "uciok" to acknowledge the uci mode.
            // 	If no uciok is sent within a certain time period, the engine task will be killed by the GUI.

            std::cout << "id name LessChess" << VERSION << std::endl;
            std::cout << "id author Peter Lesslie" << std::endl;
            // TODO: implement sending options
            std::cout << "uciok" << std::endl;
        } else if (token == "debug") {
            // * debug [ on | off ]
            // 	switch the debug mode of the engine on and off.
            // 	In debug mode the engine should send additional infos to the GUI, e.g. with the "info string" command,
            // 	to help debugging, e.g. the commands that the engine has received etc.
            // 	This mode should be switched off by default and this command can be sent
            // 	any time, also when the engine is thinking.
            ss >> token;
            if (token == "on") {
                // TODO: turn debug on
            } else if (token == "off") {
                // TODO: turn debug off
            } else {
                std::cerr << "Unknown option to debug command: '" << token << "'" << std::endl;
            }
        } else if (token == "isready") {
            // * isready
            // 	this is used to synchronize the engine with the GUI. When the GUI has sent a command or
            // 	multiple commands that can take some time to complete,
            // 	this command can be used to wait for the engine to be ready again or
            // 	to ping the engine to find out if it is still alive.
            // 	E.g. this should be sent after setting the path to the tablebases as this can take some time.
            // 	This command is also required once before the engine is asked to do any search
            // 	to wait for the engine to finish initializing.
            // 	This command must always be answered with "readyok" and can be sent also when the engine is calculating
            // 	in which case the engine should also immediately answer with "readyok" without stopping the search.

            // TODO: implement
            std::cout << "readyok" << std::endl;
        } else if (token == "setoption") {
            // * setoption name <id> [value <x>]
            // 	this is sent to the engine when the user wants to change the internal parameters
            // 	of the engine. For the "button" type no value is needed.
            // 	One string will be sent for each parameter and this will only be sent when the engine is waiting.
            // 	The name and value of the option in <id> should not be case sensitive and can inlude spaces.
            // 	The substrings "value" and "name" should be avoided in <id> and <x> to allow unambiguous parsing,
            // 	for example do not use <name> = "draw value".
            // 	Here are some strings for the example below:
            // 	   "setoption name Nullmove value true\n"
            //       "setoption name Selectivity value 3\n"
            // 	   "setoption name Style value Risky\n"
            // 	   "setoption name Clear Hash\n"
            // 	   "setoption name NalimovPath value c:\chess\tb\4;c:\chess\tb\5\n"

            // TODO: implement
        } else if (token == "register") {
            // * register
            // 	this is the command to try to register an engine or to tell the engine that registration
            // 	will be done later. This command should always be sent if the engine	has sent "registration error"
            // 	at program startup.
            // 	The following tokens are allowed:
            // 	* later
            // 	   the user doesn't want to register the engine now.
            // 	* name <x>
            // 	   the engine should be registered with the name <x>
            // 	* code <y>
            // 	   the engine should be registered with the code <y>
            // 	Example:
            // 	   "register later"
            // 	   "register name Stefan MK code 4359874324"

            // ignore
        } else if (token == "ucinewgame") {
            // * ucinewgame
            //    this is sent to the engine when the next search (started with "position" and "go") will be from
            //    a different game. This can be a new game the engine should play or a new game it should analyse but
            //    also the next position from a testsuite with positions only.
            //    If the GUI hasn't sent a "ucinewgame" before the first "position" command, the engine shouldn't
            //    expect any further ucinewgame commands as the GUI is probably not supporting the ucinewgame command.
            //    So the engine should not rely on this command even though all new GUIs should support it.
            //    As the engine's reaction to "ucinewgame" can take some time the GUI should always send "isready"
            //    after "ucinewgame" to wait for the engine to finish its operation.

            // TODO: implement
        } else if (token == "position") {
            // * position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
            // 	set up the position described in fenstring on the internal board and
            // 	play the moves on the internal chess board.
            // 	if the game was played  from the start position the string "startpos" will be sent
            // 	Note: no "new" command is needed. However, if this position is from a different game than
            // 	the last position sent to the engine, the GUI should have sent a "ucinewgame" inbetween.

            // TODO: implement
            ss >> token;
            if (token == "startpos") {
                position = Position::from_fen(startpos);
            } else if (token == "fen") {
                ss >> token;
                position = Position::from_fen(token);
            } else {
                std::cerr << "invalid 'position' command: expected FEN" << std::endl;
                continue;
            }

            if (ss >> token) {
                if (token != "moves") {
                    std::cerr << "invalid 'position' command: expected 'moves'" << std::endl;
                }
                while (ss >> token) {
                    try {
                        Move move = position.move_from_long_algebraic(token);
                    } catch (const std::exception& ex) {
                        std::cerr << "invalid move: " << ex.what() << std::endl;
                    }
                }
            }
        } else if (token == "go") {
            // * go
            // 	start calculating on the current position set up with the "position" command.
            // 	There are a number of commands that can follow this command, all will be sent in the same string.
            // 	If one command is not sent its value should be interpreted as it would not influence the search.
            // 	* searchmoves <move1> .... <movei>
            // 		restrict search to this moves only
            // 		Example: After "position startpos" and "go infinite searchmoves e2e4 d2d4"
            // 		the engine should only search the two moves e2e4 and d2d4 in the initial position.
            // 	* ponder
            // 		start searching in pondering mode.
            // 		Do not exit the search in ponder mode, even if it's mate!
            // 		This means that the last move sent in in the position string is the ponder move.
            // 		The engine can do what it wants to do, but after a "ponderhit" command
            // 		it should execute the suggested move to ponder on. This means that the ponder move sent by
            // 		the GUI can be interpreted as a recommendation about which move to ponder. However, if the
            // 		engine decides to ponder on a different move, it should not display any mainlines as they are
            // 		likely to be misinterpreted by the GUI because the GUI expects the engine to ponder
            // 	   on the suggested move.
            // 	* wtime <x>
            // 		white has x msec left on the clock
            // 	* btime <x>
            // 		black has x msec left on the clock
            // 	* winc <x>
            // 		white increment per move in mseconds if x > 0
            // 	* binc <x>
            // 		black increment per move in mseconds if x > 0
            // 	* movestogo <x>
            //       there are x moves to the next time control,
            // 		this will only be sent if x > 0,
            // 		if you don't get this and get the wtime and btime it's sudden death
            // 	* depth <x>
            // 		search x plies only.
            // 	* nodes <x>
            // 	   search x nodes only,
            // 	* mate <x>
            // 		search for a mate in x moves
            // 	* movetime <x>
            // 		search exactly x mseconds
            // 	* infinite
            // 		search until the "stop" command. Do not exit the search without being told so in this mode!

            // TODO: implement

            // TEMP TEMP
            std::cout << "bestmove g8f6" << std::endl;
        } else if (token == "stop") {
            // * stop
            // 	stop calculating as soon as possible,
            // 	don't forget the "bestmove" and possibly the "ponder" token when finishing the search

            // TODO: implement
        } else if (token == "ponderhit") {
            // * ponderhit
            //     the user has played the expected move. This will be sent if the engine was told to ponder on the same move
            //     the user has played. The engine should continue searching but switch from pondering to normal search.

            // TODO: implement
        } else if (token == "quit") {
            break;
        } else {
            std::cerr << "Unknown command: '" << token << "'" << std::endl;
        }
    }

    return 0;
}
