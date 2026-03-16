#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"
#include "movegen.hh"
#include <sys/time.h>
#include "uci.hh"
#include "evaluation.hh"
#include "search.hh"


int get_time_ms()
{
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

int main() {
  Attacks::init();
  Evaluator::init();

  // Board board = Board{};
  // board.parse_FEN(START_FEN);

  // Move bestmove = Searcher::root_alphabeta(board);
  // cout << square_to_string[bestmove.getFrom()] << square_to_string[bestmove.getTo()];
  UCI::loop();
}