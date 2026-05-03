#include "search.hh"
#include "evaluation.hh"
#include "scorer.hh"

int Searcher::root_game_ply;

#ifdef BENCH
    int Searcher::nodes;
    int Searcher::qnodes;
#endif

Move Searcher::root_alphabeta(Board& board, int depth) {
    root_game_ply = board.Ply;
    Color us = board.ActiveColor;   
    int king_sq = board.get_king_sq(us);  
    int score;
    Move bestmove;
    int alpha = -INFINITY_VAL;
    int beta = INFINITY_VAL;
    int bestValue = -INFINITY_VAL;

    Scorer sc = Scorer(board, false);

    Move current_move;

     while ((current_move = sc.next_move()) != Move::empty_move()) {
        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

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
#ifdef BENCH
    std::cout << "nodes: " << nodes << "\n";
    std::cout << "qnodes: " << qnodes << "\n";
#endif
    return bestmove;
}

int Searcher::alphabeta(Board& board, int alpha, int beta, int depth_left) {
    if(depth_left == 0) return quiescence(board, alpha, beta);
    #ifdef BENCH
        nodes++;
    #endif

    int ply_since_search_root = board.Ply - root_game_ply;
    

    if(board.is_forced_draw(ply_since_search_root)) 
        return 0;

    Color us = board.ActiveColor;     
    int king_sq = board.get_king_sq(us);
    int score;     
    int bestValue = -INFINITY_VAL;       

    int legals = 0;

    Scorer sc = Scorer(board, false);

    Move current_move;

    while ((current_move = sc.next_move()) != Move::empty_move()) {

        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

        legals++;

        std::cout << "search" << ply_since_search_root << ": " << current_move.move_to_str(us) << "\n";

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

// int Searcher::quiescence(Board& board, int alpha, int beta) {
//     return Evaluator::evaluate(board); // temp
// }

int Searcher::quiescence(Board& board, int alpha, int beta) {
    bool in_check = board.king_in_check();
    Color us = board.ActiveColor;
    int king_sq = board.get_king_sq(us);
    int ply_since_search_root = board.Ply - root_game_ply;

    #ifdef BENCH
        qnodes++;
    #endif

    if(board.is_forced_draw(ply_since_search_root)) 
        return 0;

    int bestValue = -INFINITY_VAL;
    int score;

    if (!in_check) {
        int static_eval = Evaluator::evaluate(board); 
        bestValue = static_eval;
        
        // stand pat
        if(bestValue >= beta)
            return bestValue;
        if(bestValue > alpha)   
            alpha = bestValue;
    }


    Scorer sc = Scorer(board, true);

    Move current_move;

    while ((current_move = sc.next_move()) != Move::empty_move()) {

        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

        std::cout << "qsearch" << ply_since_search_root << ": " << current_move.move_to_str(us) << "\n";

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        score = -quiescence(board, -beta, -alpha);

        board.unmake_move(current_move);

        if( score >= beta )
            return score;
        if( score > bestValue )
            bestValue = score;
        if( score > alpha )
            alpha = score;
    }

    if(in_check && bestValue == -INFINITY_VAL) { // mate
        return -CHECKMATE_VAL + ply_since_search_root;
    }

    return bestValue;
}