#pragma once

#include "move.h"
#include <string_view>
#include <array>

namespace lesschess {

struct Savepos {
    u8 halfmoves;
    u8 ep_target;
    u8 castle_rights;
    Piece captured;
};
static_assert(std::is_trivially_copyable<Savepos>::value == true, "");

class Position {
public:
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

	[[nodiscard]]
    static Position from_fen(std::string_view fen);
    [[nodiscard]]
    static Position from_ascii(std::string_view ascii);

    [[nodiscard]]
    std::string dump_fen() const noexcept;
    [[nodiscard]]
    std::string dump_ascii() const noexcept;

    void make_move(Savepos& sp, Move move) noexcept;

    void undo_move(const Savepos& sp, Move move) noexcept;

    [[nodiscard]]
    Square enpassant_target_square() const noexcept
    {
        assert(enpassant_available());
        return Square(_ep_target);
    }

    [[nodiscard]]
    bool enpassant_available() const noexcept
    { return _ep_target != ENPASSANT_NONE; }

    [[nodiscard]]
    Piece piece_on_square(Square square) const noexcept
    { return piece_on_square(square.value()); }

    [[nodiscard]]
    Piece piece_on_square(u8 square) const noexcept
    {
        assert(square >= A1 && square <= H8);
        return Piece(_sq2pc[square]);
    }

    [[nodiscard]]
    bool white_to_move() const noexcept
    { return _wtm == WHITE; }

    [[nodiscard]]
    u8 castle_flags() const noexcept
    { return _castle_rights; }

    [[nodiscard]]
    bool castle_kind_allowed(Move::CastleKind kind) const noexcept
    { return (castle_flags() & static_cast<u8>(kind)) != 0; }

    [[nodiscard]]
    int move_number() const noexcept
    { return _moves; }

    [[nodiscard]]
    int fifty_move_rule_moves() const noexcept
    { return _halfmoves; }

    [[nodiscard]]
    bool operator==(const Position& rhs) const noexcept;

    [[nodiscard]]
    bool operator!=(const Position& rhs) const noexcept;

    // TODO(peter): this is probably what the interface should look like:
    // template <class OutputIter>
    // OutputIter generate_legal_moves(OutputIter) const noexcept;

    // TODO(peter): implement
    [[nodiscard]]
    int generate_legal_moves(Move* moves) const noexcept;

    // TODO(peter): implement
    [[nodiscard]]
    bool in_check(Color side) const noexcept
    { return false; }

    // strict legality checking of a move, for checking is a user
    // input move is legal or not
    [[nodiscard]]
    bool is_legal_move(Move move) const noexcept;

    [[nodiscard]]
    bool attacks(Color side, Square square) const noexcept;

    [[nodiscard]]
    constexpr Color wtm() const noexcept
    { return Color(_wtm); };

    // XXX: move this back to private and remove tests
    u64 _generate_attacked(Color side) const noexcept;
    u64 _generate_checkers(Color side) const noexcept;

private:
    void _set_white_to_move(bool white_to_move) noexcept
    { _wtm = white_to_move ? WHITE : BLACK; }

    void _set_enpassant_square(u8 sq) noexcept
    {
        _ep_target = sq;
        assert(_ep_target == Position::ENPASSANT_NONE ||
              (_ep_target >= A3 && _ep_target <= H3) ||
              (_ep_target >= A6 && _ep_target <= H6));
    }

    Square ep_capture_square() const noexcept {
        assert(_ep_target != Position::ENPASSANT_NONE);
        return { _wtm == WHITE ? _ep_target - 8 : _ep_target + 8 };
    }

    Square ep_target() const noexcept {
        assert(_ep_target != Position::ENPASSANT_NONE);
        return { _ep_target };
    }

    void _validate() const noexcept;


    [[nodiscard]]
    u64 _occupied() const noexcept
    { return _sidemask[WHITE] | _sidemask[BLACK]; }

    template <class Iter>
    friend void parse_fen_spec(Iter it, Iter last, Position& position);

    [[nodiscard]]
    u64 _bboard(Color c, PieceKind p) const noexcept
    { return _boards[Piece(c, p).value()]; }

    [[nodiscard]]
    bool _is_legal(u64 pinned, Move m) const noexcept;

    static Move* _generate_knight_moves(u64 knights, u64 targets, Move* moves) noexcept;
    static Move* _generate_bishop_moves(u64 bishops, u64 occupied, u64 targets, Move* moves) noexcept;
    static Move* _generate_rook_moves(u64 rooks, u64 occupied, u64 targets, Move* moves) noexcept;
    static Move* _generate_king_moves(Square ksq, u64 targets, Move* moves) noexcept;
    // bitboard of pieces from `side` that are blocking checking on `kingcolor` king
    Move* _generate_evasions(u64 checkers, Move* moves) const noexcept;
    Move* _generate_non_evasions(Move* moves) const noexcept;
    u64 _generate_pinned(Color side, Color kingcolor) const noexcept;

private:
    std::array<u64, 10>   _boards;
    std::array<u64, 2>    _sidemask;
    std::array<Piece, 64> _sq2pc;
    std::array<Square, 2> _kings;
    u16 _moves;
    u8 _halfmoves;
    u8 _wtm;
    u8 _ep_target;
    u8 _castle_rights;
};

} // ~namespace lesschess
