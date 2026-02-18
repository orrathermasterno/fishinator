#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"

int main() {

  int square;

  printf("const uint64 RMagic[64] = {\n");
  for(square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", Attacks::generate_magics<Xorshift>(rook, Square(square)));
  printf("};\n\n");

  printf("const uint64 BMagic[64] = {\n");
  for(square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", Attacks::generate_magics<Xorshift>(bishop, Square(square)));
  printf("};\n\n");

}