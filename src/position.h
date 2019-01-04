#pragma once

#include "move.h"
#include <string_view>

struct Savepos {
    u8 halfmoves;
    u8 epsq;
    u8 castle;
    u8 capture;
};
static_assert(std::is_pod<Savepos>::value == true, "");

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

    Position() noexcept;

    static Position from_fen(std::string_view fen);

    void make_move(Savepos& sp, Move move) noexcept;

    void undo_move(const Savepos& sp, Move move) noexcept;

    [[nodiscard]]
    Square enpassant_target_square() const noexcept {
        assert(enpassant_available());
        // NOTE(peter): If white to move, then last move must have been
        //              black therefore, target square must be on black
        //              side.
        // u8 epsq = _unpack_enpassant_square();
        return white_to_move() ? epsq + A6 : epsq + A3;
    }

    [[nodiscard]]
    bool enpassant_available() const noexcept {
        // u8 epsq = _unpack_enpassant_square();
        return epsq != ENPASSANT_NONE;
    }

    [[nodiscard]]
    Piece piece_on_square(Square square) const noexcept {
        return piece_on_square(square.value());
    }

    [[nodiscard]]
    Piece piece_on_square(u8 square) const noexcept {
        assert(square >= A1 && square <= H8);
        return Piece{sq2p[square]};
    }

    [[nodiscard]]
    bool white_to_move() const noexcept {
        return (flags & 0x01) == 0x01;
    }

    void flip_to_move() noexcept {
        flags ^= 0x01;
    }

    void set_white_to_move(bool white_to_move) {
        flags = (flags & 0xFE) | (white_to_move & 0x01);
    }

    [[nodiscard]]
    u8 castle_flags() const noexcept {
        return castle;
        // return (flags >> 1) & 0x03;
    }

    [[nodiscard]]
    int move_number() const noexcept {
        return moves;
    }

    [[nodiscard]]
    int fifty_move_rule_moves() const noexcept {
        return halfmoves;
    }

private:
    void set_castle_flag(u8 bit) noexcept {
        castle |= bit;
    }

    void clear_castle_flag(u8 bit) noexcept {
        castle &= ~bit;
    }

    void clear_castle_flags() noexcept {
        castle = Position::CASTLE_NONE;
    }

    void set_castle_flags(u8 flags) noexcept {
        // TODO(peter): validate
        castle = flags;
    }

    // u8 _unpack_enpassant_square() noexcept {
    //     return (flags >> 4) & 0x0F;
    // }

    // void _pack_enpassant_square(u8 sq) noexcept {
    //     flags = flags & 

    // }

    // u8 _unpack_castle_flags() noexcept {
    //     return (flags >> 1) & 0x07;
    // }

    // data
    u64 bbrd[10];
    u64 side[2];
    u8  sq2p[64];
    u8  ksqs[2];
    u16 moves;
    u8  halfmoves; // 50-move rule counter // 0..50 = 7-bits

    // REVISIT(peter): I think these will all be unpacked on every move/unmove call
    //                 so it doesn't matter where they are located?
    // flags bitfield:
    // bit0    = wtm
    // bit1..3 = castle
    // bit4..7 = epsq
    u8 flags;
    u8 castle;
    u8 epsq;

    // // TODO(peter): combine wtm, castle, and epsq?
    // u8  wtm;    // 1-bits
    // u8  castle; // 3-bits
    // // NOTE(peter): target square behind the pawn (like in FEN)
    // // If white to move, then we know target square must be on 6-th rank
    // u8  epsq; // 0..8 == 4 bits
};
