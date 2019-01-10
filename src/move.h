#pragma once

#include <cstdint>
#include <cassert>
#include <type_traits>
#include <ostream>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

enum PieceKind {
    // Must have this order so promotion piece can
    // be represented in 2-bits
    KNIGHT = 0,
    BISHOP = 1,
    ROOK   = 2,
    QUEEN  = 3,
    KING   = 4,
    PAWN   = 5,
    N_PIECES,
    EMPTY_SQUARE = N_PIECES*2,
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

enum {
    FILE_A=0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
};

enum {
    RANK_1=0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
};

struct Piece {
    static constexpr const char* const names[] = {
        "white knight",
        "white bishop",
        "white rook",
        "white queen",
        "white king",
        "white pawn",
        "black knight",
        "black bishop",
        "black rook",
        "black queen",
        "black king",
        "black pawn",
        "empty",
    };

    static constexpr char fen_names[] = {
        'N', 'B', 'R', 'Q', 'K', 'P',
        'n', 'b', 'r', 'q', 'k', 'p',
        ' ',
    };

    constexpr Piece() noexcept : rep_(EMPTY_SQUARE) {}

    constexpr explicit Piece(u8 rep) noexcept : rep_{rep} {
        assert(rep >= 0 && rep <= EMPTY_SQUARE);
    }

    constexpr Piece(Color color, PieceKind piece) noexcept
        : rep_(static_cast<u8>(color)*N_PIECES + static_cast<u8>(piece)) {}

    [[nodiscard]]
    constexpr u8 value() const noexcept {
        return rep_;
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return rep_ == EMPTY_SQUARE;
    }

    [[nodiscard]]
    constexpr Color color() const noexcept {
        assert(!empty());
        return rep_ < N_PIECES ? WHITE : BLACK;
    }

    [[nodiscard]]
    constexpr PieceKind kind() const noexcept {
        return empty() ? EMPTY_SQUARE : static_cast<PieceKind>(rep_ % N_PIECES);
    }

    [[nodiscard]]
    constexpr const char *const name() const noexcept {
        return names[rep_];
    }

    [[nodiscard]]
    constexpr char fen() const noexcept {
        return fen_names[rep_];
    }

    u8 rep_;
};
static_assert(sizeof(Piece) == 1, "");
static_assert(std::is_trivially_copyable<Piece>::value == true, "");

constexpr Piece NO_PIECE = Piece{};

constexpr bool operator==(Piece lhs, Piece rhs) noexcept {
    return lhs.value() == rhs.value();
}

constexpr bool operator!=(Piece lhs, Piece rhs) noexcept {
    return !(operator==(lhs, rhs));
}

std::ostream& operator<<(std::ostream& os, Piece pc) noexcept;

struct Square {
    constexpr static u8 INVALID = 64;

    static constexpr const char* const names[65] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "-",
    };

    Square() noexcept : rep_(INVALID) {}
    // TODO(peter): do I need to specify these?
    Square(const Square&) noexcept = default;
    Square(Square&&) noexcept = default;
    Square& operator=(const Square&) noexcept = default;
    Square& operator=(Square&&) noexcept = default;

    constexpr Square(u8 file, u8 rank) noexcept : rep_(8*rank + file)
    { assert(rep_ >= A1 && rep_ <= H8); }

    constexpr Square(u8 sq) noexcept : rep_{sq}
    { assert(rep_ >= A1 && rep_ <= H8); }

    [[nodiscard]]
    constexpr u8 value() const noexcept
    { return rep_; }

    [[nodiscard]]
    constexpr u64 mask() const noexcept
    { return static_cast<u64>(1) << rep_; }

    [[nodiscard]]
    constexpr const char* const name() const noexcept
    { return names[rep_]; }

    [[nodiscard]]
    constexpr u8 file() const noexcept {
        // return rep_ % 8;
        return rep_ & 7;
    }

    [[nodiscard]]
    constexpr u8 rank() const noexcept {
        // return rep_ / 8;
        return rep_ >> 3;
    }

    [[nodiscard]]
    constexpr bool valid() const noexcept {
        return rep_ != INVALID;
    }

    u8 rep_;
};
static_assert(sizeof(Square) == 1, "");
static_assert(std::is_trivially_copyable<Square>::value == true, "");

constexpr bool operator==(Square lhs, Square rhs) noexcept {
    return lhs.rep_ == rhs.rep_;
}

constexpr bool operator!=(Square lhs, Square rhs) noexcept {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, Square sq) noexcept;

struct ep_capture_tag {};
struct castle_tag {};
struct promotion_tag {}; // NOTE(peter): for symmetry

class Move {
public:
    enum Flags : u16 {
        NONE      = 0,
        ENPASSANT = 1,
        PROMOTION = 2,
        CASTLE    = 3,
    };

    Move() noexcept = default;

    constexpr Move(u8 from, u8 to) noexcept
        : rep_((to << 0u) | (from << 6u))
    {
        assert(from >= A1 && from <= H8);
        assert(to   >= A1 && to   <= H8);
        assert(flags() == Flags::NONE);
    }

    constexpr Move(u8 from, u8 to, ep_capture_tag) noexcept
        : rep_((to << 0u) | (from << 6u) | (Flags::ENPASSANT << 14u))
    {
        assert(((from >= A4 && from <= H4) && (to >= A3 && to <= H3)) ||
               ((from >= A5 && from <= H5) && (to >= A6 && to <= H6)));
        assert(flags() == Flags::ENPASSANT);
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
        : rep_((to << 0u) | (from << 6u) | (promo << 12u) | (Flags::PROMOTION << 14u))
    {
        assert((from >= A2 && from <= H2) || (from >= A7 && from <= H7));
        assert((to   >= A1 && to   <= H1) || (to   >= A8 && to   <= H8));
        assert(promo == KNIGHT | promo == BISHOP | promo == ROOK | promo == QUEEN);
        assert(promotion() == promo);
        assert(flags() == Flags::PROMOTION);
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
    constexpr PieceKind promotion() const noexcept {
        // TODO(peter): assert on flags
        assert(is_promotion());
        return static_cast<PieceKind>((rep_ >> 12u) & 0x03);
    }

    [[nodiscard]]
    constexpr Flags flags() const noexcept {
        return static_cast<Flags>((rep_ >> 14u) & 0x03);
    }

    [[nodiscard]]
    constexpr bool is_promotion() const noexcept {
        return flags() == Flags::PROMOTION;
    }

    [[nodiscard]]
    constexpr bool is_castle() const noexcept {
        return flags() == Flags::CASTLE;
    }

    [[nodiscard]]
    constexpr bool is_enpassant() const noexcept {
        return flags() == Flags::ENPASSANT;
    }

private:
    u16 rep_;
};
static_assert(sizeof(Move) == 2, "");
static_assert(std::is_pod<Move>::value == true, "");

