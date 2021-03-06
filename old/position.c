#include "position.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

// square x piece (8x8)x(6x2)
// white to move  (1)
// castle rights  (16)
// enpassant file (8)
#define ZOBRISTSZ ((8 * 8 * 6 * 2) + 1 + 16 + 8)
static uint64_t zobrist_values[ZOBRISTSZ];

static size_t ZOBRIST_BOARD_SQ_INDEX(int pc, int sq) {
    const size_t npieces = 12;
    const size_t index = npieces * sq + pc;
    assert(pc >= 0 && pc <= npieces);
    assert(index >= 0 && index < 64 * 12);
    return index;
}

static uint64_t ZOBRIST_BOARD_SQ(int pc, int sq) {
    return zobrist_values[ZOBRIST_BOARD_SQ_INDEX(pc, sq)];
}

static size_t ZOBRIST_BOARD_INDEX(int pc, int file, int rank) {
    assert(file >= 0 && file <= 7);
    assert(rank >= 0 && rank <= 7);
    const int sq = SQUARE(file, rank);
    return ZOBRIST_BOARD_SQ_INDEX(pc, sq);
}

static uint64_t ZOBRIST_BOARD(int pc, int file, int rank) {
    return zobrist_values[ZOBRIST_BOARD_INDEX(pc, file, rank)];
}

static size_t ZOBRIST_SIDE_TO_MOVE_INDEX() {
    const size_t base = 64 * 12;
    const size_t off = 0;
    const size_t index = base + off;
    assert(index >= 64 * 12);
    assert(index <= 64 * 12);
    return index;
}

static uint64_t ZOBRIST_SIDE_TO_MOVE() {
    return zobrist_values[ZOBRIST_SIDE_TO_MOVE_INDEX()];
}

static size_t ZOBRIST_CASTLE_RIGHTS_INDEX(uint8_t castle_flag) {
    const size_t base = 64 * 12 + 1;
    const size_t off = castle_flag;
    assert(castle_flag == CSL_WQSIDE || castle_flag == CSL_WKSIDE || castle_flag == CSL_BQSIDE ||
           castle_flag == CSL_BKSIDE);
    const size_t index = base + off;
    assert(index >= 64 * 12 + 1);
    assert(index < 64 * 12 + 1 + 16);
    return index;
}

static size_t ZOBRIST_ENPASSANT_FILE_INDEX(int file) {
    const size_t base = 64 * 12 + 1 + 16;
    const size_t off = file;
    const size_t index = base + off;
    assert(index >= 64 * 12 + 1 + 16);
    assert(index < 64 * 12 + 1 + 16 + 8);
    return index;
}

static uint64_t ZOBRIST_CASTLE_RIGHTS(uint8_t castle_flag) {
    return zobrist_values[ZOBRIST_CASTLE_RIGHTS_INDEX(castle_flag)];
}

static size_t ZOBRIST_ENPASSANT_INDEX(int sq) {
    const size_t file = sq % 8;
    return ZOBRIST_ENPASSANT_FILE_INDEX(file);
}

static uint64_t ZOBRIST_ENPASSANT(int sq) {
    return zobrist_values[ZOBRIST_ENPASSANT_INDEX(sq)];
}

void zobrist_hash_module_init() {
    srand(42);
    for (int i = 0; i < ZOBRISTSZ; ++i) {
        zobrist_values[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
#ifndef NDEBUG
    for (int i = 0; i < ZOBRISTSZ; ++i) {
        for (int j = i + 1; j < ZOBRISTSZ; ++j) {
            if (zobrist_values[i] == zobrist_values[j]) {
                fprintf(stderr, "BAD %d %d\n", i, j);
                exit(1);
            }
        }
    }
#endif
}

uint64_t zobrist_hash_from_position(const struct position *pos) {
    uint64_t h = 0;
    int sq;
    int pc;
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            sq = SQUARE(file, rank);
            pc = pos->sqtopc[sq];
            if (pc != EMPTY) {
                h ^= ZOBRIST_BOARD(pc, file, rank);
            }
        }
    }
    if (pos->wtm == WHITE) {
        h ^= ZOBRIST_SIDE_TO_MOVE();
    }
    if ((pos->castle & CSL_WKSIDE) != 0) {
        h ^= ZOBRIST_CASTLE_RIGHTS(CSL_WKSIDE);
    }
    if ((pos->castle & CSL_WQSIDE) != 0) {
        h ^= ZOBRIST_CASTLE_RIGHTS(CSL_WQSIDE);
    }
    if ((pos->castle & CSL_BKSIDE) != 0) {
        h ^= ZOBRIST_CASTLE_RIGHTS(CSL_BKSIDE);
    }
    if ((pos->castle & CSL_BQSIDE) != 0) {
        h ^= ZOBRIST_CASTLE_RIGHTS(CSL_BQSIDE);
    }
    if (pos->enpassant != EP_NONE) {
        h ^= ZOBRIST_ENPASSANT(pos->enpassant);
    }
    return h;
}

int position_from_fen(struct position *pos, const char *fen) {
    int rank;
    int file;
    char c;
    int halfmoves = 0;
    int nmoves = 0;

    pos->nmoves = 1;
    pos->wtm = WHITE;
    pos->halfmoves = 0;
    pos->castle = CSL_NONE;
    pos->enpassant = EP_NONE;
    memset(&pos->brd[0], 0, sizeof(pos->brd[0]) * 10);
    memset(&pos->sqtopc[0], EMPTY, sizeof(pos->sqtopc[0]) * 64);
    KSQ(*pos, WHITE) = 64;
    KSQ(*pos, BLACK) = 64;
    pos->side[WHITE] = 0ull;
    pos->side[BLACK] = 0ull;

    // piece placements
    for (rank = RANK_8; rank >= RANK_1; --rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            c = *fen++;
            switch (c) {
            case 0:
                fprintf(stderr, "FEN string ended too early\n");
                return 1;
            case 'P':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, PAWN);
                pos->brd[PIECE(WHITE, PAWN)] |= MASK(SQUARE(file, rank));
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'N':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, KNIGHT);
                pos->brd[PIECE(WHITE, KNIGHT)] |= MASK(SQUARE(file, rank));
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'B':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, BISHOP);
                pos->brd[PIECE(WHITE, BISHOP)] |= MASK(SQUARE(file, rank));
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'R':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, ROOK);
                pos->brd[PIECE(WHITE, ROOK)] |= MASK(SQUARE(file, rank));
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'Q':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, QUEEN);
                pos->brd[PIECE(WHITE, QUEEN)] |= MASK(SQUARE(file, rank));
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'K':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(WHITE, KING);
                KSQ(*pos, WHITE) = SQUARE(file, rank);
                pos->side[WHITE] |= MASK(SQUARE(file, rank));
                break;
            case 'p':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, PAWN);
                pos->brd[PIECE(BLACK, PAWN)] |= MASK(SQUARE(file, rank));
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            case 'n':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, KNIGHT);
                pos->brd[PIECE(BLACK, KNIGHT)] |= MASK(SQUARE(file, rank));
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            case 'b':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, BISHOP);
                pos->brd[PIECE(BLACK, BISHOP)] |= MASK(SQUARE(file, rank));
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            case 'r':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, ROOK);
                pos->brd[PIECE(BLACK, ROOK)] |= MASK(SQUARE(file, rank));
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            case 'q':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, QUEEN);
                pos->brd[PIECE(BLACK, QUEEN)] |= MASK(SQUARE(file, rank));
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            case 'k':
                pos->sqtopc[SQUARE(file, rank)] = PIECE(BLACK, KING);
                KSQ(*pos, BLACK) = SQUARE(file, rank);
                pos->side[BLACK] |= MASK(SQUARE(file, rank));
                break;
            default:
                if (c >= '1' && c <= '8') {
                    file += c - '0' - 1; // file will get incremented by for loop
                } else {
                    fprintf(stderr, "Invalid empty file specification: %c\n", c);
                    return 2;
                }
            }
        }
        assert(rank == RANK_1 || *fen == '/');
        assert(rank != RANK_1 || *fen == ' ');
        ++fen;
    }

    // active color
    c = *fen++;
    switch (c) {
    case 'w':
        pos->wtm = WHITE;
        break;
    case 'b':
        pos->wtm = BLACK;
        break;
    default:
        fprintf(stderr, "Invalid color: %c\n", c);
        return 3;
    }

    if (*fen++ != ' ') {
        fprintf(stderr, "Expected space after color spec, instead %d\n", *(fen - 1));
        return 4;
    }

    // castling availability
    while ((c = *fen++) != ' ') {
        switch (c) {
        case 0:
            return 5;
        case '-':
            pos->castle = CSL_NONE;
            break;
        case 'K':
            pos->castle |= CSL_WKSIDE;
            break;
        case 'Q':
            pos->castle |= CSL_WQSIDE;
            break;
        case 'k':
            pos->castle |= CSL_BKSIDE;
            break;
        case 'q':
            pos->castle |= CSL_BQSIDE;
            break;
        default:
            return 6;
        }
    }

    // en passant
    c = *fen++;
    if (c == 0) {
        return 7;
    } else if (c == '-') {
        pos->enpassant = EP_NONE;
    } else if (c >= 'a' && c <= 'h') {
        file = c - 'a';
        c = *fen++;
        if (c >= '1' && c <= '8') {
            rank = c - '1';
            if (rank == RANK_3) {
                pos->enpassant = SQUARE(file, rank);
            } else if (rank == RANK_6) {
                pos->enpassant = SQUARE(file, rank);
            } else {
                return 10;
            }
        } else {
            return 8;
        }
    } else {
        return 9;
    }

    assert((pos->enpassant == EP_NONE) || (pos->enpassant >= A3 && pos->enpassant <= H3) ||
           (pos->enpassant >= A6 && pos->enpassant <= H6));

    c = *fen++;
    if (c == 0) {
        // no move specification, just use 0
        pos->halfmoves = 0;
        pos->nmoves = 0;
        return 0;
    } else if (c != ' ') {
        return 11;
    }

    // half moves
    while ((c = *fen++) != ' ') {
        if (c == 0) {
            return 12;
        }
        if (c < '0' || c > '9') {
            return 13;
        }
        halfmoves *= 10;
        halfmoves += c - '0';
    }
    pos->halfmoves = halfmoves;

    // full moves
    while ((c = *fen++)) {
        if (c < '0' || c > '9') {
            return 14;
        }
        nmoves *= 10;
        nmoves += c - '0';
    }
    pos->nmoves = nmoves;

    return validate_position(pos);
}

void position_to_fen(const struct position *pos) {
    const uint8_t *s2p = &pos->sqtopc[0];
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            const int sq = SQUARE(file, rank);
            if (s2p[sq] != EMPTY) {
                if (empty > 0) {
                    printf("%d", empty);
                    empty = 0;
                }
                printf("%c", visual_pcs[s2p[sq]]);
            } else {
                ++empty;
            }
        }
        if (empty > 0) {
            printf("%d", empty);
        }
        if (rank != 0) {
            printf("/");
        }
    }

    printf(" %c ", pos->wtm == WHITE ? 'w' : 'b');

    if (pos->castle == CSL_NONE) {
        printf("-");
    } else {
        if (pos->castle & CSL_WKSIDE) {
            printf("K");
        }
        if (pos->castle & CSL_WQSIDE) {
            printf("K");
        }
        if (pos->castle & CSL_BKSIDE) {
            printf("k");
        }
        if (pos->castle & CSL_BQSIDE) {
            printf("q");
        }
    }
    printf(" %s ", pos->enpassant == EP_NONE ? "-" : sq_to_str[pos->enpassant]);
    printf("%d %d\n", pos->halfmoves, pos->nmoves);
}

void position_print(FILE *os, const struct position *pos) {
    fprintf(os, "+---+---+---+---+---+---+---+---+\n");
    for (int rank = RANK_8; rank >= RANK_1; --rank) {
        for (int file = FILE_A; file <= FILE_H; ++file) {
            fprintf(os, "| %c ", visual_pcs[pos->sqtopc[SQUARE(file, rank)]]);
        }
        fprintf(os, "|\n+---+---+---+---+---+---+---+---+\n");
    }
    fprintf(os, "%s to move\n", pos->wtm == WHITE ? "White" : "Black");
    if ((pos->castle & CSL_WKSIDE) != 0) {
        fprintf(os, "K");
    }
    if ((pos->castle & CSL_WQSIDE) != 0) {
        fprintf(os, "Q");
    }
    if ((pos->castle & CSL_BKSIDE) != 0) {
        fprintf(os, "k");
    }
    if ((pos->castle & CSL_BQSIDE) != 0) {
        fprintf(os, "q");
    }
    fprintf(os, "\n");
    fprintf(os, "Half moves: %d\n", pos->halfmoves);
    fprintf(os, "Full moves: %d\n", pos->nmoves);
    if (pos->enpassant == EP_NONE) {
        fprintf(os, "En Passant target: none\n");
    } else {
        fprintf(os, "En Passant target: %s\n", sq_to_str[pos->enpassant]);
    }
}

int validate_position(struct position *pos) {
    int pc;
    int color;
    int contra;
    int sq;
    int white_kings = 0;
    int black_kings = 0;

    for (sq = A1; sq <= H8; ++sq) {
        if (pos->sqtopc[sq] == EMPTY) {
            if ((pos->side[WHITE] & MASK(sq)) != 0) {
                fprintf(stderr, "\nvalidate_position: sqtopc empty on %s, but "
                                "white full bitboard is not empty\n",
                        sq_to_str[sq]);
                return 1;
            }
            if ((pos->side[BLACK] & MASK(sq)) != 0) {
                fprintf(stderr, "\nvalidate_position: sqtopc empty on %s, but "
                                "black full bitboard is not empty\n",
                        sq_to_str[sq]);
                return 2;
            }
        } else {
            pc = pos->sqtopc[sq];
            color = PIECECOLOR(pc);
            contra = FLIP(color);

            // check full side bitboards
            if ((pos->side[color] & MASK(sq)) == 0) {
                // full side board doesn't have this square occupied
                fprintf(stderr, "\nvalidate_position: sqtopc has %c on %s, but "
                                "%s (same) bitboard is empty\n",
                        visual_pcs[pc], sq_to_str[sq], COLORSTR(color));
                fprintf(stderr, "\nsq = %d, pc = %d, pos->sqtopc[sq] = %d\n", sq, pc, pos->sqtopc[sq]);
                return 3;
            }
            if ((pos->side[contra] & MASK(sq)) != 0) {
                // other side's full board has this square occupied
                fprintf(stderr, "\nvalidate_position: sqtopc has %c on %s, but "
                                "%s (contra) bitboard is not empty\n",
                        visual_pcs[pc], sq_to_str[sq], COLORSTR(color));
                fprintf(stderr, "%" PRIu64 "\n", pos->side[contra]);
                return 4;
            }

            // check piece bitboards
            if (pc == PIECE(WHITE, KING)) {
                if (sq != KSQ(*pos, WHITE)) {
                    fprintf(stderr, "\nvalidate_position: white king location incorrect. "
                                    "sqtopc[%d] = WHITE KING, KSQ(WHITE) = %d\n",
                            sq, KSQ(*pos, WHITE));
                    return 22;
                }
            } else if (pc == PIECE(BLACK, KING)) {
                if (sq != KSQ(*pos, BLACK)) {
                    fprintf(stderr, "\nvalidate_position: black king location incorrect\n");
                    return 23;
                }
            } else if ((pos->brd[pc] & MASK(sq)) == 0) {
                fprintf(stderr, "\nvalidate_position: sqtoc has %c on %s, but "
                                "bitboard does not\n",
                        visual_pcs[pc], sq_to_str[sq]);
                return 5;
            }

            if (pc == PIECE(WHITE, KING)) {
                ++white_kings;
            }
            if (pc == PIECE(BLACK, KING)) {
                ++black_kings;
            }
        }
    }

    for (pc = PIECE(WHITE, KNIGHT); pc <= PIECE(BLACK, KING); ++pc) {
        if (pc == PIECE(WHITE, KING)) {
            if (pos->sqtopc[KSQ(*pos, WHITE)] != PIECE(WHITE, KING)) {
                fprintf(stderr, "\nvalidate_position: white king location\n");
                return 24;
            }
            continue;
        }
        if (pc == PIECE(BLACK, KING)) {
            if (pos->sqtopc[KSQ(*pos, BLACK)] != PIECE(BLACK, KING)) {
                fprintf(stderr, "\nvalidate_position: black king location\n");
                return 25;
            }
            continue;
        }

        for (sq = A1; sq <= H8; ++sq) {
            if ((pos->brd[pc] & MASK(sq)) != 0) {
                if (pos->sqtopc[sq] != pc) {
                    fprintf(stderr, "\nvalidate_position: pos->brd[%c] has a "
                                    "piece on %s, sqtopc has %c\n",
                            visual_pcs[pc], sq_to_str[sq], visual_pcs[pos->sqtopc[sq]]);
                    return 6;
                }
            }
        }
    }

    if (white_kings != 1) {
        fprintf(stderr, "\nvalidate_position: %d white kings found!\n", white_kings);
        return 7;
    }
    if (black_kings != 1) {
        fprintf(stderr, "\nvalidate_position: %d black kings found!\n", black_kings);
        return 8;
    }

    for (sq = A1; sq <= H1; ++sq) {
        if (pos->sqtopc[sq] == PIECE(WHITE, PAWN)) {
            fprintf(stderr, "\nvalidate_position: white pawn on %s\n", sq_to_str[sq]);
            return 9;
        } else if (pos->sqtopc[sq] == PIECE(BLACK, PAWN)) {
            fprintf(stderr, "\nvalidate_position: black pawn on %s\n", sq_to_str[sq]);
            return 10;
        }
    }

    if ((pos->castle & CSL_WQSIDE) != 0) {
        if (pos->sqtopc[A1] != PIECE(WHITE, ROOK)) {
            return 11;
        } else if (pos->sqtopc[E1] != PIECE(WHITE, KING)) {
            return 12;
        }
    } else if ((pos->castle & CSL_WKSIDE) != 0) {
        if (pos->sqtopc[H1] != PIECE(WHITE, ROOK)) {
            return 13;
        } else if (pos->sqtopc[E1] != PIECE(WHITE, KING)) {
            return 14;
        }
    } else if ((pos->castle & CSL_BQSIDE) != 0) {
        if (pos->sqtopc[A8] != PIECE(BLACK, ROOK)) {
            return 15;
        } else if (pos->sqtopc[E8] != PIECE(BLACK, KING)) {
            return 16;
        }
    } else if ((pos->castle & CSL_BKSIDE) != 0) {
        if (pos->sqtopc[H8] != PIECE(BLACK, ROOK)) {
            return 17;
        } else if (pos->sqtopc[E8] != PIECE(BLACK, KING)) {
            return 18;
        }
    }

    return 0;
}

uint8_t rook_square_to_castle_flag(uint8_t sq) {
    // TODO(plesslie): make 64 entry lookup table
    uint8_t result;
    switch (sq) {
    case A8:
        result = CSL_BQSIDE;
        break;
    case H8:
        result = CSL_BKSIDE;
        break;
    case A1:
        result = CSL_WQSIDE;
        break;
    case H1:
        result = CSL_WKSIDE;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

uint64_t make_move(struct position *pos, struct savepos *sp, move m, uint64_t hash) {
    const uint8_t side = pos->wtm;
    const uint8_t contra = FLIP(side);
    const uint32_t tosq = TO(m);
    const uint32_t fromsq = FROM(m);
    const uint64_t from = MASK(fromsq);
    const uint64_t to = MASK(tosq);
    const uint32_t pc = pos->sqtopc[fromsq];
    const uint32_t topc = pos->sqtopc[tosq];
    const uint32_t flags = FLAGS(m);
    const int promopc = PIECE(side, PROMO_PC(m));
    const int epsq = side == WHITE ? tosq - 8 : tosq + 8;
    uint64_t *restrict const pcs = pc != PIECE(side, KING) ? &pos->brd[pc] : 0;
    uint8_t *restrict const s2p = pos->sqtopc;
    uint64_t *restrict const rooks = &pos->brd[PIECE(side, ROOK)];
    uint8_t *restrict const castle = &pos->castle;
    int ksq;
    int rsq;
    uint8_t castle_flag;

    assert(tosq != fromsq);
    assert(topc != PIECE(WHITE, KING) && topc != PIECE(BLACK, KING));

    sp->halfmoves = pos->halfmoves;
    sp->enpassant = pos->enpassant;
    sp->castle = *castle;
    sp->captured_pc = topc;
    hash ^= ZOBRIST_SIDE_TO_MOVE();
    if (pos->enpassant != EP_NONE) {
        hash ^= ZOBRIST_ENPASSANT(pos->enpassant);
        pos->enpassant = EP_NONE;
    }
    switch (flags) {
    case FLG_NONE:
        assert(((pcs != 0) && pc != PIECE(side, KING)) || ((pcs == 0) && pc == PIECE(side, KING)));
        if (pcs) {
            *pcs &= ~from;
            *pcs |= to;
        } else {
            KSQ(*pos, side) = tosq;
            if (side == WHITE) {
                if ((*castle & CSL_WKSIDE) != 0) {
                    hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_WKSIDE);
                }
                if ((*castle & CSL_WQSIDE) != 0) {
                    hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_WQSIDE);
                }
                *castle &= ~CSL_SIDE(WHITE);
            } else {
                if ((*castle & CSL_BKSIDE) != 0) {
                    hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_BKSIDE);
                }
                if ((*castle & CSL_BQSIDE) != 0) {
                    hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_BQSIDE);
                }
                *castle &= ~CSL_SIDE(BLACK);
            }
        }
        hash ^= ZOBRIST_BOARD_SQ(s2p[fromsq], fromsq);
        s2p[fromsq] = EMPTY;
        hash ^= ZOBRIST_BOARD_SQ(pc, tosq);
        s2p[tosq] = pc;
        pos->side[side] &= ~from;
        pos->side[side] |= to;
        if (topc != EMPTY) {
            hash ^= ZOBRIST_BOARD_SQ(topc, tosq);
            pos->brd[topc] &= ~to;
            pos->side[contra] &= ~to;
            castle_flag = rook_square_to_castle_flag(tosq);
            if (castle_flag && (*castle & castle_flag) != 0) {
                *castle &= ~castle_flag;
                hash ^= ZOBRIST_CASTLE_RIGHTS(castle_flag);
            }
        } else if (pc == PIECE(side, PAWN) && (from & RANK2(side)) && (to & EP_SQUARES(side))) {
            pos->enpassant = epsq;
            hash ^= ZOBRIST_ENPASSANT(epsq);
            assert((pos->enpassant >= A3 && pos->enpassant <= H3) || (pos->enpassant >= A6 && pos->enpassant <= H6));
        }
        if (pc == PIECE(side, ROOK)) {
            castle_flag = rook_square_to_castle_flag(fromsq);
            if (castle_flag && (*castle & castle_flag) != 0) {
                *castle &= ~castle_flag;
                hash ^= ZOBRIST_CASTLE_RIGHTS(castle_flag);
            }
        }
        break;
    case FLG_EP:
        assert(pc == PIECE(side, PAWN) && topc == EMPTY);
        hash ^= ZOBRIST_BOARD_SQ(pc, fromsq);
        hash ^= ZOBRIST_BOARD_SQ(pc, tosq);
        hash ^= ZOBRIST_BOARD_SQ(PIECE(contra, PAWN), epsq);

        *pcs &= ~from;
        *pcs |= to;
        pos->brd[PIECE(contra, PAWN)] &= ~MASK(epsq);
        s2p[epsq] = EMPTY;
        s2p[fromsq] = EMPTY;
        s2p[tosq] = PIECE(side, PAWN);
        pos->side[side] &= ~from;
        pos->side[side] |= to;
        pos->side[contra] &= ~MASK(epsq);
        break;
    case FLG_PROMO:
        assert(pc == PIECE(side, PAWN));
        *pcs &= ~from;
        pos->brd[promopc] |= to;
        hash ^= ZOBRIST_BOARD_SQ(promopc, tosq);
        s2p[tosq] = promopc;
        hash ^= ZOBRIST_BOARD_SQ(pc, fromsq);
        s2p[fromsq] = EMPTY;
        pos->side[side] &= ~from;
        pos->side[side] |= to;
        if (topc != EMPTY) {
            hash ^= ZOBRIST_BOARD_SQ(topc, tosq);
            pos->brd[topc] &= ~to;
            pos->side[contra] &= ~to;
            castle_flag = rook_square_to_castle_flag(tosq);
            if (castle_flag && (*castle & castle_flag) != 0) {
                *castle &= ~castle_flag;
                hash ^= ZOBRIST_CASTLE_RIGHTS(castle_flag);
            }
        }
        break;
    case FLG_CASTLE:
        assert(pc == PIECE(side, KING));
        switch (tosq) {
        case H1:
            ksq = G1;
            rsq = F1;
            break;
        case A1:
            ksq = C1;
            rsq = D1;
            break;
        case H8:
            ksq = G8;
            rsq = F8;
            break;
        case A8:
            ksq = C8;
            rsq = D8;
            break;
        default:
            unreachable();
        }
        KSQ(*pos, side) = ksq;
        *rooks &= ~MASK(tosq);
        *rooks |= MASK(rsq);
        hash ^= ZOBRIST_BOARD_SQ(PIECE(side, KING), fromsq);
        hash ^= ZOBRIST_BOARD_SQ(PIECE(side, KING), ksq);

        hash ^= ZOBRIST_BOARD_SQ(PIECE(side, ROOK), tosq);
        hash ^= ZOBRIST_BOARD_SQ(PIECE(side, ROOK), rsq);

        s2p[fromsq] = s2p[tosq] = EMPTY;
        s2p[ksq] = PIECE(side, KING);
        s2p[rsq] = PIECE(side, ROOK);
        pos->side[side] &= ~(to | from);
        pos->side[side] |= MASK(ksq) | MASK(rsq);
        if (side == WHITE) {
            if ((*castle & CSL_WKSIDE) != 0) {
                hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_WKSIDE);
            }
            if ((*castle & CSL_WQSIDE) != 0) {
                hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_WQSIDE);
            }
            *castle &= ~CSL_SIDE(WHITE);
        } else {
            if ((*castle & CSL_BKSIDE) != 0) {
                hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_BKSIDE);
            }
            if ((*castle & CSL_BQSIDE) != 0) {
                hash ^= ZOBRIST_CASTLE_RIGHTS(CSL_BQSIDE);
            }
            *castle &= ~CSL_SIDE(BLACK);
        }
        break;
    default:
        unreachable();
    }

    pos->wtm = contra;
    if (pc == PIECE(side, PAWN) || topc != EMPTY) {
        pos->halfmoves = 0;
    } else {
        ++pos->halfmoves;
    }
    ++pos->nmoves;

    assert(pos->enpassant == EP_NONE || pc == PIECE(side, PAWN));
    assert(validate_position(pos) == 0);

    return hash;
}

void undo_move(struct position *pos, const struct savepos *sp, move m) {
    const uint8_t side = FLIP(pos->wtm);
    const uint8_t contra = pos->wtm;
    const uint32_t fromsq = FROM(m);
    const uint32_t tosq = TO(m);
    const uint32_t promo = PROMO_PC(m);
    const uint32_t promopc = PIECE(side, promo);
    const uint32_t flags = FLAGS(m);
    const uint32_t pc = pos->sqtopc[tosq];
    const uint32_t cappc = sp->captured_pc;
    const uint64_t from = MASK(fromsq);
    const uint64_t to = MASK(tosq); // REVISIT: all uses of `to' are `~to' so just calculate that?
    const uint32_t epsq = side == WHITE ? tosq - 8 : tosq + 8;
    uint64_t *restrict pcs = pc != PIECE(side, KING) ? &pos->brd[pc] : 0;
    uint8_t *restrict s2p = pos->sqtopc;
    uint64_t *restrict rooks = &pos->brd[PIECE(side, ROOK)];
    uint64_t *restrict sidebb = &pos->side[side];
    uint64_t *restrict contrabb = &pos->side[contra];
    int ksq;
    int rsq;

    assert(validate_position(pos) == 0);
    assert(fromsq >= A1 && fromsq <= H8);
    assert(tosq >= A1 && tosq <= H8);
    assert(side == WHITE || side == BLACK);
    assert(flags == FLG_NONE || flags == FLG_EP || flags == FLG_PROMO || flags == FLG_CASTLE);
    assert(cappc == EMPTY || (cappc >= PIECE(WHITE, KNIGHT) && cappc <= PIECE(BLACK, KING)));
    assert(flags != FLG_PROMO || (promopc >= PIECE(side, KNIGHT) && promopc <= PIECE(side, KING)));

    pos->halfmoves = sp->halfmoves;

    pos->enpassant = sp->enpassant;
    // reset castling flags
    pos->castle = sp->castle;

    // reset side to move
    pos->wtm = side;

    --pos->nmoves;

    switch (flags) {
    case FLG_NONE:
        s2p[fromsq] = pc;
        if (pcs) {
            *pcs |= from;
            *pcs &= ~to;
        } else {
            KSQ(*pos, side) = fromsq;
        }
        *sidebb |= from;
        *sidebb &= ~to;
        s2p[tosq] = cappc;
        if (cappc != EMPTY) {
            pos->brd[cappc] |= to;
            *contrabb |= to;
        }
        break;
    case FLG_EP:
        s2p[fromsq] = pc;
        *pcs |= from;
        *pcs &= ~to;
        *sidebb |= from;
        *sidebb &= ~to;
        *contrabb |= MASK(epsq);
        s2p[tosq] = EMPTY;
        assert((side == WHITE && epsq == (tosq - 8)) || (side == BLACK && epsq == (tosq + 8)));
        s2p[epsq] = PIECE(contra, PAWN);
        pos->brd[PIECE(contra, PAWN)] |= MASK(epsq);
        break;
    case FLG_PROMO:
        pos->brd[PIECE(side, PAWN)] |= from;
        pos->brd[promopc] &= ~to;
        s2p[tosq] = cappc;
        s2p[fromsq] = PIECE(side, PAWN);
        *sidebb |= from;
        *sidebb &= ~to;
        if (cappc != EMPTY) {
            pos->brd[cappc] |= to;
            *contrabb |= to;
        }
        break;
    case FLG_CASTLE:
        assert(cappc == PIECE(side, ROOK));
        switch (tosq) {
        case A1:
            ksq = C1;
            rsq = D1;
            break;
        case H1:
            ksq = G1;
            rsq = F1;
            break;
        case A8:
            ksq = C8;
            rsq = D8;
            break;
        case H8:
            ksq = G8;
            rsq = F8;
            break;
        default:
            unreachable();
            break;
        }
        s2p[fromsq] = PIECE(side, KING);
        s2p[tosq] = PIECE(side, ROOK);
        s2p[ksq] = EMPTY;
        s2p[rsq] = EMPTY;
        *sidebb |= to | from;
        *sidebb &= ~(MASK(ksq) | MASK(rsq));
        *rooks &= ~MASK(rsq);
        *rooks |= to;
        KSQ(*pos, side) = fromsq;
        break;
    default:
        unreachable();
    }

    assert(validate_position(pos) == 0);
}

move parse_xboard_move(struct position *pos, const char *line, int len) {
    if (len < 4 || len > 5) {
        return INVALID_MOVE;
    }
    const int fromfile = getfile(line[0]);
    const int fromrank = getrank(line[1]);
    const int tofile = getfile(line[2]);
    const int torank = getrank(line[3]);

    if (fromrank == -1 || fromfile == -1 || torank == -1 || tofile == -1) {
        return INVALID_MOVE;
    }
    const int from = SQUARE(fromfile, fromrank);
    const int to = SQUARE(tofile, torank);
    if (len == 4) {
        if (from == E1 && KSQ(*pos, WHITE) == E1 && (to == C1 || to == G1)) {
            if (to == C1) {
                return CASTLE(E1, A1);
            } else if (to == G1) {
                return CASTLE(E1, H1);
            } else {
                unreachable();
            }
            // return CASTLE(from, to);
        } else if (from == E8 && KSQ(*pos, BLACK) == E8 && (to == C8 || to == G8)) {
            if (to == C8) {
                return CASTLE(E8, A8);
            } else if (to == G8) {
                return CASTLE(E8, H8);
            } else {
                unreachable();
            }
            // return CASTLE(from, to);
        } else if (to == pos->enpassant && pos->sqtopc[from] == PIECE(pos->wtm, PAWN)) {
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
