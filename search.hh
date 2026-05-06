#pragma once
#include "move.hh"
#include "movegen.hh"

constexpr int DEF_DEPTH = 6;
constexpr int MAX_KILLERS = 2;

class Searcher {
#ifdef BENCH
    static int nodes;
    static int qnodes;
#endif
    static Move killers[MAX_KILLERS][MAX_MOVES];
    static int root_game_ply; // for is_forced_draw(int ply_since_search_root)
public:
    static Move root_alphabeta(Board& board, int depth);
    static int alphabeta(Board& board, int alpha, int beta, int depthleft);
    static int quiescence(Board& board, int alpha, int beta);
};