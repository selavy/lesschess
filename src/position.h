#pragma once

#include "move.h"
#include <string_view>
#include <optional>


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

    u64 bbrd_[10];
    u64 side_[2];
    u8  sq2p_[64];
    u8  ksq_[2];
    u16 moves_;
    u8  halfmoves_; // 50-move rule counter
    // TODO(peter): combine wtm and castle?
    u8  wtm_;    // 1-bits
    u8  castle_; // 3-bits
    // target square behind the pawn (like in FEN)
    // 0..7  = a3..h3
    // 8..15 = a6..h6
    u8  epsq_; // 0..16 == 5 bits

    static std::optional<Position> from_fen(std::string_view fen);
    void make_move(Savepos& sp, Move move);
    void undo_move(const Savepos& sp, Move move);

    [[nodiscard]]
    u8 castle() const noexcept {
        return castle_;
    }

    [[nodiscard]]
    u8 epsq() const noexcept {
        return epsq_ < 8 ? A3 + epsq_ : A6 + epsq_;
    }

    [[nodiscard]]
    ColorPiece piece_on_square(u8 square) const noexcept {
        assert(square >= A1 && square <= H8);
        return sq2p_[square];
    }
};


std::string_view::iterator
parse_fen_board(std::string_view::iterator first,
                std::string_view::iterator last,
                Position& p) noexcept;
