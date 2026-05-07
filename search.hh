#pragma once
#include "move.hh"
#include "movegen.hh"
#include "ttable.hh"
#include "board.hh"

constexpr int DEF_DEPTH = 8;
constexpr int MAX_KILLERS = 2;
constexpr int MAX_PLY = 256;

class Searcher {
#ifdef BENCH
    static int nodes;
    static int qnodes;
    static void clear_bench();
#endif
    static Move killers[MAX_PLY][MAX_KILLERS];
    static int history[12][ILLEGAL_SQ]; // indexed by colored piece and sq_to
    static int root_game_ply; // for is_forced_draw(int ply_since_search_root)
    static TTable ttable;
public:
    static void clear();
    static void clear_killers();

    static void add_killer(const Move& move, int ply);
    static void update_history(ColoredPiece moved_piece, int sq_to, int depth);

    static Move root_alphabeta(Board& board, int depth);
    static int alphabeta(Board& board, int alpha, int beta, int depthleft);
    static int quiescence(Board& board, int alpha, int beta);
};