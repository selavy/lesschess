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
static int get_move_helper(move *m) {
    char *line = readline("> ");
    int result = -EBADINPUT;

    if (!line) {
        result = -EEOF;
    } else if (strlen(line) == 4) {
        const int fromcol = getcol(line[0]);
        const int fromrow = getrow(line[1]);
        const int tocol = getcol(line[2]);
        const int torow = getrow(line[3]);
        if (fromcol != -1 && fromrow != -1 && tocol != -1 && torow != -1) {
            *m = MOVE(fromrow*8 + fromcol, torow*8 + tocol);
            result = 0;
        }
    }

    free(line);
    return result;
}

void get_move(const struct position *restrict const pos, move *m) {
    int result;
    position_print(stdout, pos);
    while (1) {
        result = get_move_helper(m);
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
