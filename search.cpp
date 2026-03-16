#include "search.hh"
#include "evaluation.hh"

int DEF_DEPTH = 4;

Move Searcher::root_alphabeta(Board& board) {
    MoveList ml = MoveList();
    Color us = board.ActiveColor;     
    int score;
    Move bestmove;
    int alpha = -INFINITY_VAL;
    int beta = INFINITY_VAL;
    int bestValue = -INFINITY_VAL;

    ml.generate_pseudolegals<QUIET_AND_CAPTURE>(board);

    for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
        Move current_move = *ptr;
        if (!board.legal(current_move)) {
            continue; 
        }

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        score = -alphabeta(board, -beta, -alpha, DEF_DEPTH - 1);

        board.unmake_move(current_move);

        if(score > bestValue) {
            bestValue = score;
            bestmove = current_move;
            
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    return bestmove;
}

// doesn't deal with zero legal moves cases yet 
int Searcher::alphabeta(Board& board, int alpha, int beta, int depth_left) {
    if(depth_left == 0) return quiescence(board, alpha, beta);

    MoveList ml = MoveList();

    Color us = board.ActiveColor;     
    int score;     
    int bestValue = -INFINITY_VAL;       

    ml.generate_pseudolegals<QUIET_AND_CAPTURE>(board);

    for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
        Move current_move = *ptr;

        if (!board.legal(current_move)) {
            continue; 
        }

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        score = -alphabeta(board, -beta, -alpha, depth_left - 1);

        board.unmake_move(current_move);

        if(score > bestValue)
        {
            bestValue = score;
            if(score > alpha)
                alpha = score; // alpha acts like max in MiniMax
        }
        if(score >= beta)
            return bestValue;   // fail soft beta-cutoff
    }
    return bestValue;
}

int Searcher::quiescence(const Board& board, int alpha, int beta) {
    return Evaluator::evaluate(board); // temp
}