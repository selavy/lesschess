#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <time.h>
#include "magic_tables.h"
#include "move.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"

static struct timespec timediff(struct timespec start, struct timespec end) {
    struct timespec result;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = end.tv_sec - start.tv_sec - 1;
        result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        result.tv_sec = end.tv_sec - start.tv_sec;
        result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return result;
}

static void time_test(int depth) {
    uint64_t nodes = 0;
    struct timespec begin;
    struct timespec end;
    struct timespec dur;
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
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
    nodes = perft_speed(&pos, depth);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    dur = timediff(begin, end);
    printf("Depth %d, nodes = %" PRIu64 " took %ld seconds %ld millis\n",
            depth, nodes, dur.tv_sec, dur.tv_nsec / 1000000);
}

int main(int argc, char **argv) {
    for (int depth = 0; depth < 7; ++depth) {
        time_test(depth);
    }
    return 0;
}
