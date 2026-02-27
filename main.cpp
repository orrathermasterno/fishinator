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
  board.parse_FEN("8/4k3/8/8/8/8/r6r/R3K2R w KQ - 89 1");

  ml.generate_pseudolegals<QUIET_AND_CAPTURE, WHITE>(board);
  
  board.print_board_state();
  ml.print_movelist();
  BoardState state = BoardState();
  board.make_move(ml.Moves[6], state);

  board.print_board_state();

  board.unmake_move(ml.Moves[6]);
  board.print_board_state();
}