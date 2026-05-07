#pragma once
#include "bitboard.hh"
#include "board.hh"

constexpr int INFINITY_VAL = 32001;
constexpr int CHECKMATE_VAL = 30000;

typedef uint16_t Score;

class Evaluator {
public:
    static void init();
    static int evaluate(const Board& board);

    static void init_tables();
    static int PeSTO(const Board& board);
};