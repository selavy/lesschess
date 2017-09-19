#ifndef MOVEGEN__H_
#define MOVEGEN__H_

#include "move.h"
#include "position.h"
#include <stdint.h>

// Move Generation code heavily inspired/taken from Stockfish

extern int is_legal(const struct position *const restrict pos, uint64_t pinned,
                    move m);
extern uint64_t generate_checkers(const struct position *const restrict pos,
                                  uint8_t side);
extern uint64_t generate_attacked(const struct position *const restrict pos,
                                  const uint8_t side);
extern int attacks(const struct position *const restrict pos, uint8_t side,
                   int square);
extern uint64_t generate_pinned(const struct position *const restrict pos,
                                uint8_t side, uint8_t kingcolor);
extern move *generate_evasions(const struct position *const restrict pos,
                               uint64_t checkers, move *restrict moves);
extern move *generate_non_evasions(const struct position *const restrict pos,
                                   move *restrict moves);
extern int generate_legal_moves(const struct position *const restrict pos,
                                move *restrict moves);
extern int in_check(const struct position *const restrict pos, uint8_t side);
extern int is_legal_move_FIXME(const struct position *restrict const pos,
                               move m);
extern int is_mate_FIXME(const struct position *restrict const pos);

#endif // MOVEGEN__H_
