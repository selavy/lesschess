#ifndef POSITION__H_
#define POSITION__H_

#include <stdio.h>
#include <stdint.h>
#include "move.h"

// `nmoves'    - number of full moves, incremented after black's move
// `halfmoves' - number of halfmoves since the last capture or pawn advance (like in FEN)
//               used for 50 move rule
// `enpassant' - is the target square behind the pawn (like in FEN)
//               16    = no enpassant
//               0..7  = a3..h3
//               8..15 = a6..h6
struct position {
    uint64_t brd[10];
    uint64_t side[2];
    uint8_t  sqtopc[64];
    uint8_t  ksq[2];
    uint16_t nmoves;
    uint8_t  wtm;
    uint8_t  halfmoves;
    uint8_t  castle;
    uint8_t  enpassant;
};
#define PIECES(p, color, type) (p).brd[PIECE(color, type)]
#define KSQ(p, color) (p).ksq[color]

struct savepos {
    uint8_t halfmoves;
    uint8_t enpassant;
    uint8_t castle;
    uint8_t was_ep;
    uint8_t captured_pc; // EMPTY if no capture
};

// square x piece (8x8)x(6x2)
// white to move  (1)
// castle rights  (16)
// enpassant file (8)
#define ZOBRISTSZ ((8*8*6*2) + 1 + 16 + 8)
struct zobrist_hash {
    uint8_t hash[ZOBRISTSZ];
};
typedef struct zobrist_hash zobrist_hash;

extern size_t ZOBRIST_BOARD_INDEX(int pc, int file, int rank);
extern size_t ZOBRIST_BOARD_SQ_INDEX(int pc, int sq);
extern size_t ZOBRIST_SIDE_TO_MOVE_INDEX();
extern size_t ZOBRIST_CASTLE_RIGHTS_INDEX(uint8_t castle_flag);
extern size_t ZOBRIST_ENPASSANT_INDEX(int sq);
extern size_t ZOBRIST_ENPASSANT_FILE_INDEX(int file);
extern int zobrist_compare(const zobrist_hash *h1, const zobrist_hash *h2);

extern void zobrist_hash_module_init();
extern void zobrist_hash_init(zobrist_hash *zh);
extern void zobrist_hash_from_position(const struct position *const pos, uint64_t *hash, zobrist_hash *zh);
extern void zobrist_hash_description(FILE *fp, const zobrist_hash *zh);

extern int position_from_fen(struct position *restrict pos, const char *fen);
extern void position_print(FILE *os, const struct position *restrict const pos);
extern int validate_position(struct position *restrict const pos);
extern void make_move(struct position *restrict pos, struct savepos *restrict sp, move m);
extern void make_move_ex(struct position *restrict pos, struct savepos *restrict sp, move m, uint64_t *hash, zobrist_hash *zh);
extern void undo_move(struct position *restrict pos, const struct savepos *restrict sp, move m);
extern void undo_move_ex(struct position *restrict pos, const struct savepos *restrict sp, move m, uint64_t *hash, zobrist_hash *zh);
extern move parse_xboard_move(struct position *restrict const pos, const char *line, int len);

#endif // POSITION__H_
