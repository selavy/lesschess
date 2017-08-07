#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <regex.h>
#include "magic_tables.h"
#include "move.h"
#include "position.h"
#include "movegen.h"
#include "perft.h"
#include "textgui.h"
#include "search.h"
#include "xboard.h"

const char *const g_starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    const char *fen = g_starting_position_fen;
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

static void run_text_gui() {
    int result;
    move m;
    struct position pos;
    struct savepos sp;

    result = position_from_fen(&pos, g_starting_position_fen);
    if (result != 0) {
        fprintf(stderr, "position_from_fen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 100; ++i) {
        if (is_mate_FIXME(&pos)) {
            printf("%s is mated.\n", pos.wtm==WHITE? "WHITE":"BLACK");
            break;
        }
        get_move(&pos, &m);
        make_move(&pos, &sp, m);
        position_print(stdout, &pos);
        m = search(&pos);
        if (m == MATED) {
            printf("Game Over! Computer Lost\n");
            break;
        }
        printf("Computer move: %s\n", xboard_move_print(m));
        make_move(&pos, &sp, m);
    }
}

static void run_perft_test(int argc, char **argv) {
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
        fprintf(stderr, "Invalid FEN: %d\n", result);
        exit(EXIT_FAILURE);
    }
    result = validate_position(&pos);
    if (result != 0) {
        fprintf(stderr, "Invalid position: %d\n", result);
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

static void replay_file(const char *filename) {
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    move m;
    struct position pos;
    struct savepos sp;

    stream = fopen(filename, "r");
    if (stream == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    if (position_from_fen(&pos, g_starting_position_fen) != 0) {
        fprintf(stderr, "position_from_fen failed\n");
        exit(EXIT_FAILURE);
    }
    if (validate_position(&pos) != 0) {
        fprintf(stderr, "validate_position failed\n");
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, stream)) != -1) {
        m = parse_xboard_move(&pos, line, (int)read - 1);
        if (m == INVALID_MOVE) {
            fprintf(stderr, "Invalid move! '%.*s'\n", (int)read - 1, line);
            exit(EXIT_FAILURE);
        }
        make_move(&pos, &sp, m);
        if (validate_position(&pos) != 0) {
            fprintf(stderr, "validate_position failed\n");
            exit(EXIT_FAILURE);
        }
        position_print(stdout, &pos);
    }
    printf("PGN Parser Info: finished parsing file\n");
    free(line);
    fclose(stream);

    m = search(&pos);
    if (m == INVALID_MOVE) {
        printf("Search returned invalid move!\n");
    } else if (m == MATED) {
        printf("Search returned MATED\n");
    } else {
        printf("Search result: %s\n", xboard_move_print(m));
    }
}

static void zobrist_test() {
    int result;
    struct position pos;
    struct savepos sp;
    move m;

    result = position_from_fen(&pos, g_starting_position_fen);
    if (result != 0) {
        fprintf(stderr, "position_from_fen");
        exit(EXIT_FAILURE);
    }

    zobrist_hash zh;
    zobrist_hash_module_init();
    zobrist_hash_init(&zh);
    zobrist_hash_from_position(&pos, &zh);
    zobrist_hash_description(stdout, &zh);

    m = MOVE(E2, E4);
    make_move(&pos, &sp, m);
    zobrist_hash_from_position(&pos, &zh);
    zobrist_hash_description(stdout, &zh);
}


int main(int argc, char **argv) {
    if (argc < 2) {
       xboard_uci_main();
    } else {
        if (strcmp(argv[1], "perft") == 0) {
            run_perft_test(argc-2, argv+2);
        } else if (strcmp(argv[1], "textgui") == 0) {
            run_text_gui();
        } else if (strcmp(argv[1], "timetest") == 0) {
            for (int depth = 0; depth < 8; ++depth) {
                time_test(depth);
            }
        } else if (strcmp(argv[1], "replay") == 0) {
            if (argc == 3) {
                replay_file(argv[2]);
            } else {
                fprintf(stderr, "Usage: ./lesschess replay <FILE>\n");
            }
        } else if (strcmp(argv[1], "zobrist") == 0) {
            zobrist_test();
        }
    }

    return 0;
}
