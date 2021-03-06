#ifndef POSITION__H_
#define POSITION__H_

#include "move.h"
#include <stdint.h>
#include <stdio.h>

// `nmoves'    - number of full moves, incremented after black's move
// `halfmoves' - number of halfmoves since the last capture or pawn advance
// (like in FEN)
//               used for 50 move rule
// `enpassant' - is the target square behind the pawn (like in FEN)
//               16    = no enpassant
//               0..7  = a3..h3
//               8..15 = a6..h6
struct position {
    uint64_t brd[10];
    uint64_t side[2];
    uint8_t sqtopc[64];
    uint8_t ksq[2];
    uint16_t nmoves;
    uint8_t wtm;
    uint8_t halfmoves;
    uint8_t castle;
    uint8_t enpassant;
};
#define PIECES(p, color, type) (p).brd[PIECE(color, type)]
#define KSQ(p, color) (p).ksq[color]

struct savepos {
    uint8_t halfmoves;
    uint8_t enpassant;
    uint8_t castle;
    uint8_t captured_pc; // EMPTY if no capture
};

extern void zobrist_hash_module_init();
extern uint64_t zobrist_hash_from_position(const struct position *pos);
extern int position_from_fen(struct position *pos, const char *fen);
extern void position_to_fen(const struct position *pos);
extern void position_print(FILE *os, const struct position *pos);
extern int validate_position(struct position *pos);
extern uint64_t make_move(struct position *pos, struct savepos *sp, move m, uint64_t hash);
extern void undo_move(struct position *pos, const struct savepos *sp, move m);
extern move parse_xboard_move(struct position *pos, const char *line, int len);

#endif // POSITION__H_
