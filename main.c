#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
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

void run_perft_test(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./lesschess perft <fen> <depth>\n");
        return;
    }
    struct position pos;
    uint64_t nodes;
    uint64_t captures;
    uint64_t eps;
    uint64_t castles;
    uint64_t promos;
    uint64_t checks;
    uint64_t mates;
    int result;
    const char *fen = argv[0];
    const int depth = atoi(argv[1]);

    result = position_from_fen(&pos, fen);
    if (result != 0) {
        fprintf(stderr, "Invalid FEN\n");
        exit(EXIT_FAILURE);
    }
    result = validate_position(&pos);
    if (result != 0) {
        fprintf(stderr, "Invalid position\n");
        exit(EXIT_FAILURE);
    }
    result = perft_test(&pos, depth, &nodes, &captures, &eps, &castles,
            &promos, &checks, &mates);
    if (result != 0) {
        fprintf(stderr, "perft_test failed\n");
        exit(EXIT_FAILURE);
    }

    printf("%" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
            nodes, captures, eps, castles, promos, checks, mates);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        for (int depth = 0; depth < 7; ++depth) {
            time_test(depth);
        }
    } else {
        if (strcmp(argv[1], "perft") == 0) {
            run_perft_test(argc-2, argv+2);
        }
    }

    return 0;
}
