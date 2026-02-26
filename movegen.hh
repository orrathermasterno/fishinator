#pragma once

#include "board.hh"

constexpr int MAX_MOVES = 256;

enum MoveType: uint8_t {
    QUIET, CAPTURE, QUIET_AND_CAPTURE,
    GET_OUT_OF_CHECK // this one is supposed to be useful during quiescence search
};

class MoveList {
public:
    Move Moves[MAX_MOVES], *last;

    MoveList() {
        last = &Moves[0]; 
    }

    void print_movelist() const;

    template<Piece P, Color ActiveColor>
    void generate_pseudolegals_for(const Board& board, Bitboard targets);

    template<MoveFlag Flag>
    void add_pawn_moves(Bitboard moves_to, Direction dir);

    template<MoveType T, Color ActiveColor>
    void generate_pawn_pseudolegals(const Board& board, Bitboard targets);

    template<Color ActiveColor>
    void generate_castling(const Board& board);

    template<MoveType T, Color ActiveColor>
    void generate_pseudolegals(const Board& board);
};