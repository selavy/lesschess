#pragma once

#include "move.h"
#include <string_view>

struct Savepos {
    u8 halfmoves;
    u8 epsq;
    u8 castle;
    u8 capture;
};

struct Position {
    enum {
        CASTLE_NONE             = 0,
        CASTLE_WHITE_KING_SIDE  = 1 << 0,
        CASTLE_WHITE_QUEEN_SIDE = 1 << 1,
        CASTLE_BLACK_KING_SIDE  = 1 << 2,
        CASTLE_BLACK_QUEEN_SIDE = 1 << 3,
        CASTLE_WHITE = CASTLE_WHITE_KING_SIDE | CASTLE_WHITE_QUEEN_SIDE,
        CASTLE_BLACK = CASTLE_BLACK_KING_SIDE | CASTLE_BLACK_QUEEN_SIDE,
        CASTLE_ALL   = CASTLE_WHITE | CASTLE_BLACK,
    };

    enum {
        ENPASSANT_NONE = 16,
    };

    u64 bbrd[10];
    u64 side[2];
    u8  sq2p[64];
    u8  ksqs[2];
    u16 moves;
    u8  halfmoves; // 50-move rule counter
    // TODO(peter): combine wtm and castle?
    u8  wtm;    // 1-bits
    u8  castle; // 3-bits
    // target square behind the pawn (like in FEN)
    // 0..7  = a3..h3
    // 8..15 = a6..h6
    u8  epsq; // 0..16 == 5 bits

    Position() noexcept;

    static Position from_fen(std::string_view fen);

    void make_move(Savepos& sp, Move move) noexcept;

    void undo_move(const Savepos& sp, Move move) noexcept;

    [[nodiscard]]
    u8 enpassant_target_square() const noexcept {
        assert(enpassant_available());
        return epsq < 8 ? A3 + epsq : A6 + epsq;
    }

    [[nodiscard]]
    bool enpassant_available() const noexcept {
        return epsq != ENPASSANT_NONE;
    }

    [[nodiscard]]
    Piece piece_on_square(u8 square) const noexcept {
        assert(square >= A1 && square <= H8);
        return Piece{sq2p[square]};
    }

    [[nodiscard]]
    bool white_to_move() const noexcept {
        return wtm == WHITE;
    }
};
