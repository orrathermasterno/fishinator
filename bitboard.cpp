#include "bitboard.hh"
#include <iostream>

void print_bitboard(Bitboard bb) {
    int square = 0;
    for (int rank = RANK_8; rank >= RANK_1; --rank) {
        std::cout << (rank + 1) << "  ";
        for (int file = FILE_A; file <= FILE_H; ++file) {
            square = rank * 8 + file;
            
            if (get_bit(bb, square)) {
                std::cout << "X ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

Bitboard rank_mask(Square sq) {return  C64(Rank1_const) << (sq & 56);}

Bitboard file_mask(Square sq) {return C64(FileA_const) << get_file(sq);}

Bitboard diag_mask(Square sq) {
   int diag = 8*get_file(sq) - (sq & 56); // offset 
   int nort = -diag & sign_mask(diag);
   int sout =  diag & sign_mask(-diag);
   return (MAIN_DIAG >> sout) << nort;
}

Bitboard anti_diag_mask(Square sq) {
   int diag =56 - 8*get_file(sq) - (sq & 56); // offset
   int nort = -diag & sign_mask(diag);
   int sout =  diag & sign_mask(-diag);
   return (MAIN_ANTIDIAG >> sout) << nort;
}

// Bitboard generate_sliding_attacks(Piece piece, Square sq, Bitboard blockers) {
//     Bitboard  attacks             = 0;
//     Direction RookDirections[4]   = {NORTH, SOUTH, EAST, WEST};
//     Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

//     Bitboard next_attack_bit = 0;

//     for(Direction d : (piece == ROOK ? RookDirections : BishopDirections)) {

//         Square curr_sq = sq;

//         while(is_safe_shift(curr_sq, d)) {
//             curr_sq = Square(curr_sq + d);
//             next_attack_bit = set_bit(0ULL, curr_sq);

//             attacks |= next_attack_bit;

//             if (next_attack_bit & blockers) {
//                 break;
//             }
//         }
//     }
//     return attacks;
// }

// Bitboard generate_sliding_mask(Piece piece, Square sq) {
//     Bitboard edges = ((Rank1_const | Rank8_const) & ~get_rank_bb(get_rank(sq))) | ((FileA_const | FileH_const) & ~get_file_bb(get_file(sq)));
//     return generate_sliding_attacks(piece, sq, 0ULL) & ~edges;
// }

// void set_occupancies(Bitboard mask) {
//     Bitboard curr_subset = 0ULL;

//     do {
//         // do smth with curr_subset not sure yet. maybe save into arr maybe repurpose whole func to calc stuff directly

//         curr_subset = (curr_subset - mask) & mask;
//     } while(curr_subset);
// }