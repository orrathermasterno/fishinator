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
    Attacks() = delete;

    // Plain Magics approach
    static Bitboard bishop_attacks[SQ_AMOUNT][SIZE_FOR_BISHOP]; // 256K
    static Bitboard rook_attacks[SQ_AMOUNT][SIZE_FOR_ROOK]; // 2048K

    static Bitboard king_attacks[SQ_AMOUNT];
    static Bitboard knight_attacks[SQ_AMOUNT];
    static Bitboard pawn_attacks[BOTH][SQ_AMOUNT];

    // non-attack helper arrays
    static Bitboard betweeen_sq[SQ_AMOUNT][SQ_AMOUNT]; 

public:
    /**********************************\
    ==================================
    
                Non-magics init
    
    ==================================
    \**********************************/
    template<Color C>
    static inline Bitboard generate_pawn_attacks(Bitboard bb){
        return C == WHITE ? shift<NORTH_WEST>(bb) | shift<NORTH_EAST>(bb)
                            : shift<SOUTH_WEST>(bb) | shift<SOUTH_EAST>(bb);
    }

    static inline Bitboard generate_king_attacks(Bitboard bb){
        return shift<NORTH_WEST>(bb) | shift<NORTH>(bb) | shift<NORTH_EAST>(bb) | shift<SOUTH_EAST>(bb) | 
            shift<SOUTH>(bb) | shift<SOUTH_WEST>(bb) | shift<EAST>(bb) | shift<WEST>(bb);
    }

    //         noNoWe    noNoEa
    //             +15  +17
    //              |     |
    // noWeWe  +6 __|     |__+10  noEaEa
    //               \   /
    //                >0<
    //            __ /   \ __
    // soWeWe -10   |     |   -6  soEaEa
    //              |     |
    //             -17  -15
    //         soSoWe    soSoEa
    static inline Bitboard generate_knight_attacks(Bitboard bb){
        return shift<static_cast<Direction>(NORTH + NORTH + WEST)>(bb) |
        shift<static_cast<Direction>(NORTH + NORTH + EAST)>(bb) |  
        shift<static_cast<Direction>(SOUTH + SOUTH + WEST)>(bb) |
        shift<static_cast<Direction>(SOUTH + SOUTH + EAST)>(bb) | 
        shift<static_cast<Direction>(WEST + WEST + NORTH)>(bb) | 
        shift<static_cast<Direction>(WEST + WEST + SOUTH)>(bb) | 
        shift<static_cast<Direction>(EAST + EAST + NORTH)>(bb) | 
        shift<static_cast<Direction>(EAST + EAST + SOUTH)>(bb);
    }

    //getters
    static inline Bitboard get_knight_attack(int sq) {
        return knight_attacks[sq];
    }
    static inline Bitboard get_king_attack(int sq) {
        return king_attacks[sq];
    }
    static inline Bitboard get_pawn_attack(int sq, Color c) {
        return pawn_attacks[c][sq];
    }
    /**********************************\
     ==================================
    
                Magics init
    
    ==================================
    \**********************************/
    struct Magic {
        Bitboard mask;
        uint64_t magic; 

        constexpr size_t index(Bitboard blockers, int bits) {
            Bitboard relevant_blockers = blockers & mask;
            return (size_t)((relevant_blockers * magic) >> (SQ_AMOUNT - bits));
        }
    };

private:
    static Magic RookMagics[SQ_AMOUNT];
    static Magic BishopMagics[SQ_AMOUNT];

public:
    static Bitboard generate_sliding_attacks(SliderPiece pt, int sq, Bitboard blockers);
    // generates rook/bishop occupancy masks. generate_sliding_attacks(...) wrapper
    static Bitboard generate_sliding_mask(SliderPiece piece, int sq);

    // runs every startup to init slider tables; totally deterministic unless seed changed
    template <typename TPrng, size_t TableSize>
    static void generate_magics(SliderPiece piece, int sq, Bitboard (&target_table)[SQ_AMOUNT][TableSize], Magic (&target_magics)[SQ_AMOUNT]);

    // slider getters
    static inline Bitboard get_rook_attack(Bitboard blockers, int sq) {
        return rook_attacks[sq][RookMagics[sq].index(blockers, slider_bits[rook][sq])];
    }
    static inline Bitboard get_bishop_attack(Bitboard blockers, int sq) {
        return bishop_attacks[sq][BishopMagics[sq].index(blockers, slider_bits[bishop][sq])];
    }
    static inline Bitboard get_queen_attack(Bitboard blockers, int sq) {
        return get_bishop_attack(blockers, sq) | get_rook_attack(blockers, sq);
    }

    static inline Bitboard get_between_sq_bb(int sq1, int sq2) {
        return betweeen_sq[sq1][sq2];
    }

    /**********************************\
    ==================================
    
            Everything init
    
    ==================================
    \**********************************/
    static void init_between_bb();
    static void init();

    template<Piece P>
    static inline Bitboard get_attack_of(int sq, Bitboard blockers=0ULL) {
        switch(P) {
            case KNIGHT: return knight_attacks[sq];
            case KING: return king_attacks[sq];
            case BISHOP: return get_bishop_attack(blockers, sq);
            case ROOK: return get_rook_attack(blockers, sq);
            case QUEEN: return get_queen_attack(blockers, sq);
            default: return 0ULL;
        }
    }
};