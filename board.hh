#pragma once

#include "bitboard.hh"
#include <iostream>

class Board {
public:
    // piece boards 
    static Bitboard PieceBB[ALL_PIECES];
    static Bitboard ColorBB[INVALID_COLOR];

    inline static Color ActiveColor;
    inline static CastlingRights Castling;
    inline static Square EnPassant = ILLEGAL_SQ;
    inline static int Ply;
    inline static int FullMove;

    void print_board_state();
    void set_piece(ColoredPiece p, int sq);
    void parse_FEN(const std::string& fenStr);

    static inline Bitboard get_white_pawns() {
        return PieceBB[PAWN] & ColorBB[WHITE];
    }

    template<Piece P>
    static inline Bitboard get_colored_piece_bb(Color C) {
        return PieceBB[P] & ColorBB[C];
    }

    template<Piece P>
    static inline Bitboard get_piece_bb() {
        return PieceBB[P];
    }

    template<typename... Pieces>
    static inline Bitboard get_colored_joint_piece_bb(Color C, Pieces... pts) {
        return ColorBB[C] & (PieceBB[pts] | ...);
    }

    template<typename... Pieces>
    static inline Bitboard get_joint_piece_bb(Pieces... pts) {
        return (PieceBB[pts] | ...);
    }

    static inline Piece type_of(ColoredPiece pc) { return Piece(pc & 7); }

    static inline Color color_of(ColoredPiece pc) { return Color(pc >> 3); }

    static Bitboard attackers_of(int sq, Bitboard blockers);
    static bool is_attacked(int sq, Bitboard blockers, Color color);
};