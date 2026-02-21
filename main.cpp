#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"

int main() {
  Attacks::init();

  std::string fen = "8/8/8/4p1K1/2k1P3/8/8/8 b - - 0 1"; 
  Board board = Board();
  board.parse_FEN(fen);
  board.print_board_state();
}