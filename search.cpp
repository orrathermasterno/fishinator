#include "search.hh"
#include "evaluation.hh"

int Searcher::root_game_ply;

Move Searcher::root_alphabeta(Board& board, int depth) {
    root_game_ply = board.Ply;
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

    int ply_since_search_root = board.Ply - root_game_ply;

    if(board.is_forced_draw(ply_since_search_root)) 
        return 0;

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
            return -CHECKMATE_VAL + ply_since_search_root;

        return 0; // stalemate
    }

    return bestValue;
}

int Searcher::quiescence(Board& board, int alpha, int beta) {
    return Evaluator::evaluate(board); // temp
}

// int Searcher::quiescence(Board& board, int alpha, int beta) {
//     int static_eval = Evaluator::evaluate(board); 
//     int bestValue = static_eval, score;
//     bool in_check = board.king_in_check();
//     MoveList ml = MoveList();
//     int ply_since_search_root = board.Ply - root_game_ply;

//     if(board.is_forced_draw(ply_since_search_root)) 
//         return 0;


//     if (!in_check) {
//         // stand pat
//         if(bestValue >= beta)
//             return bestValue;
//         if(bestValue > alpha)   
//             alpha = bestValue;

//         ml.generate_pseudolegals<CAPTURE>(board);
//     }
//     else {
//         bestValue = -INFINITY_VAL;
//         ml.generate_pseudolegals<GET_OUT_OF_CHECK>(board);
//     }

//     for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
//         Move current_move = *ptr;

//         if (!board.legal(current_move)) {
//             continue; 
//         }

//         BoardState state = BoardState(); 
//         board.make_move(current_move, state);

//         score = -quiescence(board, -beta, -alpha);

//         board.unmake_move(current_move);

//         if( score >= beta )
//             return score;
//         if( score > bestValue )
//             bestValue = score;
//         if( score > alpha )
//             alpha = score;
//     }

//     if(in_check && bestValue == -INFINITY_VAL) { // mate
//         return -CHECKMATE_VAL + board.Ply;
//     }

//     return bestValue;
// }