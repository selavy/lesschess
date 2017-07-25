#include "textgui.h"
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "position.h"
#include "movegen.h"

static int getcol(char c) {
    if (c >= 'a' && c <= 'h') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'H') {
        return c - 'A';
    } else {
        return -1;
    }
}

static int getrow(char c) {
    if (c >= '1' && c <= '8') {
        return c - '1';
    } else {
        return -1;
    }
}

#define EEOF 1
#define EBADINPUT 2
static int get_move_helper(const struct position *restrict const pos, move *m) {
    char *line = readline("> ");
    int result = -EBADINPUT;

    if (!line) {
        return -EEOF;
    }

    const size_t len = strlen(line);
    if (len == 4 || len == 5) {
        const int fromfile = getcol(line[0]);
        const int fromrank = getrow(line[1]);
        const int tofile = getcol(line[2]);
        const int torank = getrow(line[3]);
        if (fromfile != -1 && fromrank != -1 && tofile != -1 && torank != -1) {
            const int fromsq = SQUARE(fromfile, fromrank);
            const int tosq = SQUARE(tofile, torank);
            if (len == 5) {
                switch (line[4]) {
                    case 'n':
                    case 'N':
                        *m = PROMOTION(fromsq, tosq, KNIGHT);
                        break;
                    case 'b':
                    case 'B':
                        *m = PROMOTION(fromsq, tosq, BISHOP);
                        break;
                    case 'r':
                    case 'R':
                        *m = PROMOTION(fromsq, tosq, ROOK);
                        break;
                    case 'q':
                    case 'Q':
                        *m = PROMOTION(fromsq, tosq, QUEEN);
                        break;
                    default:
                        goto error;
                }
            } else if ((fromsq == pos->ksq[pos->wtm]) &&
                    ((fromsq == E1 && (tosq == G1 || tosq == C1)) ||
                     (fromsq == E8 && (tosq == G8 || tosq == C8)) )) {
                *m = CASTLE(fromsq, tosq);
            } else {
                *m = MOVE(fromsq, tosq);
            }
            result = 0;
        }
    }

error:
    free(line);
    return result;
}

void get_move(const struct position *restrict const pos, move *m) {
    int result;
    position_print(stdout, pos);
    while (1) {
        result = get_move_helper(pos, m);
        if (result == -EEOF) {
            printf("Bye.\n");
            exit(EXIT_FAILURE);
        } else if (result < 0) {
            printf("Invalid input.\n");
        } else if (!(result < 0)) {
            if (is_legal_move_FIXME(pos, *m)) {
                break;
            } else {
                printf("Illegal move.\n");
            }
        }
    }
}

#undef EEOF
#undef EBADINPUT
