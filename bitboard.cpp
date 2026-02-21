#include "bitboard.hh"
#include <iostream>
#include "board.hh"

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