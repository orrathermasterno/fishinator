#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"
#include <chrono>
#include "movegen.hh"

uint64_t Perft(Board& board, int depth)
{
  if (depth == 0) {
    return 1ULL;
  }

  MoveList ml = MoveList();
  uint64_t nodes = 0;

  int us = board.ActiveColor;   
  int them = us ^ 1;               

  if (us == WHITE) {
    ml.generate_pseudolegals<QUIET_AND_CAPTURE, WHITE>(board);
  } else {
    ml.generate_pseudolegals<QUIET_AND_CAPTURE, BLACK>(board);
  }

for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
    Move current_move = *ptr;

    if (!board.legal(current_move)) {
        continue; 
    }

    BoardState state = BoardState(); 
    board.make_move(current_move, state);
    
    nodes += Perft(board, depth - 1);
    
    board.unmake_move(current_move);
  }
  
  return nodes;
}

int main() {
  Attacks::init();

  Board board = Board();
  board.parse_FEN("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");

  MoveList ml = MoveList();
  ml.generate_pseudolegals<QUIET_AND_CAPTURE, BLACK>(board);
  
  board.print_board_state();

  ml.print_movelist();


  // Move move = ml.Moves[8];
  // if(board.legal(move)) {
  //   BoardState state = BoardState();
  //   board.make_move(move, state);

  //   board.print_board_state();

  //   board.unmake_move(move);
  //   board.print_board_state();
  // }
  // else std::cout << "\nIllegal\n";

  uint64_t smth = Perft(board, 5);
      std::cout << smth << "\n";
}