#include "search.h"
#include "evaluate.h"
#include <array>
#include <cassert>
#include <functional>
#include <iostream> // TEMP

namespace lesschess {

// Number of moves upper bound:
// K -> 8 + 2 castles
// Q -> 27
// R -> 14 x 2 = 28
// N -> 8 x 2 = 16
// P -> (2 moves + 2 captures) x 8 = 32 (same as if all 8 pawns could promote)
// --> 139
//
// Worst worst case is 7 promoted queens => 7 x 27 + 8 x 2 = 394
// 128 is probably safe? 256 is definitely safe since willing to bet the the 7 Qs
// position isn't going to happen -- will either be checkmate or stalemate.

// TODO: what is the maximum number of possible moves in a position?
using Moves = std::vector<Move>; // TODO: change this back to std::array<Move, 256>?

template <int N>
void PrimaryVariation<N>::dump() const
{
    // for (const Move& move : *this) {
    for (int i = 0; i < count; ++i) {
        const Move& move = moves[i];
        std::cout << move.to_long_algebraic_string() << " ";
    }
}

int negamax(Position& position, int alpha, int beta, int depth, PV& pv, TT& tt, bool useTT, s64& nodes)
{
    ++nodes;

    int alpha_orig = alpha;
    auto& tt_entry = tt.find(position.zobrist_hash());
    if (useTT && tt_entry.is_valid() && tt_entry.depth >= depth) {
        tt.record_hit();
        if (tt_entry.is_exact()) {
            return tt_entry.value;
        } else if (tt_entry.is_lower()) {
            // std::cout << "adjusting alpha from to MAX(" << alpha << ", " << tt_entry.value << ")\n";
            alpha = std::max(alpha, tt_entry.value);
        } else if (tt_entry.is_upper()) {
            // std::cout << "adjusting beta from to MIN(" << beta << ", " << tt_entry.value << ")\n";
            beta = std::min(beta, tt_entry.value);
        } else {
            assert(0 && "invalid tt entry");
        }

        if (alpha >= beta) {
            return tt_entry.value;
        }
    }

    int value;
    if (depth == 0) {
        int score = evaluate(position);
        value = position.white_to_move() ? score : -score;
    } else if (position.fifty_move_rule_moves() >= 50) {
        // TODO: check for 3-move repetition
        value = FIFTY_MOVE_RULE_DRAW;
    } else {
        Moves moves{256};
        int nmoves = position.generate_legal_moves(&moves[0]);
        if (nmoves == 0) {
            // TODO: cache `checkers` from generate_legal_moves() so we can check if mate or stalemate?
            value = position.in_check(position.color_to_move()) ? -CHECKMATE : STALEMATE;
        } else {
            value = -MAX_SCORE;
            Savepos sp;
            for (int i = 0; i < nmoves; ++i) {
                position.make_move(sp, moves[i]);
                pv.push(moves[i]);
                value = std::max(value, negamax(position, /*alpha*/-beta, /*beta*/-alpha, depth - 1, pv, tt, useTT, nodes));
                pv.pop();
                position.undo_move(sp, moves[i]);
                alpha = std::max(alpha, value);
                if (alpha >= beta) {
                    break;
                }
            }
        }
    }

    tt_entry.value = -value;
    if (value <= alpha_orig) {
        tt_entry.flag = TT::Flag::kUpper;
    } else if (value >= beta) {
        tt_entry.flag = TT::Flag::kLower;
    } else {
        tt_entry.flag = TT::Flag::kExact;
    }
    tt_entry.depth = depth;

    return -value;
}

SearchResult search(Position& position, TT& tt, int max_depth, bool useTT, s64& nodes_searched)
{
    nodes_searched = 0;

    int bestmove = -1;
    int bestscore = -MAX_SCORE;
    Savepos sp;
    Moves moves{256};
    int nmoves = position.generate_legal_moves(&moves[0]);
    PV pv ;
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        pv.push(moves[i]);
        int score = negamax(
                        position,
                        /*alpha*/-MAX_SCORE,
                        /*beta*/MAX_SCORE,
                        /*depth*/max_depth - 1,
                        pv,
                        tt,
                        useTT,
                        nodes_searched
                    );
		position.undo_move(sp, moves[i]);
        pv.pop();
        if (score > bestscore) {
            bestscore = score;
            bestmove = i;
        }
    }
    assert(bestmove != -1);
    bestscore = position.white_to_move() ? bestscore : -bestscore;
    return {moves[bestmove], bestscore};
}

SearchResult easy_search(Position& position, bool useTT)
{
    TT tt;
    s64 nodes_searched = 0;
    return search(position, tt, /*max_depth*/4, useTT, nodes_searched);
}

} // ~namespace lesschess
