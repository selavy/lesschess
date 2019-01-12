#pragma once

#include "../position.h"

// TODO(peter): move into `detail` directory

u64 generate_checkers(const Position *const pos, u8 side) noexcept;
u64 generate_attacked(const Position *const pos, const u8 side) noexcept;
int attacks(const Position *const pos, u8 side, int square) noexcept;
u64 generate_pinned(const Position *const pos, u8 side, u8 kingcolor) noexcept;
Move *generate_evasions(const Position *const pos, u64 checkers, Move *moves) noexcept;
Move *generate_non_evasions(const Position *const pos, Move *moves) noexcept;
int generate_legal_moves(const Position *const pos, Move *moves) noexcept;
int in_check(const Position *const pos, u8 side) noexcept;
int is_legal_move(const Position *const pos, move m) noexcept;
