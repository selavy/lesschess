#pragma once

#include <string>
#include <string_view>
#include <array>
#include "move.h"
#include "ring_buffer.h"

namespace lesschess {

const std::string start_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct Savepos {
    u8 halfmoves;
    u8 ep_target;
    u8 castle_rights;
    Piece captured;
};
static_assert(std::is_trivially_copyable<Savepos>::value == true, "");

struct Zobrist {
    // square x piece = 64 squares x 6 piece types x 2 colors
    // white to move  = 1
    // castle rights  = 16
    // enpassant file = 8
    static constexpr int SIZE = ((64 * 6 * 2) + 1 + 16 + 8);
    static u64 _values[SIZE];

    static u64 board(Piece piece, Square square) noexcept
    { return _values[board_index(piece, square)]; }

    static u64 side_to_move() noexcept
    { return _values[side_to_move_index()]; }

    static u64 castle_rights(Castle kind) noexcept
    { return _values[castle_rights_index(kind)]; }

    static u64 enpassant(Square square) noexcept
    { return _values[enpassant_index(square)]; }

    static void initialize() {
        srand(42);
        for (int i = 0; i < SIZE; ++i) {
            _values[i] = (static_cast<u64>(rand()) << 32) | static_cast<u64>(rand());
        }

try_again:
        for (int i = 0; i < SIZE; ++i) {
            for (int j = i + 1; j < SIZE; ++j) {
                if (_values[i] == _values[j]) {
                    _values[j] = (static_cast<u64>(rand()) << 32) | static_cast<u64>(rand());
                    goto try_again;
                }
            }
        }
    }

private:
    enum IndexBoundaries
    {
        BOARD_BEGIN         = 0,
        BOARD_END           = BOARD_BEGIN + 12*64,
        SIDE_TO_MOVE_BEGIN  = BOARD_END,
        SIDE_TO_MOVE_END    = SIDE_TO_MOVE_BEGIN + 1,
        CASTLE_RIGHTS_BEGIN = SIDE_TO_MOVE_END,
        CASTLE_RIGHTS_END   = CASTLE_RIGHTS_BEGIN + 16,
        ENPASSANT_BEGIN     = CASTLE_RIGHTS_END,
        ENPASSANT_END       = ENPASSANT_BEGIN + 8,
    };

    static int board_index(Piece piece, Square square) noexcept
    {
        int index = 12*square.value() + piece.value();
        assert(BOARD_BEGIN <= index && index < BOARD_END);
        return index;
    }

    static int side_to_move_index() noexcept
    {
        int index = SIDE_TO_MOVE_BEGIN;
        assert(SIDE_TO_MOVE_BEGIN <= index && index < SIDE_TO_MOVE_END);
        return index;
    }

    static int castle_rights_index(Castle kind) noexcept
    {
        int index = CASTLE_RIGHTS_BEGIN + static_cast<int>(kind);
        assert(CASTLE_RIGHTS_BEGIN <= index && index < CASTLE_RIGHTS_END);
        return index;
    }

    static int enpassant_index(Square square) noexcept
    {
        int index = ENPASSANT_BEGIN + (square.value() % 8);
        assert(ENPASSANT_BEGIN <= index && index < ENPASSANT_END);
        return index;
    }
};

class Position {
public:
    Position() noexcept;
    Position(const Position&) noexcept = default;
    Position& operator=(const Position&) noexcept = default;
    Position(Position&&) noexcept = default;
    Position& operator=(Position&&) noexcept = default;

	[[nodiscard]]
    static Position from_fen(std::string_view fen);
    [[nodiscard]]
    static Position from_ascii(std::string_view ascii);

    [[nodiscard]]
    Move move_from_long_algebraic(std::string_view move) const;

    [[nodiscard]]
    std::string dump_fen() const noexcept;
    [[nodiscard]]
    std::string dump_ascii() const noexcept;

    void make_move(Savepos& sp, Move move) noexcept;

    void undo_move(const Savepos& sp, Move move) noexcept;

    [[nodiscard]]
    Square enpassant_target_square() const noexcept
    { assert(enpassant_available()); return Square(_ep_target); }

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
    bool black_to_move() const noexcept
    { return !white_to_move(); }

    [[nodiscard]]
    Color color_to_move() const noexcept
    { return white_to_move() ? WHITE : BLACK; }

    [[nodiscard]]
    u8 castle_flags() const noexcept
    { return _castle_rights; }

    [[nodiscard]]
    bool castle_allowed(Castle kind) const noexcept
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

    [[nodiscard]]
    int generate_legal_moves(Move* moves) const noexcept;

    [[nodiscard]]
    int generate_captures(Move* moves) const noexcept;

    [[nodiscard]]
    bool in_check(Color side) const noexcept
    { return _generate_checkers(side) != 0; }

    // strict legality checking of a move, for checking is a user
    // input move is legal or not
    [[nodiscard]]
    bool is_legal_move(Move move) const noexcept;

    [[nodiscard]]
    bool attacks(Color side, Square square) const noexcept;

    [[nodiscard]]
    constexpr Color wtm() const noexcept
    { return Color(_wtm); };

    [[nodiscard]]
    int piece_count(Color c, PieceKind p) const noexcept
    { return popcountll(_bboard(c, p)); }

    u64 zobrist_hash() const noexcept
    { return _hash; }

    struct UnitTestAccess
    {
        UnitTestAccess(Position& position) : p(position) {}

        u64 generate_attacked(Color side) const noexcept
        { return p._generate_attacked(side); }
        u64 generate_checkers(Color side) const noexcept
        { return p._generate_checkers(side); }

        Position& p;
    };

    Square ep_target() const noexcept {
        assert(_ep_target != Position::ENPASSANT_NONE);
        return { _ep_target };
    }

    bool is_repetition() const noexcept;

private:
    enum {
        ENPASSANT_NONE = Square::INVALID,
    };

    void _compute_zobrist_hash() noexcept;

    [[nodiscard]]
    u64 _bboard(Color c, PieceKind p) const noexcept
    { return _boards[Piece(c, p).value()]; }

    void _set_white_to_move(bool white_to_move) noexcept
    { _wtm = white_to_move ? WHITE : BLACK; }

    void _set_enpassant_square(u8 sq) noexcept
    {
        _ep_target = sq;
        assert(_ep_target == Position::ENPASSANT_NONE ||
              (_ep_target >= A3 && _ep_target <= H3) ||
              (_ep_target >= A6 && _ep_target <= H6));
    }

    Square _ep_capture_square() const noexcept {
        assert(_ep_target != Position::ENPASSANT_NONE);
        return { _wtm == WHITE ? _ep_target - 8 : _ep_target + 8 };
    }

    void _validate() const noexcept;


    [[nodiscard]]
    u64 _occupied() const noexcept
    { return _sidemask[WHITE] | _sidemask[BLACK]; }

    template <class Iter>
    friend void parse_fen_spec(Iter it, Iter last, Position& position);

    [[nodiscard]]
    bool _is_legal(u64 pinned, Move m) const noexcept;

    static Move* _generate_knight_moves(u64 knights, u64 targets, Move* moves) noexcept;
    static Move* _generate_bishop_moves(u64 bishops, u64 occupied, u64 targets, Move* moves) noexcept;
    static Move* _generate_rook_moves(u64 rooks, u64 occupied, u64 targets, Move* moves) noexcept;
    static Move* _generate_king_moves(Square ksq, u64 targets, Move* moves) noexcept;
    Move* _generate_castle_moves(Color side, Square ksq, Move* moves) const noexcept;
    // bitboard of pieces from `side` that are blocking checking on `kingcolor` king
    Move* _generate_evasions(u64 checkers, Move* moves) const noexcept;
    Move* _generate_non_evasions(Move* moves) const noexcept;
    u64 _generate_pinned(Color side, Color kingcolor) const noexcept;
    u64 _generate_attacked(Color side) const noexcept;
    u64 _generate_checkers(Color side) const noexcept;

private:
    std::array<u64, 10>   _boards;
    std::array<u64, 2>    _sidemask;
    std::array<Piece, 64> _sq2pc;
    std::array<Square, 2> _kings;
    RingBuffer<u64, 50>   _hashs; // TODO: size up to 64?
    u64 _hash;
    u16 _moves;
    u8 _halfmoves;
    u8 _wtm;
    u8 _ep_target;
    u8 _castle_rights;
};

std::ostream& operator<<(std::ostream& os, const Position& position);

} // ~namespace lesschess
