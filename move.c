#include "move.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
const char *sq_to_str[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
// clang-format on

const char *const visual_pcs = "NnBbRrQqPpKk ";

void move_print(FILE *fp, move mv) {
    const uint32_t to = TO(mv);
    const uint32_t from = FROM(mv);
    const uint32_t prm = PROMO_PC(mv);
    const uint32_t flags = FLAGS(mv);

    fprintf(fp, "from(%s) to(%s)", sq_to_str[from], sq_to_str[to]);
    switch (flags) {
    case FLG_EP:
        printf(" e.p.");
        break;
    case FLG_PROMO:
        printf(" promo(%c)", visual_pcs[prm]);
        break;
    case FLG_CASTLE:
        printf(" castle");
    default:
        break;
    }
    fprintf(fp, "\n");
}

void move_print_short(move m) {
    printf("%s%s", sq_to_str[FROM(m)], sq_to_str[TO(m)]);
}

const char *xboard_move_print(move m) {
    // max move length is "e7e8q", most moves are "e7e8"
    static char buffer[7];
    const char *pieces = "nbrq";
    const uint32_t from = FROM(m);
    const uint32_t flags = FLAGS(m);
    uint32_t to = TO(m);
    if (flags == FLG_CASTLE) {
        // castling to square is "capturing" the rook.
        // Xboard expects white 0-0 to be e1g1 so adjust
        // to square if necessary.
        switch (to) {
        case H1:
            to = G1;
            break;
        case A1:
            to = C1;
            break;
        case H8:
            to = G8;
            break;
        case A8:
            to = C8;
            break;
        default:
            unreachable();
        }
    }
    memset(&buffer[0], 0, sizeof(buffer));
    sprintf(&buffer[0], "%s%s", sq_to_str[from], sq_to_str[to]);
    if (flags == FLG_PROMO) {
        buffer[5] = pieces[PROMO_PC(m)];
    }
    return &buffer[0];
}

int getfile(char c) {
    if (c >= 'a' && c <= 'h') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'H') {
        return c - 'A';
    } else {
        return -1;
    }
}

int getrank(char c) {
    if (c >= '1' && c <= '8') {
        return c - '1';
    } else {
        return -1;
    }
}

int getpromopiece(char c) {
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

int getpiece(char c) {
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
    case 'k':
    case 'K':
        return KING;
    default:
        return -1;
    }
}
