#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"

int main() {

  Attacks::init();

  int sq = e4;
  Bitboard bb = 0ULL;
  bb = set_bit(bb, sq);
  bb = set_bit(bb, e2);
  bb = set_bit(bb, g4);
  bb = set_bit(bb, g6);
  print_bitboard(bb);
  Bitboard get_attack_bb = Attacks::get_bishop_attack(bb, Square(sq));

  print_bitboard(get_attack_bb);

  Bitboard corr_sliding_attack = Attacks::generate_sliding_attacks(bishop, Square(sq), bb);
  print_bitboard(corr_sliding_attack);

}