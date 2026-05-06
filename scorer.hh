#pragma once
#include "move.hh"
#include "movegen.hh"

constexpr int MAX_HISTORY = 16384;

enum Stage : uint8_t {
    SHASH_MOVE, 

        SINIT_CAPTURES,
    SCAPTURES,

        SINIT_QUIETS,
    SQUIETS, 

        SINIT_OUTOFCHECK,
    SOUTOFCHECK
};
constexpr Stage& operator++(Stage& s) {
    s = static_cast<Stage>(static_cast<uint8_t>(s) + 1);
    return s;
}

class ScoredMove {
public:
    Move m;
    int score;
};

// state machine
class Scorer {
    bool qsearch;
    const Board& board;
    Stage state;
    const Move* current_move;

    ScoredMove SMoves[MAX_MOVES], *sm_start, *sm_end, *yield;

    const Move* node_killers;
    const int (&history)[12][64];

public:
    Scorer(const Board& passed_board, bool is_qs, const Move* killers_for_ply, const int (&hist)[12][64]) : board(passed_board), state(SHASH_MOVE),
     current_move(nullptr), sm_start(SMoves), qsearch(is_qs), node_killers(killers_for_ply), history(hist) {
        if (board.king_in_check()) state = SINIT_OUTOFCHECK;
    };
    Move next_move();

    template<MoveType T>
    ScoredMove* score(const MoveList& mv);

    void insertion_sort();

    Move get_yield() { return yield->m; }
};