#pragma once 

#include "bitboard.hh"

constexpr uint64_t prng_seed = 6522720ULL; // arbitrary number. recorded for reproducibility

enum SliderPiece: std::uint8_t {
    bishop, rook
};

// used for slider attack tables size init
constexpr size_t SIZE_FOR_BISHOP = 512;
constexpr size_t SIZE_FOR_ROOK   = 4096; // 2^12 -- max possible amount of blocker combinations (for rooks in the corners)


constexpr int slider_bits[2][SQ_AMOUNT] = {
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
    { // rook
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
    static Bitboard bishop_attacks[SQ_AMOUNT][SIZE_FOR_BISHOP]; // 256K
    static Bitboard rook_attacks[SQ_AMOUNT][SIZE_FOR_ROOK]; // 2048K


    /**********************************\
     ==================================
    
                    Magics
    
    ==================================
    \**********************************/
    struct Magic {
        Bitboard mask;
        uint64_t magic; 

        constexpr size_t index(Bitboard occupancy, int bits) {
            Bitboard relevant_occ = occupancy & mask;
            return (size_t)((relevant_occ * magic) >> (SQ_AMOUNT - bits));
        }
    };

    static Magic RookMagics[SQ_AMOUNT];
    static Magic BishopMagics[SQ_AMOUNT];

    static Bitboard generate_sliding_attacks(SliderPiece pt, Square sq, Bitboard blockers);
    // generates rook/bishop occupancy masks. generate_sliding_attacks(...) wrapper
    static Bitboard generate_sliding_mask(SliderPiece piece, Square sq);

    // magic multipliers now hardcoded into "attacks.cpp", this function does not run
    template <typename TPrng, size_t TableSize>
    static void generate_magics(SliderPiece piece, Square sq, Bitboard (&target_table)[SQ_AMOUNT][TableSize], Magic (&target_magics)[SQ_AMOUNT]);


    static inline Bitboard get_rook_attack(Bitboard occ, Square sq) {
        occ &= RookMagics[sq].mask;
        occ *= RookMagics[sq].magic;
        int bits = slider_bits[rook][sq];
        occ >>= 64-bits;
        return rook_attacks[sq][occ];
    }


};