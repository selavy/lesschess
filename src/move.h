#pragma once

#include <cstdint>
#include <cassert>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

enum Piece {
    // Must have this order so promotion piece can
    // be represented in 2-bits
    KNIGHT = 0,
    BISHOP = 1,
    ROOK   = 2,
    QUEEN  = 3,
    KING   = 4,
    PAWN   = 5,
    N_PIECES = 6,
};

enum Color {
    WHITE = 0,
    BLACK = 1,
};

enum {
    N_ROWS = 8,
    N_COLS = 8,
    N_SQUARES = N_ROWS*N_COLS,

    NO_ENPASSANT = 16,
};

// clang-format off
enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};
// clang-format on

struct ep_capture_tag {};
struct castle_tag {};
struct promotion_tag {}; // NOTE(peter): for symmetry

class Move {
public:
    enum Flags : u16 {
        NONE   = 0,
        EP     = 1,
        PROMO  = 2,
        CASTLE = 3,
    };

    constexpr Move() noexcept : rep_{0u} {}

    constexpr Move(u8 from, u8 to) noexcept
        : rep_((to << 0u) | (from << 6u))
    {
        assert(from >= A1 && from <= H8);
        assert(to   >= A1 && to   <= H8);
        assert(flags() == Flags::NONE);
    }

    constexpr Move(u8 from, u8 to, ep_capture_tag) noexcept
        : rep_((to << 0u) | (from << 6u) | (Flags::EP << 14u))
    {
        assert(((from >= A4 && from <= H4) && (to >= A3 && to <= H3)) ||
               ((from >= A5 && from <= H5) && (to >= A6 && to <= H6)));
        assert(flags() == Flags::EP);
    }

    constexpr Move(u8 from, u8 to, castle_tag) noexcept
        : rep_((to << 0u) | (from << 6u) | (Flags::CASTLE << 14u))
    {
        assert((from == E1 && to == G1) ||
               (from == E1 && to == C1) ||
               (from == E8 && to == G8) ||
               (from == E8 && to == C8));
        assert(flags() == Flags::CASTLE);
    }

    constexpr Move(u8 from, u8 to, u8 promo) noexcept
        : rep_((to << 0u) | (from << 6u) | (promo << 12u) | (Flags::PROMO << 14u))
    {
        assert((from >= A2 && from <= H2) || (from >= A7 && from <= H7));
        assert((to   >= A1 && to   <= A1) || (to   >= A8 && to   <= H8));
        assert(flags() == Flags::PROMO);
    }

    [[nodiscard]]
    constexpr u8 to() const noexcept {
        return (rep_ >> 0u) & 0x3f;
    }

    [[nodiscard]]
    constexpr u8 from() const noexcept {
        return (rep_ >> 6u) & 0x3f;
    }

    [[nodiscard]]
    constexpr u8 promo() const noexcept {
        // TODO(peter): assert on flags
        assert(flags() == Flags::PROMO);
        return (rep_ >> 12u) & 0x03;
    }

    [[nodiscard]]
    constexpr Flags flags() const noexcept {
        return static_cast<Flags>((rep_ >> 14u) & 0x03);
    }

    [[nodiscard]]
    constexpr bool is_promo() const noexcept {
        return flags() == Flags::PROMO;
    }

    [[nodiscard]]
    constexpr bool is_castle() const noexcept {
        return flags() == Flags::CASTLE;
    }

    [[nodiscard]]
    constexpr bool is_enpassant() const noexcept {
        return flags() == Flags::EP;
    }

private:
    u16 rep_;
};
static_assert(sizeof(Move) == 2, "Move must be 2-bytes!");

// class Position {
//     u64 board[10]; // NOTE(peter): don't need bitmap for king position, use :ksq:
//     u8  sq2pc[N_SQUARES];
//     u8  ksq[2];
//     // number of full moves
//     u16 nmoves;
//     // number of tempi since last capture or pawn advance
//     // for 50-move rule
//     u8  halfmoves;
//     u8  castle;
//     // target square behind pawn (like FEN)
//     // 16 = no en passant
//     // 0..7  = a3..h3
//     // 8..15 = a6..h6
//     u8  epsq;
// };
// 
// class SavePos {
//     uint8_t halfmoves;
//     uint8_t epsq;
//     uint8_t 
// }
