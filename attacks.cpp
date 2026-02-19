#include "attacks.hh"
#include "prng.hh"

// const uint64_t RookMagics[SQ_AMOUNT] = {
//   0x80004008668030ULL,
//   0x1040004010002004ULL,
//   0x4100200100104008ULL,
//   0x100082004100300ULL,
//   0x900104448000300ULL,
//   0x500180400110002ULL,
//   0x200208104084200ULL,
//   0xa080004900002080ULL,
//   0x4004800040008820ULL,
//   0x2400040201000ULL,
//   0xb002001a00402081ULL,
//   0x8002803002080080ULL,
//   0x808004000800ULL,
//   0x282a004408220030ULL,
//   0x1000100040200ULL,
//   0x2001000d40920100ULL,
//   0x80004000402000ULL,
//   0x20810030400100ULL,
//   0x100110020004104ULL,
//   0x2104090010010220ULL,
//   0x8041010008001004ULL,
//   0x2808004000200ULL,
//   0x6828040010122118ULL,
//   0x82600048120c4ULL,
//   0x10400280108420ULL,
//   0x2005062100400680ULL,
//   0x2850001080200086ULL,
//   0x80204200120008ULL,
//   0x58000900100500ULL,
//   0x40080020080ULL,
//   0x1000100040200ULL,
//   0x18004600040283ULL,
//   0x400090800020ULL,
//   0x1248400081002100ULL,
//   0x2060040010100201ULL,
//   0x800900109002100ULL,
//   0x4400240082800800ULL,
//   0x4840800400800200ULL,
//   0x9018224004810ULL,
//   0x41000041000082ULL,
//   0x180804000208004ULL,
//   0x4160100040014020ULL,
//   0xa0c0108022020040ULL,
//   0x80204200120008ULL,
//   0x3302001020060008ULL,
//   0x8020004008080ULL,
//   0x902111068040002ULL,
//   0x800a5494020011ULL,
//   0x200d026092004200ULL,
//   0x200d026092004200ULL,
//   0x4110843200100ULL,
//   0x40200900100100ULL,
//   0x9100080010050100ULL,
//   0x40080020080ULL,
//   0x2320021018310400ULL,
//   0x2001000d40920100ULL,
//   0x4110801020420102ULL,
//   0x100804000102101ULL,
//   0x6810104009002001ULL,
//   0x200500081001ULL,
//   0x2083001028008407ULL,
//   0x4022002804102162ULL,
//   0x8024810008104ULL,
//   0x4004020840112ULL,
// };

// const uint64_t BishopMagics[SQ_AMOUNT] = {
//   0x8510200884808901ULL,
//   0x5188021840410000ULL,
//   0x1290013204200600ULL,
//   0x4410020020002ULL,
//   0xa80202114a481200ULL,
//   0x1010840100940ULL,
//   0x1009010080000ULL,
//   0x2400920304224004ULL,
//   0x221040c202440100ULL,
//   0x8100c048030ULL,
//   0x40040104210820ULL,
//   0x1104241042148010ULL,
//   0x40020210801048ULL,
//   0x4000008804400140ULL,
//   0x1001020104824000ULL,
//   0x4804018084882101ULL,
//   0x20100620440101ULL,
//   0x1008a049850900ULL,
//   0x9000820a820108ULL,
//   0x4050804101000ULL,
//   0x2404010211040005ULL,
//   0x6011800900600228ULL,
//   0x32104301304200ULL,
//   0x4428300501011040ULL,
//   0x40094200089014a0ULL,
//   0x8600028014900ULL,
//   0xc405008020040ULL,
//   0x20208040c004008ULL,
//   0x4200404104010041ULL,
//   0x8202002020100ULL,
//   0x2044c20105051000ULL,
//   0x4080860a2104ULL,
//   0x1008294004481a23ULL,
//   0x141082002090100ULL,
//   0x10c060809410041ULL,
//   0x8400200800090106ULL,
//   0x20154040c00c0100ULL,
//   0x1890091601204044ULL,
//   0x504107010a2a80ULL,
//   0x220c8911008a0199ULL,
//   0x84500800c004ULL,
//   0x40480c50402402ULL,
//   0x40000c0048000400ULL,
//   0x804041c208000085ULL,
//   0x5c12880100400400ULL,
//   0x40405822810000a0ULL,
//   0x210040104080070ULL,
//   0x4802040040202dULL,
//   0x1009010080000ULL,
//   0x12008404820101ULL,
//   0x1008120209443002ULL,
//   0x8000104880084ULL,
//   0x89000a002048830ULL,
//   0x5200418408008300ULL,
//   0x8080818084001ULL,
//   0x5188021840410000ULL,
//   0x2400920304224004ULL,
//   0x4804018084882101ULL,
//   0x40081810504c1000ULL,
//   0x60204400820a800ULL,
//   0x410000289210100ULL,
//   0xc040081020010104ULL,
//   0x221040c202440100ULL,
//   0x8510200884808901ULL,
// };
Bitboard Attacks::rook_attacks[SQ_AMOUNT][SIZE_FOR_ROOK];
Bitboard Attacks::bishop_attacks[SQ_AMOUNT][SIZE_FOR_BISHOP];

Attacks::Magic Attacks::RookMagics[SQ_AMOUNT];
Attacks::Magic Attacks::BishopMagics[SQ_AMOUNT];

Bitboard Attacks::generate_sliding_attacks(SliderPiece piece, Square sq, Bitboard blockers) {
    Bitboard  attacks             = 0;
    Direction RookDirections[4]   = {NORTH, SOUTH, EAST, WEST};
    Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    Bitboard next_attack_bit = 0;

    for(Direction d : (piece == rook ? RookDirections : BishopDirections)) {

        Square curr_sq = sq;

        while(is_safe_shift(curr_sq, d)) {
            curr_sq = Square(curr_sq + d);
            next_attack_bit = set_bit(0ULL, curr_sq);

            attacks |= next_attack_bit;

            if (next_attack_bit & blockers) {
                break;
            }
        }
    }
    return attacks;
}

Bitboard Attacks::generate_sliding_mask(SliderPiece piece, Square sq) {
    Bitboard edges = ((Rank1_const | Rank8_const) & ~get_rank_bb(get_rank(sq))) | ((FileA_const | FileH_const) & ~get_file_bb(get_file(sq)));
    return generate_sliding_attacks(piece, sq, 0ULL) & ~edges;
}

template <typename TPrng, size_t TableSize>
void Attacks::generate_magics(SliderPiece piece, Square sq, Bitboard (&target_table)[SQ_AMOUNT][TableSize], Magic (&target_magics)[SQ_AMOUNT]
    ){
    Bitboard blockers[SIZE_FOR_ROOK]; // occupancy mask power set 
    Bitboard reference[SIZE_FOR_ROOK]; // holds attack bbs corresponding to blockers at same index
    int used[SIZE_FOR_ROOK] = {}, count = 0;
    Bitboard mask = generate_sliding_mask(piece, sq); target_magics[sq].mask = mask;
    TPrng prng{prng_seed};
    uint64_t magic_number;
    bool fail; size_t index;

    // generate all subsets of a mask using Carry-Rippler trick
    Bitboard curr_subset = 0ULL; int blocker_index = 0;
    do {
        blockers[blocker_index] = curr_subset;
        reference[blocker_index] = generate_sliding_attacks(piece, sq, curr_subset);
        blocker_index++;

        curr_subset = (curr_subset - mask) & mask;
    } while(curr_subset);

    for (size_t attempt = 0; attempt < 100000000; attempt++) {
        magic_number = prng.sparse_rand64();
        if(population_count((mask * magic_number) & 0xFF00000000000000ULL) < 6) continue;

        fail = false;
        count++;
        for (int i = 0; !fail && i < (1 << slider_bits[piece][sq]); i ++) {
            index = (size_t)((blockers[i] * magic_number) >> (64 - slider_bits[piece][sq]));
            if (used[index] < count){
                used[index] = count;
                target_table[sq][index] = reference[i];
            }
            else if (target_table[sq][index] != reference[i]) fail = true;
        }
        if(!fail) {
            target_magics[sq].magic = magic_number;
            return;
        }
    }
    return;
}

// explicit xorshift instantiation
template void Attacks::generate_magics<Xorshift, SIZE_FOR_BISHOP>(
    SliderPiece, 
    Square, 
    Bitboard (&)[SQ_AMOUNT][SIZE_FOR_BISHOP], 
    Magic (&)[SQ_AMOUNT]
);

template void Attacks::generate_magics<Xorshift, SIZE_FOR_ROOK>(
    SliderPiece, 
    Square, 
    Bitboard (&)[SQ_AMOUNT][SIZE_FOR_ROOK], 
    Magic (&)[SQ_AMOUNT]
);