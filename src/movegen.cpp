#include "movegen.h"

namespace lesschess {

// [[nodiscard]]
// bool attacks(const Position& pos, Color side, Square square) noexcept {
//     u64 pcs;
//     const Color contra = flip_color(side);
//     const u64 occupied = pos.sidemasks_[side] | pos->side[contra];
//     pcs = pos->brd[PIECE(side, ROOK)] | pos->brd[PIECE(side, QUEEN)];
//     if ((rook_attacks(square, occupied) & pcs) != 0) {
//         return 1;
//     }
//     pcs = pos->brd[PIECE(side, BISHOP)] | pos->brd[PIECE(side, QUEEN)];
//     if ((bishop_attacks(square, occupied) & pcs) != 0) {
//         return 2;
//     }
//     pcs = pos->brd[PIECE(side, KNIGHT)];
//     if ((knight_attacks(square) & pcs) != 0) {
//         return 3;
//     }
//     pcs = pos->brd[PIECE(side, PAWN)];
//     if ((pawn_attacks(contra, square) & pcs) != 0) {
//         return 4;
//     }
//     pcs = MASK(KSQ(*pos, side));
//     if ((king_attacks(square) & pcs) != 0) {
//         return 5;
//     }
//     return 0;
// }

}
