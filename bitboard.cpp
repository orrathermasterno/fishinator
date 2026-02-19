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

// Bitboard rank_mask(Square sq) {return  C64(Rank1_const) << (sq & 56);}

// Bitboard file_mask(Square sq) {return C64(FileA_const) << get_file(sq);}

// Bitboard diag_mask(Square sq) {
//    int diag = 8*get_file(sq) - (sq & 56); // offset 
//    int nort = -diag & sign_mask(diag);
//    int sout =  diag & sign_mask(-diag);
//    return (MAIN_DIAG >> sout) << nort;
// }

// Bitboard anti_diag_mask(Square sq) {
//    int diag =56 - 8*get_file(sq) - (sq & 56); // offset
//    int nort = -diag & sign_mask(diag);
//    int sout =  diag & sign_mask(-diag);
//    return (MAIN_ANTIDIAG >> sout) << nort;
// }