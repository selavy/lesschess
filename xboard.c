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
#include "search.h"

enum xboard_state {
    XB_NONE,
    XB_BEGIN,
    XB_FEATURES_SENT,
    XB_SETUP,
    XB_PLAYING,
};

static int g_xbstate = XB_NONE;
static FILE *g_debug_stream;
static struct position g_pos;
static struct savepos g_sp;

static const char *statestr()
{
    switch (g_xbstate) {
        case XB_NONE:
            return "None";
        case XB_BEGIN:
            return "Begin";
        case XB_FEATURES_SENT:
            return "FeaturesSent";
        case XB_SETUP:
            return "Setup";
        case XB_PLAYING:
            return "Playing";
        default:
            return "Unknown";
    }
}

static void xbstate_init();
static void xberror(const char *fmt, ...) __attribute__((noreturn));
static void xbdebug(const char *fmt, ...);
static void xbwrite(const char *fmt, ...);
static void handle_input(const char *line, int len);
static void handle_begin_state(const char *line, int len);
static void handle_features_sent(const char *line, int len);
static void handle_setup(const char *line, int len);
static void handle_playing(const char *line, int len);

static int strictcmp(const char *a, const char *b)
{
    return strncmp(a, b, strlen(b)) == 0;
}

static void sigh(int nsig)
{
    xbdebug("received signal: %d", nsig);
}


void handle_input(const char *line, int len)
{
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
            case XB_PLAYING:
                return handle_playing(line, len);
            default:
                xberror("unknown xboard state.");
        }
    }
}

void handle_begin_state(const char *line, int len) 
{
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

void handle_features_sent(const char *line, int len)
{
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

void handle_setup(const char *line, int len)
{
    xbdebug("handle_setup: %.*s", len, line);
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
        const move m = parse_xboard_move(&g_pos, line, len);
        if (m == INVALID_MOVE) {
            xberror("unrecognized command: '%.*s'", len, line);
            //xberror("unrecognized command: '%s'", line);
        } else {
            xbdebug("received move!!! %s", xboard_move_print(m));
            //exit(EXIT_SUCCESS);
            g_xbstate = XB_PLAYING;
            handle_playing(line, len);
        }
    }
    xbdebug("hit end of handle_setup");
}

void handle_playing(const char *line, int len)
{
    move m = parse_xboard_move(&g_pos, line, len);
    if (m == INVALID_MOVE) {
        xbwrite("Illegal move (invalid rank or file): %.*s", len, line);
        return;
    }
    if (!is_legal_move_FIXME(&g_pos, m)) {
        xbwrite("Illegal move: %.*s", len, line);
        return;
    }
    make_move(&g_pos, &g_sp, m);
    m = search(&g_pos);
    if (m == MATED) {
        xbdebug("lesschess has been mated\n");
    }
    make_move(&g_pos, &g_sp, m);
    xbwrite("move %s\n", xboard_move_print(m));
}

void xberror(const char *fmt, ...)
{
    fprintf(g_debug_stream, "LessChess ERROR(%s): ", statestr());
    va_list args;
    va_start(args, fmt);
    vfprintf(g_debug_stream, fmt, args);
    va_end(args);
    fprintf(g_debug_stream, "\n");
    fflush(g_debug_stream);
    exit(EXIT_FAILURE);
}

void xbdebug(const char *fmt, ...)
{
    fprintf(g_debug_stream, "LessChess INFO(%s): ", statestr());
    va_list args;
    va_start(args, fmt);
    vfprintf(g_debug_stream, fmt, args);
    va_end(args);
    fprintf(g_debug_stream, "\n");
}

void xbwrite(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void xbstate_init()
{
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

void xboard_uci_main()
{
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

