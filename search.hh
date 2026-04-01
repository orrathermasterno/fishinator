#pragma once
#include "move.hh"
#include "movegen.hh"

constexpr int DEF_DEPTH = 6;

class Searcher {
public:
    static Move root_alphabeta(Board& board, int depth);
    static int alphabeta(Board& board, int alpha, int beta, int depthleft);
    static int quiescence(const Board& board, int alpha, int beta);
};