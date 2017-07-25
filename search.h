#ifndef SEARCH__H_
#define SEARCH__H_

#include "move.h"

struct position;
extern move search(const struct position *restrict const pos);

#endif
