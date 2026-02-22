#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"
#include <chrono>

int main() {
  Attacks::init();

  Board board = Board();
  board.set_piece(W_PAWN, e4);
  board.set_piece(W_BISHOP, d5);
  board.set_piece(B_PAWN, f5);
 board.set_piece(B_ROOK, e1);
 board.set_piece(B_PAWN, d2);
 board.set_piece(B_QUEEN, h1);

  board.print_board_state();
  std::cout << Board::is_attacked(h1, Board::ColorBB[BOTH], WHITE) << "\n";
  print_bitboard(Board::attackers_of(h1, Board::ColorBB[BOTH]));
}