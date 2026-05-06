#include "search.hh"
#include "evaluation.hh"
#include "scorer.hh"
#include <algorithm>

int Searcher::root_game_ply;


/**********************************\
==================================

            move ordering

==================================
\**********************************/
Move Searcher::killers[MAX_PLY][MAX_KILLERS];
int Searcher::history[12][ILLEGAL_SQ];

void Searcher::add_killer(const Move& move, int ply) {
    if (move == killers[ply][FIRST_KILLER]) return;

    killers[ply][SECOND_KILLER] = killers[ply][FIRST_KILLER];
    killers[ply][FIRST_KILLER] = move;
}

void Searcher::update_history(ColoredPiece moved_piece, int sq_to, int bonus) {
    int clampedBonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
    history[moved_piece][sq_to]
        += clampedBonus - history[moved_piece][sq_to] * abs(clampedBonus) / MAX_HISTORY;
}

/**********************************\
==================================

            search

==================================
\**********************************/

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

    Scorer sc = Scorer(board, false, nullptr, history);

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
    
    int history_bonus = depth_left * depth_left + depth_left - 1;

    int legals = 0;

    Scorer sc = Scorer(board, false, killers[ply_since_search_root], history);

    Move current_move;

    while ((current_move = sc.next_move()) != Move::empty_move()) {

        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

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
        if(score >= beta) {
            if(!current_move.is_capture() && !current_move.is_promotion()) {
                add_killer(current_move, ply_since_search_root);
                update_history(board.get_piece_from_sq(current_move.getFrom()), current_move.getTo(), history_bonus); // higher cutoff costs more
            }

            // for(int i = 0; i < quiets; i++) {
            //     update_history(board.get_piece_from_sq(searched_quiets[i].getFrom()), searched_quiets[i].getTo(), -history_bonus); // penalize bad moves
            // }
            return bestValue;   // fail soft beta-cutoff
        }

        // if(!current_move.is_capture() && !current_move.is_promotion()) {
        //     searched_quiets[quiets] = current_move;
        //     quiets++;
        // }

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


    Scorer sc = Scorer(board, true, nullptr, history);

    Move current_move;

    while ((current_move = sc.next_move()) != Move::empty_move()) {

        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        score = -quiescence(board, -beta, -alpha);

        board.unmake_move(current_move);

        if(score >= beta) 
            return score;
        if(score > bestValue)
            bestValue = score;
        if(score > alpha)
            alpha = score;
    }

    if(in_check && bestValue == -INFINITY_VAL) { // mate
        return -CHECKMATE_VAL + ply_since_search_root;
    }

    return bestValue;
}