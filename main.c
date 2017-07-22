#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include "magic_tables.h"
#include "move.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"

static void time_test(int depth) {
    uint64_t nodes = 0;
    struct position pos;
    const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (position_from_fen(&pos, fen) != 0) {
        fprintf(stderr, "position_from_fen failed\n");
        exit(EXIT_FAILURE);
    }
    if (validate_position(&pos) != 0) {
        fprintf(stderr, "validate_position failed\n");
        exit(EXIT_FAILURE);
    }
    nodes = perft_speed(&pos, depth);
    printf("Depth %d, nodes = %" PRIu64 "\n", depth, nodes);
}

int main(int argc, char **argv) {
    for (int depth = 0; depth < 7; ++depth) {
        time_test(depth);
    }
    return 0;
}
