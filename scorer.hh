#pragma once
#include "move.hh"
#include "movegen.hh"

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

public:
    Scorer(const Board& passed_board, bool is_qs) : board(passed_board), state(SHASH_MOVE), current_move(nullptr), sm_start(SMoves), qsearch(is_qs) {
        if (board.king_in_check()) state = SINIT_OUTOFCHECK;
    };
    Move next_move();

    template<MoveType T>
    ScoredMove* score(const MoveList& mv);

    void insertion_sort();

    Move get_yield() { return yield->m; }
};