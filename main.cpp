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
  board.parse_FEN("8/8/8/4pP2/8/8/8/8 w - e6 0 1");

  ml.generate_pseudolegals<QUIET_AND_CAPTURE, WHITE>(board);
  
  board.print_board_state();
  ml.print_movelist();
  BoardState state = BoardState();
  board.make_move(ml.Moves[1], state);

  board.print_board_state();
}