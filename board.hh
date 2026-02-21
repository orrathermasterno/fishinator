#pragma once

#include "bitboard.hh"
#include <iostream>

class Board {
public:
    // piece boards 
    static Bitboard PieceBB[ALL_PIECES];
    static Bitboard ColorBB[BOTH];

    inline static Color ActiveColor;
    inline static CastlingRights Castling;
    inline static Square EnPassant = ILLEGAL_SQ;
    inline static int Ply;
    inline static int FullMove;

    void print_board_state();
    void set_piece(ColouredPiece p, int sq);
    void parse_FEN(const std::string& fenStr);
};