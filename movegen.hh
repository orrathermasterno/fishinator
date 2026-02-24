#pragma once

#include "board.hh"

constexpr int MAX_MOVES = 256;

enum MoveFlag: uint8_t {
    QUIET_F, DOUBLE_PAWN_PUSH_F, KING_CASTLE_F, QUEEN_CASTLE_F, CAPTURES_F, EP_CAPTURE_F, 
    KNIGHT_PROM_F = 8, BISHOP_PROM_F, ROOK_PROM_F, QUEEN_PROM_F, 
    KNIGHT_PROM_CAPTURE_F, BISHOP_PROM_CAPTURE_F, ROOK_PROM_CAPTURE_F, QUEEN_PROM_CAPTURE_F 
};

enum MoveType: uint8_t {
    QUIET, CAPTURE, QUIET_AND_CAPTURE,
    GET_OUT_OF_CHECK // this one is supposed to be useful during quiescence search
};

class Move {
protected:
    uint16_t data;
public:
    constexpr Move() : data(0) {}
    Move(int from, int to, int flags) {
      data = ((flags & 0xf)<<12) | ((from & 0x3f)<<6) | (to & 0x3f);
    }

    Move(int from, int to) {
        data = ((from & 0x3f) << 6) | (to & 0x3f);
    }
   
    void operator=(Move a) {data = a.data;}

    constexpr int getTo() const {return data & 0x3f;}
    constexpr int getFrom() const {return (data >> 6) & 0x3f;}
    constexpr int getFlags() const {return (data >> 12) & 0x0f;}

    constexpr void setTo(unsigned int to) {data &= ~0x3f; data |= to & 0x3f;}
    constexpr void setFrom(unsigned int from) {data &= ~0xfc0; data |= (from & 0x3f) << 6;}
};

class MoveList {
protected:
    Move Moves[MAX_MOVES], *last;
public:
    MoveList() {
        last = &Moves[0]; 
    }

    void print_movelist() const;

    template<Piece P>
    void generate_pseudolegals_for(const Board& board, Bitboard targets, Color ActiveColor);

    template<MoveFlag Flag>
    void add_pawn_moves(Bitboard moves_to, Direction dir);

    template<MoveType T, Color ActiveColor>
    void generate_pawn_pseudolegals(const Board& board, Bitboard targets);

};