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
  board.parse_FEN("8/8/2P3p1/8/4B3/8/8/1p5P w - - 0 1");
  Bitboard knight_quiet_targets = ~board.ColorBB[BOTH];
  Bitboard knight_capture_targets = board.ColorBB[BLACK];
  Bitboard bishop_quiet_targets = ~board.ColorBB[BOTH];
  ml.generate_pseudolegals_for<BISHOP>(board, knight_capture_targets, WHITE);
  ml.generate_pseudolegals_for<BISHOP>(board, bishop_quiet_targets, WHITE);
  board.print_board_state();
  ml.print_movelist();
}