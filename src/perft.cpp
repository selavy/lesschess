#include "perft.h"
#include "position.h"
#include <vector>

namespace lesschess {

u64 perft_speed(Position& position, int depth) {
    assert(depth >= 0);
    if (depth == 0) {
        return 1;
    }

    // XXX: consider putting on stack; or try to avoid the memory allocation
    std::vector<Move> moves(256);
    int nmoves = position.generate_legal_moves(&moves[0]);
    if (depth == 1) {
        return nmoves;
    }

    uint64_t nodes = 0;
    Savepos sp;
    for (int i = 0; i < nmoves; ++i) {
#ifndef NDEBUG
        auto orig_position = position;
#endif
        position.make_move(sp, moves[i]);
        nodes += perft_speed(position, depth - 1);
        position.undo_move(sp, moves[i]);
#ifndef NDEBUG
        assert(position == orig_position);
#endif
    }
    return nodes;
}

} /*lesschess*/
