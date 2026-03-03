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


int get_time_ms()
{
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

int main() {
  Attacks::init();
  UCI::loop();
}