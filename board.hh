#pragma once

#include "bitboard.hh"
#include <iostream>
#include <algorithm>
#include <cassert>

enum SetPieceSwitch {
    ADD_PIECE, REMOVE_PIECE
};

enum MoveSwitch {
    FORWARD, BACK
};

// irreversible stuff for Copy-Make
struct BoardState {
    CastlingRights Castling;
    int EnPassant;
    ColoredPiece CapturedPiece;
    BoardState* Previous; 

    BoardState() {
        EnPassant = ILLEGAL_SQ;
        Castling = NO_CASTLING;
        Previous = nullptr;
        CapturedPiece = NO_PIECE;
    }

    // lacks 50-moves counter, at the very least
};

class Board {
    BoardState root_state;
public:
    // piece boards 
    Bitboard PieceBB[ALL_PIECES];
    Bitboard ColorBB[COLOR_NB];
    ColoredPiece Mailbox[SQ_AMOUNT];

    Color ActiveColor;
    BoardState* bs;

    int Ply;


    void clear();

    Board() {
        clear();
    }

    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    void print_board_state();

    template<SetPieceSwitch sw>
    inline void set_piece(ColoredPiece p, int sq) { 
        if constexpr (sw == ADD_PIECE) {
            assert(Mailbox[sq] == NO_PIECE);
            Mailbox[sq] = p;
        }
        else {
            assert(Mailbox[sq] == p);
            Mailbox[sq] = NO_PIECE; // sw == REMOVE_PIECE
        }

        Bitboard square_bb = set_bit(0ULL, sq);

        PieceBB[type_of(p)] ^= square_bb;
        ColorBB[color_of(p)] ^= square_bb;

        ColorBB[BOTH] ^=  square_bb;
    }

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

    inline ColoredPiece get_piece_from_sq(int sq) {
        return Mailbox[sq];
    }

    inline CastlingRights get_castling_rights() const{
        return bs->Castling;
    }

    inline int get_ep() const{
        return bs->EnPassant;
    }

    inline ColoredPiece get_captured_piece() const{
        return bs->CapturedPiece;
    }

    template<MoveSwitch sw>
    void move_piece(int sq_from, int sq_to, ColoredPiece p);

    template<MoveSwitch sw>
    void make_promotion(int sq_from, int sq_to, ColoredPiece piece_to);

    template<MoveSwitch sw>
    void castle(bool kingside);

    void make_move(Move& move, BoardState& new_state);
    void unmake_move(Move& move);
};