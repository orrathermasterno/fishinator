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

int main() {
  Attacks::init();
  Evaluator::init();

  // // Board board = Board{};
  // // board.parse_FEN("8/8/8/8/8/8/7k/QQQQQQQQ b - - 0 1");

  UCI::loop();
}