#pragma once
#include "bitboard.hh"
#include <string>

enum MoveFlag: uint8_t {
    QUIET_F, DOUBLE_PAWN_PUSH_F, KING_CASTLE_F, QUEEN_CASTLE_F, CAPTURES_F, EP_CAPTURE_F, // CAPTURES_F is only set for pawns
    KNIGHT_PROM_F = 8, BISHOP_PROM_F, ROOK_PROM_F, QUEEN_PROM_F, 
    KNIGHT_PROM_CAPTURE_F, BISHOP_PROM_CAPTURE_F, ROOK_PROM_CAPTURE_F, QUEEN_PROM_CAPTURE_F 
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
   
    static constexpr Move empty_move() { return Move(); }
    constexpr bool is_empty() { return data == 0; }

    void operator=(Move a) {data = a.data;}
    bool operator==(Move a) {return data == a.data;}

    constexpr int getTo() const {return data & 0x3f;}
    constexpr int getFrom() const {return (data >> 6) & 0x3f;}
    constexpr int getFlags() const {return (data >> 12) & 0x0f;}

    constexpr void setTo(unsigned int to) {data &= ~0x3f; data |= to & 0x3f;}
    constexpr void setFrom(unsigned int from) {data &= ~0xfc0; data |= (from & 0x3f) << 6;}

    constexpr bool is_pawn_capture() const { return getFlags() & CAPTURES_F; }
    constexpr bool is_ep() const { return getFlags() == EP_CAPTURE_F; }
    constexpr bool is_double_push() const { return getFlags() == DOUBLE_PAWN_PUSH_F; }

    constexpr bool is_king_castle() const { return getFlags() == KING_CASTLE_F; }
    constexpr bool is_queen_castle() const { return getFlags() == QUEEN_CASTLE_F; }
    constexpr bool is_castle() const { return is_queen_castle() || is_king_castle(); }

    constexpr bool is_promotion() const { return getFlags() & 8; }

    constexpr ColoredPiece get_promotion_type(int ActiveColor) const {
        Piece pt = Piece((getFlags() & 3) + KNIGHT);
        
        return make_colored_piece(Color(ActiveColor), pt);
    }

    inline std::string move_to_str(int ActiveColor) const {
        std::string result = std::string(square_to_string[getFrom()]) + 
                        square_to_string[getTo()];
        
        if (is_promotion()) {
            int piece_type = type_of(get_promotion_type(ActiveColor));
            piece_type += 8;

            result += EncodedPieces[piece_type];
        }

        return result;
    }

   
};

constexpr uint8_t CastlingMasks[SQ_AMOUNT] = {
    13, 15, 15, 15, 12, 15, 15, 14, // a1, b1, c1, d1, e1, f1, g1, h1
    15, 15, 15, 15, 15, 15, 15, 15, // a2, b2, c2, d2, e2, f2, g2, h2
    15, 15, 15, 15, 15, 15, 15, 15, // a3, b3, c3, d3, e3, f3, g3, h3
    15, 15, 15, 15, 15, 15, 15, 15, // a4, b4, c4, d4, e4, f4, g4, h4
    15, 15, 15, 15, 15, 15, 15, 15, // a5, b5, c5, d5, e5, f5, g5, h5
    15, 15, 15, 15, 15, 15, 15, 15, // a6, b6, c6, d6, e6, f6, g6, h6
    15, 15, 15, 15, 15, 15, 15, 15, // a7, b7, c7, d7, e7, f7, g7, h7
     7, 15, 15, 15,  3, 15, 15, 11  // a8, b8, c8, d8, e8, f8, g8, h8
};