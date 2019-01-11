#pragma once

#include "move.h"
#include <string_view>
#include <array>

struct Savepos {
    u8 halfmoves;
    u8 ep_target;
    u8 castle;
    Piece captured;
};
static_assert(std::is_trivially_copyable<Savepos>::value == true, "");

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

        ENPASSANT_NONE = Square::INVALID,
    };

    Position() noexcept;
    Position(const Position&) noexcept = default;
    Position& operator=(const Position&) noexcept = default;

    static Position from_fen(std::string_view fen);

    // Move move_from_san(std::string_view san) const;

    [[nodiscard]]
    std::string dump_fen() const noexcept;

    void make_move(Savepos& sp, Move move) noexcept;

    void undo_move(const Savepos& sp, Move move) noexcept;

    [[nodiscard]]
    Square enpassant_target_square() const noexcept {
        assert(enpassant_available());
        // NOTE(peter): If white to move, then last move must have been
        //              black therefore, target square must be on black
        //              side.
        // return white_to_move() ? epsq + A6 : epsq + A3;
        return Square(ep_target);
    }

    [[nodiscard]]
    bool enpassant_available() const noexcept {
        return ep_target != ENPASSANT_NONE;
    }

    [[nodiscard]]
    Piece piece_on_square(Square square) const noexcept {
        return piece_on_square(square.value());
    }

    [[nodiscard]]
    Piece piece_on_square(u8 square) const noexcept {
        assert(square >= A1 && square <= H8);
        return Piece(sq2p[square]);
    }

    [[nodiscard]]
    bool white_to_move() const noexcept {
        return wtm == WHITE;
    }

    // TODO(peter): make this private, should only be called on make_move/undo_move
    void flip_to_move() noexcept {
        wtm ^= 1;
    }

    [[nodiscard]]
    u8 castle_flags() const noexcept {
        return castle;
    }

    [[nodiscard]]
    int move_number() const noexcept {
        return moves;
    }

    [[nodiscard]]
    int fifty_move_rule_moves() const noexcept {
        return halfmoves;
    }

    [[nodiscard]]
    bool operator==(const Position& rhs) const noexcept;

    [[nodiscard]]
    bool operator!=(const Position& rhs) const noexcept;

private:
    void _set_white_to_move(bool white_to_move) noexcept {
        wtm = white_to_move ? WHITE : BLACK;
    }

    void _set_castle_flags(u8 flags) noexcept {
        castle = flags;
    }

    void _set_enpassant_square(u8 sq) noexcept {
        ep_target = sq;
        assert(ep_target == Position::ENPASSANT_NONE ||
              (ep_target >= A3 && ep_target <= H3) ||
              (ep_target >= A6 && ep_target <= H6));
    }

    void _validate() const noexcept;

private:
    std::array<u64, 10> boards;
    std::array<u64, 2> sidemask;
    std::array<Piece, 64> sq2p;
    std::array<Square, 2> kings;
    u16 moves;
    u8 halfmoves;
    u8 wtm;
    u8 ep_target;
    u8 castle;
};
