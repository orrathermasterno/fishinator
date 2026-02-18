#pragma once 

#include "bitboard.hh"

constexpr uint64_t prng_seed = 6522720ULL; // arbitrary number. recorded for reproducibility

enum SliderPiece: std::uint8_t {
    bishop, rook
};

constexpr int slider_bits[2][64] = {
    { // bishop
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
    },
    // rook
    {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    }
};

class Attacks {
public:
    // Plain Magics approach
    static const Bitboard bishop_attacks[64][512];
    static const Bitboard rookAttacks[64][4096];


    /**********************************\
     ==================================
    
                    Magics
    
    ==================================
    \**********************************/
    static const uint64_t RookMagics[64];
    static const uint64_t BishopMagics[64];

    static Bitboard generate_sliding_attacks(SliderPiece pt, Square sq, Bitboard blockers);
    // generates rook/bishop occupancy masks. generate_sliding_attacks(...) wrapper
    static Bitboard generate_sliding_mask(SliderPiece piece, Square sq);

    // magic multipliers generation now hardcoded into "attacks.cpp"
    template <typename TPrng>
    static uint64_t generate_magics(SliderPiece piece, Square square);

};