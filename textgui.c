#include "textgui.h"
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

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

int get_move(move *m) {
    char *line = readline("> ");
    int result = 1;
    if (strlen(line) == 4) {
        const int fromcol = getcol(line[0]);
        const int fromrow = getrow(line[1]);
        const int tocol = getcol(line[2]);
        const int torow = getrow(line[3]);
        if (fromcol != -1 && fromrow != -1 && tocol != -1 && torow != -1) {
            *m = MOVE(fromrow*8 + fromcol, torow*8 + tocol);
            result = 0;
        }
    } else {
        fprintf(stderr, "Line too long: '%s'\n", line);
    }
    free(line);
    return result;
}
