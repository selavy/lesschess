#pragma once

#include "move.h"

namespace lesschess {

struct Position;

u64 perft_speed(Position& position, int depth);

} /*lesschess*/
