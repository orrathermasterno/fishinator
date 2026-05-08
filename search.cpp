#include "search.hh"
#include "evaluation.hh"
#include "scorer.hh"
#include <algorithm>
#include <cstring>

constexpr int MATE_BOUND = CHECKMATE_VAL - 1000;

int Searcher::root_game_ply;


/**********************************\
==================================

            move ordering

==================================
\**********************************/
Move Searcher::pv_table[MAX_PLY][MAX_PLY];
int Searcher::pv_length[MAX_PLY];
Move Searcher::killers[MAX_PLY][MAX_KILLERS];
int Searcher::history[12][ILLEGAL_SQ];
TTable Searcher::ttable;

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

void Searcher::update_pv(int ply_since_search_root, Move move) {
    pv_table[ply_since_search_root][0] = move;
    for (int i = 0; i < pv_length[ply_since_search_root + 1]; i++) {
        pv_table[ply_since_search_root][i + 1] = pv_table[ply_since_search_root + 1][i];
    }
    pv_length[ply_since_search_root] = pv_length[ply_since_search_root + 1] + 1;
}

int Searcher::score_from_tt(int tt_score, int ply) {
    if (tt_score > MATE_BOUND) return tt_score - ply;
    if (tt_score < -MATE_BOUND) return tt_score + ply;
    return tt_score;
}

int Searcher::score_to_tt(int score, int ply) {
    if (score > MATE_BOUND) return score + ply;
    if (score < -MATE_BOUND) return score - ply;
    return score;
}

/**********************************\
==================================

            clear stuff

==================================
\**********************************/

void Searcher::clear() {
    memset(history, 0, sizeof(history));
    ttable.clear();
}

void Searcher::clear_killers() {
    memset(killers, 0, sizeof(killers));
}

void Searcher::clear_pv() {
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));
}


/**********************************\
==================================

            search

==================================
\**********************************/

#ifdef BENCH
    int Searcher::nodes;
    int Searcher::qnodes;

    void Searcher::clear_bench() {
        nodes = 0; qnodes = 0;
    }
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
    pv_length[0] = 0; // pv mess

    TTEntry ttentry;
    ttable.get_entry(board.bs->Key, ttentry);
    Scorer sc = Scorer(board, false, nullptr, history, ttentry.BestMove);

    Move current_move;
    int legals = 0;
    

    while ((current_move = sc.next_move()) != Move::empty_move()) {
        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

        legals++;
        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        if (legals == 1) {
            score = -alphabeta(board, -beta, -alpha, depth - 1);
        } else {
            score = -alphabeta(board, -alpha - 1, -alpha, depth - 1);

            if (score > alpha && score < beta) {
                score = -alphabeta(board, -beta, -alpha, depth - 1);
            }
        }

        board.unmake_move(current_move);

        if(score > bestValue) {
            bestValue = score;
            bestmove = current_move;
            
            if (score > alpha) {
                alpha = score;

                // pv mess
                update_pv(0, current_move);
            }
        }
    }
    clear_killers();
#ifdef BENCH
    std::cout << "nodes: " << nodes << "\n";
    std::cout << "qnodes: " << qnodes << "\n";
    clear_bench();
    std::cout << "info depth " << depth << " score cp " << bestValue << " pv ";
    for (int i = 0; i < pv_length[0]; i++) {
        std::cout << pv_table[0][i].move_to_str(board.ActiveColor) << " "; 
    }
    std::cout << "\n";
#endif

    clear_pv();
    return bestmove;
}

int Searcher::alphabeta(Board& board, int alpha, int beta, int depth_left) {
    if(depth_left == 0) return quiescence(board, alpha, beta);
    #ifdef BENCH
        nodes++;
    #endif

    int ply_since_search_root = board.Ply - root_game_ply;
    pv_length[ply_since_search_root] = 0;
    

    if(board.is_forced_draw(ply_since_search_root)) 
        return 0;

    Color us = board.ActiveColor;     
    int king_sq = board.get_king_sq(us);
    int score;     
    int bestValue = -INFINITY_VAL;    
    
    int history_bonus = depth_left * depth_left + depth_left - 1;

    int legals = 0;

    Move bestmove = Move::empty_move();
    NodeType hashflag = ALL_NODE;

    TTEntry ttentry;
    bool tthit = ttable.get_entry(board.bs->Key, ttentry);

    bool isPV = (alpha != beta - 1);
    if (!isPV && tthit && ttentry.Depth >= depth_left) { 

        int tt_score = score_from_tt(ttentry.Value, ply_since_search_root);

        if (ttentry.Type == PV_NODE) return tt_score; 
        else if (ttentry.Type == ALL_NODE && tt_score <= alpha) return tt_score; 
        else if (ttentry.Type == CUT_NODE && tt_score >= beta) return tt_score;
    }

    Scorer sc = Scorer(board, false, killers[ply_since_search_root], history, ttentry.BestMove);

    Move current_move;

    while ((current_move = sc.next_move()) != Move::empty_move()) {

        bool is_pinned = board.is_pinned(current_move.getFrom(), us);
        if ((is_pinned || current_move.getFrom() == king_sq || current_move.is_ep()) 
            && !board.legal(current_move))
           continue;

        legals++;

        BoardState state = BoardState(); 
        board.make_move(current_move, state);

        if (legals == 1) {
            score = -alphabeta(board, -beta, -alpha, depth_left - 1);
        } else {
            score = -alphabeta(board, -alpha-1, -alpha, depth_left - 1);

            if((score > alpha) && (score < beta))       // if (score > alpha && beta - alpha > 1) ???
                score = -alphabeta(board, -beta, -alpha, depth_left - 1);
        }

        board.unmake_move(current_move);

        if(score > bestValue)
        {
            bestValue = score;
            bestmove = current_move;
            if(score > alpha) {
                hashflag = PV_NODE;
                alpha = score; // alpha acts like max in MiniMax

                // pv mess
                update_pv(ply_since_search_root, current_move);
            }
        }
        if(score >= beta) {
            if(!current_move.is_capture() && !current_move.is_promotion()) {
                add_killer(current_move, ply_since_search_root);
                update_history(board.get_piece_from_sq(current_move.getFrom()), current_move.getTo(), history_bonus); // higher cutoff costs more
            }

            int tt_score = score_to_tt(bestValue, ply_since_search_root);
            ttable.store_entry({board.bs->Key, tt_score, CUT_NODE, depth_left, current_move});
            return bestValue;   // fail soft beta-cutoff
        }
    }

    if (!legals) {
        if (board.king_in_check()) { // checkmate
            bestValue = -CHECKMATE_VAL + ply_since_search_root;
            hashflag = PV_NODE;
        }

        else bestValue = 0; // stalemate
    }

    int tt_score = score_to_tt(bestValue, ply_since_search_root);
    ttable.store_entry({board.bs->Key, tt_score, hashflag, depth_left, bestmove});

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

    TTEntry ttentry;
    bool tthit = ttable.get_entry(board.bs->Key, ttentry);

    // bool isPV = (alpha != beta - 1);
    // if (!isPV && tthit) { 

    //     int tt_score = score_from_tt(ttentry.Value, ply_since_search_root);

    //     if (ttentry.Type == PV_NODE) return tt_score; 
    //     else if (ttentry.Type == ALL_NODE && tt_score <= alpha) return tt_score; 
    //     else if (ttentry.Type == CUT_NODE && tt_score >= beta) return tt_score;
    // }

    Move passmove = ttentry.BestMove.is_capture() ? ttentry.BestMove : Move::empty_move();
    Scorer sc = Scorer(board, true, nullptr, history, passmove);

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