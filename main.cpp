#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"
#include <chrono>
#include "movegen.hh"

int main() {
  Attacks::init();

  MoveList ml = MoveList();
  Board board = Board();
  board.parse_FEN("2P1n1N1/3P1P2/8/8/8/8/8/8 b - - 0 1");

  ml.generate_pawn_pseudolegals<CAPTURE, WHITE>(board, 0ULL);
  
  board.print_board_state();
  ml.print_movelist();
}