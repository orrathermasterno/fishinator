#pragma once
#include "move.hh"
#include "movegen.hh"

class Searcher {
public:
    static Move root_alphabeta(Board& board);
    static int alphabeta(Board& board, int alpha, int beta, int depthleft);
    static int quiescence(const Board& board, int alpha, int beta);
};