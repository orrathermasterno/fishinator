#pragma once

#include "move.hh"
class Board;

constexpr int MAX_MOVES = 256;


// decomposition from stockfish
enum MoveType: uint8_t {
    QUIET, 
    CAPTURE, 
    QUIET_AND_CAPTURE,
    GET_OUT_OF_CHECK 
};

class MoveList {
public:
    Move Moves[MAX_MOVES], *last;

    MoveList() {
        last = &Moves[0]; 
    }

    const Move* begin() const { return Moves; }
    const Move* end() const { return last; }

    void print_movelist() const;

    template<Piece P, Color ActiveColor, MoveType T>
    void generate_pseudolegals_for(const Board& board, Bitboard targets);

    template<MoveFlag Flag>
    void add_pawn_moves(Bitboard moves_to, Direction dir);

    template<MoveType T, Color ActiveColor>
    void generate_pawn_pseudolegals(const Board& board, Bitboard targets);

    template<Color ActiveColor>
    void generate_castling(const Board& board);

    template<MoveType T, Color ActiveColor>
    void generate_pseudolegals(const Board& board);

    template<MoveType T>
    void generate_pseudolegals(const Board& board);

    // generate_pseudolegals wrapper with a legality check
    void generate_all_legals(const Board& board);

};

template<bool isRoot>
uint64_t Perft(Board& board, int depth);

#ifdef BENCH
uint64_t get_time_ms();
#endif