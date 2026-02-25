#pragma once

#include "bitboard.hh"
#include <iostream>

class Board {
public:
    // piece boards 
    Bitboard PieceBB[ALL_PIECES];
    Bitboard ColorBB[COLOR_NB];

    Color ActiveColor;
    CastlingRights Castling;
    Square EnPassant = ILLEGAL_SQ;
    int Ply;
    int FullMove;

    void print_board_state();
    void set_piece(ColoredPiece p, int sq);
    void parse_FEN(const std::string& fenStr);

    inline Bitboard get_white_pawns() const{
        return PieceBB[PAWN] & ColorBB[WHITE];
    }

    template<Piece P>
    inline Bitboard get_colored_piece_bb(Color C) const{
        return PieceBB[P] & ColorBB[C];
    }

    template<Piece P>
    inline Bitboard get_piece_bb() const{
        return PieceBB[P];
    }

    template<typename... Pieces>
    inline Bitboard get_colored_joint_piece_bb(Color C, Pieces... pts) const {
        return ColorBB[C] & (PieceBB[pts] | ...);
    }

    template<typename... Pieces>
    inline Bitboard get_joint_piece_bb(Pieces... pts) const {
        return (PieceBB[pts] | ...);
    }

    inline Piece type_of(ColoredPiece pc) { return Piece(pc & 7); }

    inline Color color_of(ColoredPiece pc) { return Color(pc >> 3); }

    Bitboard attackers_of(int sq, Bitboard blockers) const;
    inline Bitboard enemy_attackers_of(int sq, Bitboard blockers, Color ActiveColor) const {
        return attackers_of(sq, blockers) & ColorBB[ActiveColor ^ 1];
    }
    bool is_attacked(int sq, Bitboard blockers, Color color);

    template<CastlingRights Cr>
    inline bool can_castle() const {
        if constexpr (Cr == WHITE_OO)
            return !(ColorBB[BOTH] & WHITE_OO_BLOCKERS);
            
        else if constexpr (Cr == WHITE_OOO)
            return !(ColorBB[BOTH] & WHITE_OOO_BLOCKERS);
            
        else if constexpr (Cr == BLACK_OO)
            return !(ColorBB[BOTH] & BLACK_OO_BLOCKERS);
            
        else if constexpr (Cr == BLACK_OOO)
            return !(ColorBB[BOTH] & BLACK_OOO_BLOCKERS);
            
        return false;
    }

    inline int get_king_sq(Color color) const { // maybe store king sq as state array idk
        return bit_scan_forward(PieceBB[KING] & ColorBB[color]);
    }
};