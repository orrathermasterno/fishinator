#pragma once

#include "bitboard.hh"

// xorshift* based on http://vigna.di.unimi.it/ftp/papers/xorshift.pdf by Sebastiano Vigna
class Xorshift {
    uint64_t seed;
public:
    Xorshift(uint64_t s) : seed(s == 0 ? 1 : s) {}

    uint64_t rand64() {
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;

        return seed * C64(2685821657736338717);
    }

    uint64_t sparse_rand64() {
        return rand64() & rand64() & rand64();
    }
};



// xoroshiro?