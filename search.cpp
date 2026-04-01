#include "search.hh"
#include "evaluation.hh"

Move Searcher::root_alphabeta(Board& board, int depth) {
    MoveList ml = MoveList();
    Color us = board.ActiveColor;     
    int score;
    Move bestmove;
    int alpha = -INFINITY_VAL;
    int beta = INFINITY_VAL;
    int bestValue = -INFINITY_VAL;

    ml.generate_all_legals(board);

    for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
        Move current_move = *ptr;

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        score = -alphabeta(board, -beta, -alpha, depth - 1);

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

int Searcher::alphabeta(Board& board, int alpha, int beta, int depth_left) {
    if(depth_left == 0) return quiescence(board, alpha, beta);

    MoveList ml = MoveList();

    Color us = board.ActiveColor;     
    int score;     
    int bestValue = -INFINITY_VAL;       

    ml.generate_all_legals(board);

    int legals = 0;

    for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
        Move current_move = *ptr;

        legals++;

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


    if (!legals) {

        if (board.king_in_check()) // checkmate
            return -CHECKMATE_VAL + board.Ply;

        return 0; // stalemate
    }

    return bestValue;
}

int Searcher::quiescence(const Board& board, int alpha, int beta) {
    return Evaluator::evaluate(board); // temp
}