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
    const char *fen = g_starting_position_fen;
    char *buffer = 0;
    size_t buflen = 0;
    struct position pos;
    const char *c;

    stream = fopen(filename, "r");
    if (stream == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    if (position_from_fen(&pos, fen) != 0) {
        fprintf(stderr, "position_from_fen failed\n");
        exit(EXIT_FAILURE);
    }
    if (validate_position(&pos) != 0) {
        fprintf(stderr, "validate_position failed\n");
        exit(EXIT_FAILURE);
    }
    // XXX: for simplicity, load entire file into a buffer
    while ((read = getline(&line, &len, stream)) != -1) {
        buffer = realloc(buffer, buflen + read + 1);
        memcpy(&buffer[buflen], line, read);
        buflen += read;
    }
    if (buflen == 1) {
        goto end;
    }
    buffer[buflen-1] = 0;
    printf("Buffer Length: %zu\n", buflen);
    printf("Buffer '%s'\n", buffer);

    const char *whitespace_pattern = "[ \n\t\r]+";
    const char *moveno_pattern = "^[0-9]+\\.";
    const char *piece_pattern = "^[KQRBNkqrbn][a-h][1-8]";
    const char *pawn_pattern  = "^[a-h][1-8]";
    const char *piece_capture_pattern = "^[KQRBNkqrbn]x[a-h][1-8]";
    const char *pawn_capture_pattern = "^[a-h]x[a-h][1-8]";
    const char *castle_kingside_pattern = "^O-O";
    const char *castle_queenside_pattern = "^O-O-O";
    const char *pawn_queen_pattern = "[a-h][1-8]=[QRBNqrbn]";

    regex_t whitespace_regex;
    regex_t moveno_regex;
    regex_t piece_regex;
    regex_t pawn_regex;
    regex_t piece_capture_regex;
    regex_t pawn_capture_regex;
    regex_t castle_kingside_regex;
    regex_t castle_queenside_regex;
    regex_t pawn_queen_regex;

    printf("Compiling regex...\n");
    if (regcomp(&whitespace_regex, whitespace_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&moveno_regex, moveno_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&piece_regex, piece_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&pawn_regex, pawn_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&piece_capture_regex, piece_capture_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&pawn_capture_regex, pawn_capture_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&castle_kingside_regex, castle_kingside_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&castle_queenside_regex, castle_queenside_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    if (regcomp(&pawn_queen_regex, pawn_queen_pattern, REG_EXTENDED) != 0) {
        perror("regcomp");
        goto end;
    }
    printf("Successfully compiled regex...\n");

    regmatch_t pmatch[1];

   int regcomp(regex_t *preg, const char *regex, int cflags);
   int regexec(const regex_t *preg, const char *string, size_t nmatch,
               regmatch_t pmatch[], int eflags);
    c = buffer;
    while (*c) {
        if (regexec(&whitespace_regex, c, 1, pmatch, 0) > 0) {
            printf("matched whitespace\n");
            goto end;
        }
        if (regexec(&moveno_regex, c, 1, pmatch, 0) > 0) {
            printf("matched moveno\n");
            goto end;
        }
        if (regexec(&piece_regex, c, 1, pmatch, 0) > 0) {
            printf("matched piece\n");
            goto end;
        }
        if (regexec(&pawn_regex, c, 1, pmatch, 0) > 0) {
            printf("matched pawn\n");
            goto end;
        }
        if (regexec(&piece_capture_regex, c, 1, pmatch, 0) > 0) {
            printf("matched piece capture\n");
            goto end;
        }
        if (regexec(&pawn_capture_regex, c, 1, pmatch, 0) > 0) {
            printf("matched pawn capture\n");
            goto end;
        }
        if (regexec(&castle_kingside_regex, c, 1, pmatch, 0) > 0) {
            printf("matched castle king\n");
            goto end;
        }
        if (regexec(&castle_queenside_regex, c, 1, pmatch, 0) > 0) {
            printf("matched whitespace\n");
            goto end;
        }
        if (regexec(&pawn_queen_regex, c, 1, pmatch, 0) > 0) {
            printf("matched pawn queen\n");
            goto end;
        }
        printf("Didn't match anything!");
        goto end;
    }

end:
    free(buffer);
    free(line);
    fclose(stream);
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
        }
    }

    return 0;
}
