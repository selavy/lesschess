#include "xboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include "move.h"
#include "position.h"
#include "movegen.h"

enum xboard_state {
    XB_NONE,
    XB_BEGIN,
    XB_FEATURES_SENT,
    XB_SETUP,
};

static int g_xbstate = XB_NONE;
static struct position g_pos;
static FILE *g_debug_stream;

static void xbstate_init();
static void xberror(const char *fmt, ...) __attribute__((noreturn));
static void xbdebug(const char *fmt, ...);
static void xbwrite(const char *fmt, ...);
static void handle_input(const char *line, int len);
static void handle_begin_state(const char *line, int len);
static void handle_features_sent(const char *line, int len);
static void handle_setup(const char *line, int len);
static int getfile(char c);

static int strictcmp(const char *a, const char *b) {
    return strncmp(a, b, strlen(b)) == 0;
}

static void sigh(int nsig) {
    xbdebug("received signal: %d", nsig);
}


static int getfile(char c) {
    if (c >= 'a' && c <= 'h') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'H') {
        return c - 'A';
    } else {
        return -1;
    }
}

static int getrank(char c) {
    if (c >= '1' && c <= '8') {
        return c - '1';
    } else {
        return -1;
    }
}

static int getpromopiece(char c) {
    switch (c) {
        case 'n':
        case 'N':
            return KNIGHT;
        case 'b':
        case 'B':
            return BISHOP;
        case 'r':
        case 'R':
            return ROOK;
        case 'q':
        case 'Q':
            return QUEEN;
        default:
            return -1;
    }
}

void handle_input(const char *line, int len) {
    if (strictcmp(line, "quit")) {
        xbdebug("Bye.\n");
        exit(EXIT_SUCCESS);
    } else {
        switch (g_xbstate) {
            case XB_BEGIN:
                handle_begin_state(line, len);
                break;
            case XB_FEATURES_SENT:
                handle_features_sent(line, len);
                break;
            case XB_SETUP:
                handle_setup(line, len);
                break;
            default:
                xberror("unknown xboard state.");
        }
    }
}

void handle_begin_state(const char *line, int len) {
    if (strictcmp(line, "protover")) {
        if (len < 10 || line[9] != '2') {
            xberror("Unknown protocol version: %c", len < 10 ? '?' : line[9]);
        }
        xbwrite("myname=\"lesschess\"");
        xbwrite("feature reuse=0");
        xbwrite("feature analyze=0");
        xbwrite("feature time=0");
        xbwrite("feature done=1");
        g_xbstate = XB_FEATURES_SENT;
    } else {
        xberror("unrecognized command: '%.*s'", len, line);
    }
}

void handle_features_sent(const char *line, int len) {
    if (strictcmp(line, "accepted")) {
        // TODO: keep bitfield of features that have been accepted.
        // verify that everything requested is accepted
    } else if (strictcmp(line, "rejected")) {
        xberror("features rejected");
    } else if (strictcmp(line, "new")) {
        xbstate_init();
        g_xbstate = XB_SETUP;
    } else {
        xberror("unrecognized command: '%.*s'", len, line);
    }
}

static move parse_xboard_move(const char *line, int len)
{
    if (len < 4 || len > 5) {
        return INVALID_MOVE;
    }
    const int fromrank = getrank(line[0]);
    const int fromfile = getfile(line[1]);
    const int torank = getrank(line[2]);
    const int tofile = getfile(line[3]);

    if (fromrank == -1 || fromfile == -1 || torank == -1 || tofile == -1) {
        return INVALID_MOVE;
    }
    const int from = SQUARE(fromfile, fromrank);
    const int to = SQUARE(tofile, torank);
    if (len == 4) {
        if (from == E1 && KSQ(g_pos, WHITE) == E1 && (to == C1 || to == G1)) {
            return CASTLE(from, to);
        } else if (from == E8 && KSQ(g_pos, BLACK) == E8 && (to == C8 || to == G8)) {
            return CASTLE(from, to);
        } else if (to == g_pos.enpassant && g_pos.sqtopc[from] == PIECE(g_pos.wtm, PAWN)) {
            return EP_CAPTURE(from, to);
        } else {
            return MOVE(from, to);
        }
    } else if (len == 5) {
        const int promopc = getpromopiece(line[4]);
        return PROMOTION(from, to, promopc);
    } else {
        unreachable();
        return INVALID_MOVE;
    }

}

void handle_setup(const char *line, int len) {
    if (strictcmp(line, "random")) {
        // nop
    } else if (strictcmp(line, "hard")) {
        // nop
    } else if (strictcmp(line, "easy")) {
        // nop
    } else if (strictcmp(line, "post")) {
        // nop
        // TODO: turn on thinking/pondering output
    } else if (strictcmp(line, "level")) {
        // nop
        // XXX: parse level
    } else {
        const move m = parse_xboard_move(line, len);
        if (m == INVALID_MOVE) {
            xberror("unrecognized command: '%.*s'", len, line);
            //xberror("unrecognized command: '%s'", line);
        } else {
            xbdebug("received move!!! %s", xboard_move_print(m));
            exit(EXIT_SUCCESS);
        }
    }
}

void xberror(const char *fmt, ...) {
    fprintf(stderr, "LessChess ERROR: ");
    fprintf(g_debug_stream, "LessChess ERROR: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    vfprintf(g_debug_stream, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fprintf(g_debug_stream, "\n");
    exit(EXIT_FAILURE);
}

void xbdebug(const char *fmt, ...) {
    fprintf(g_debug_stream, "LessChess INFO: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(g_debug_stream, fmt, args);
    va_end(args);
    fprintf(g_debug_stream, "\n");
}

void xbwrite(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void xbstate_init() {
    int retcode;
    const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    retcode = position_from_fen(&g_pos, fen);
    if (retcode != 0) {
        xberror("position_from_fen failed: %d", retcode);
    }
    retcode = validate_position(&g_pos);
    if (retcode != 0) {
        fprintf(stderr, "validate_position failed: %d", retcode);
        exit(EXIT_FAILURE);
    }
}

void xboard_uci_main() {
    FILE *istream;
    char *line = 0;
    size_t len = 0;
    ssize_t read;

    g_debug_stream = fopen("/tmp/debug_stream.txt", "w");
    if (!g_debug_stream) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    setbuf(g_debug_stream, 0);

    signal(SIGINT, &sigh);

    xbdebug("Beginning xboard_uci_main()...\n");

    istream = fdopen(STDIN_FILENO, "rb");
    if (!istream) {
        perror("fdopen");
        exit(EXIT_FAILURE);
    }
    setbuf(stdout, 0);
    setbuf(istream, 0);

    while ((read = getline(&line, &len, istream)) > 0) {
        xbdebug("received line: '%.*s'", (int)read - 1, line);
        if (g_xbstate == XB_NONE) {
            if (strictcmp(line, "xboard")) {
                g_xbstate = XB_BEGIN;
                xbdebug("transitioned to XB_BEGIN");
            } else {
                xberror("unrecognized command: '%.*s'", len, line);
            }
        } else {
            handle_input(line, ((int)read) - 1);
        }
    }
}

