#include "search.h"
#include "evaluate.h"
#include <array>
#include <cassert>
#include <functional>
#include <vector> // TEMP TEMP
#include <cstring>

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
// using Moves = std::vector<Move>; // TODO: change this back to std::array<Move, 256>?
using Moves = std::array<Move, 128>;

template <int N>
void PrimaryVariation<N>::dump() const
{
    // for (const Move& move : *this) {
    for (int i = 0; i < count; ++i) {
        const Move& move = moves[i];
        std::cout << move.to_long_algebraic_string() << " ";
    }
}

int side_relative_score(Position& position, int score) noexcept {
    return position.white_to_move() ? score : -score;
}

void copy_line(Line& dst, Line& src, Move move, int score) noexcept
{
    memcpy(&dst.moves[1], &src.moves[0], src.count * sizeof(Move));
    dst.moves[0] = move;
    dst.score = score;
    dst.count = src.count + 1;
}

// alpha = lower bound on maximizer's score
// beta  = upper bound on minimizer's score

int quiescence(Position& position, int alpha, int beta, SearchMetrics& metrics, Line& pline)
{
    assert(beta >= alpha);
    metrics.qnodes++;

    int score = side_relative_score(position, evaluate(position));
    if (score >= beta) { // failed hard beta-cutoff
        metrics.beta_cutoffs++;
        pline.count = 0;
        return beta;
    }
    if (score > alpha) {
        alpha = score;
    }

    Line line;
    Savepos sp;
    Moves moves; // TODO: tune this number, likely can be lower
    int nmoves = position.generate_captures(&moves[0]);
    for (int i = 0; i < nmoves; ++i) {
        position.make_move(sp, moves[i]);
        metrics.pv.push(moves[i]);
        score = -quiescence(position, -beta, -alpha, metrics, line);
        metrics.pv.pop();
        position.undo_move(sp, moves[i]);
        if (score >= beta) { // failed hard beta-cutoff
            ++metrics.beta_cutoffs;
            return beta;
        }
        if (score > alpha) {
            ++metrics.alpha_cutoffs;
            alpha = score;
            copy_line(pline, line, moves[i], score);
        }
    }

    return alpha;
}

std::ostream& operator<<(std::ostream& os, const SearchMetrics& metrics)
{
    os << "SearchMetrics\n=========================\n"
        << "Alpha Cutoffs   : " << metrics.alpha_cutoffs << "\n"
        << "Beta Cutoffs    : " << metrics.beta_cutoffs << "\n"
        << "Nodes Searched  : " << metrics.nodes << "\n"
        << "Leaf Nodes      : " << metrics.lnodes << "\n"
        << "Quiescence Nodes: " << metrics.qnodes << "\n"
        << "=========================\n";
    return os;
}

int move_sort_value(const Position& position, Move move) noexcept
{
    auto pc = position.piece_on_square(move.to());
    int value = 0;
    value += !pc.empty()         ? BasePieceValues[pc.kind()]        : 0;
    value += move.is_promotion() ? BasePieceValues[move.promotion()] : 0;
    return value;
}

template <class MoveIter>
void sort_moves(const Position& position, MoveIter first, MoveIter last) noexcept
{
    // sort captures and promotions to front of list
    std::stable_sort(
        first, last,
        [&](Move m1, Move m2) {
            return move_sort_value(position, m1) > move_sort_value(position, m2);
        }
    );
}

int negamax(Position& position, int alpha, int beta, int depth, TT* tt,
        SearchMetrics& metrics, Line& pline)
{
    metrics.nodes++;
    assert(beta >= alpha);

    Moves moves;
    Savepos sp;
    int value, score;
    int alpha_orig = alpha;
    auto* tt_entry = tt ? &tt->find(position.zobrist_hash()) : nullptr;
    if (tt_entry && tt_entry->is_valid() && tt_entry->depth >= depth) {
        tt->record_hit();

        if (tt_entry->is_exact()) {
            return tt_entry->value;
        } else if (tt_entry->is_lower()) {
            alpha = std::max(alpha, tt_entry->value);
        } else if (tt_entry->is_upper()) {
            beta = std::min(beta, tt_entry->value);
        } else {
            assert(0 && "invalid tt entry");
        }

        if (alpha >= beta) {
            metrics.beta_cutoffs++;
            return tt_entry->value;
        }
    }

    if (depth == 0) {
        value = quiescence(position, alpha, beta, metrics, pline);
        // value = side_relative_score(position, evaluate(position));
        metrics.lnodes++;
    } else if (position.fifty_move_rule_moves() >= 50) {
        // TODO: check for 3-move repetition
        value = FIFTY_MOVE_RULE_DRAW;
    } else {
        int nmoves = position.generate_legal_moves(&moves[0]);
        if (nmoves == 0) {
            // TODO: cache `checkers` from generate_legal_moves() so we can check if mate or stalemate?
            value = position.in_check(position.color_to_move()) ? -CHECKMATE : STALEMATE;
        } else {
            Line line;
            sort_moves(position, &moves[0], &moves[nmoves]);
            value = -MAX_SCORE;
            for (int i = 0; i < nmoves; ++i) {
                position.make_move(sp, moves[i]);
                metrics.pv.push(moves[i]);
                score = -negamax(position, -beta, -alpha, depth - 1, tt, metrics, line);
                metrics.pv.pop();
                position.undo_move(sp, moves[i]);
                value = std::max(value, score);
                if (value >= beta) {
                    metrics.beta_cutoffs++;
                    break;
                }
                if (value > alpha) {
                    metrics.alpha_cutoffs++;
                    alpha = value;
                    copy_line(pline, line, moves[i], value);
                }
            }
        }
    }

    if (tt_entry) {
        tt_entry->value = value;
        if (value <= alpha_orig) {
            tt_entry->flag = TT::Flag::kUpper;
        } else if (value >= beta) {
            tt_entry->flag = TT::Flag::kLower;
        } else {
            tt_entry->flag = TT::Flag::kExact;
        }
        tt_entry->depth = depth;
    }

    return value;
}

SearchResult search(Position& position, TT* tt, int depth, SearchMetrics& metrics, Line& bestline)
{
    Savepos sp;
    Moves moves;
    int nmoves = position.generate_legal_moves(&moves[0]);
    sort_moves(position, &moves[0], &moves[nmoves]);
    memset(&bestline, 0, sizeof(bestline));
    int bestmove = -1;
    int bestscore = -MAX_SCORE;
    int alpha = -MAX_SCORE; // -10;
    int beta  =  MAX_SCORE; // 10;
    for (int i = 0; i < nmoves; ++i) {
        Line line;
        position.make_move(sp, moves[i]);
        metrics.pv.push(moves[i]);
        int score = -negamax(position, alpha, beta, depth - 1, tt, metrics, line);
        metrics.pv.pop();
        position.undo_move(sp, moves[i]);
        if (score > bestscore) {
            bestscore = score;
            bestmove = i;
            copy_line(bestline, line, moves[i], score);
        }
    }

    assert(bestmove != -1);
    bestscore = position.white_to_move() ? bestscore : -bestscore;

    // TEMP TEMP
    std::cout << "\nEnd of search:\nMove : " << moves[bestmove].to_long_algebraic_string() << "\n"
        << "Score: " << bestscore << "\n"
        << "Principal Variation: ";
    for (int i = 0; i < bestline.count; ++i) {
        std::cout << bestline.moves[i].to_long_algebraic_string() << " ";
    }
    std::cout << "\nbestline.score = " << bestline.score << "\n\n";

    return {moves[bestmove], bestscore};
}

SearchResult easy_search(Position& position, bool useTT)
{
    SearchMetrics metrics;
    Line bestline;
    TT table;
    TT* tt = useTT ? &table : nullptr;
    return search(position, tt, /*max_depth*/4, metrics, bestline);
}

} // ~namespace lesschess
