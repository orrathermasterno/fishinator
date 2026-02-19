#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"

int main() {

  int square;

  for(square = 0; square < 64; square++)
    Attacks::generate_magics<Xorshift, SIZE_FOR_ROOK>(rook, Square(square), Attacks::rook_attacks, Attacks::RookMagics);

  for(square = 0; square < 64; square++)
    Attacks::generate_magics<Xorshift, SIZE_FOR_BISHOP>(bishop, Square(square), Attacks::bishop_attacks, Attacks::BishopMagics);

  int sq = e4;
  Bitboard bb = 0ULL;
  bb = set_bit(bb, sq);
  bb = set_bit(bb, e2);
  bb = set_bit(bb, g4);
  print_bitboard(bb);
  Bitboard get_attack_bb = Attacks::get_rook_attack(bb, Square(sq));

  print_bitboard(get_attack_bb);

  Bitboard corr_sliding_attack = Attacks::generate_sliding_attacks(rook, Square(sq), bb);
  print_bitboard(corr_sliding_attack);

}