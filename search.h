#ifndef SEARCH__H_
#define SEARCH__H_

#include "move.h"

extern void transposition_table_init();

struct position;
extern move search(struct position *pos, int *score, int *searched_depth);

#endif
