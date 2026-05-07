#include "scorer.hh"

// mvv_lva[attacker][victim]
constexpr int mvv_lva[ALL_PIECES][ALL_PIECES-1] = {
// ( Attacker ↓ / Victim → )
//   Pawn       Knight  Bishop      Rook        Queen   King
    1000105, 1000205,   1000305,    1000405,    1000505,     
    1000104, 1000204,   1000304,    1000404,    1000504,    
    1000103, 1000203,   1000303,    1000403,    1000503,   
    1000102, 1000202,   1000302,    1000402,    1000502,    
    1000101, 1000201,   1000301,    1000401,    1000501,   
    1000100, 1000200,   1000300,    1000400,    1000500,   
};

void Scorer::insertion_sort() {
    for (ScoredMove* i = sm_start + 1; i < sm_end; ++i) {
        
        ScoredMove temp = *i; 

        ScoredMove* j = i - 1;

        while (j >= sm_start && j->score < temp.score) {
            *(j + 1) = *j;
            --j;           
        }

        *(j + 1) = temp; 
    }
}

template<MoveType T>
ScoredMove* Scorer::score(const MoveList& ml) {
    ScoredMove* cur = sm_start;

    for (const Move& move : ml) {
        cur->m = move;
        
        if constexpr (T == CAPTURE) {
            Piece attacker = type_of(board.Mailbox[move.getFrom()]);
            Piece victim   = type_of(board.Mailbox[move.getTo()]);
            if (move.is_ep()) victim = PAWN; 
            cur->score     = mvv_lva[attacker][victim];
        } 
        else if constexpr (T == QUIET) {
            cur->score = history[board.get_piece_from_sq(move.getFrom())][move.getTo()];
            if(node_killers) {
            if (move == node_killers[FIRST_KILLER])
                cur->score = 900000;

            else if (move == node_killers[SECOND_KILLER])
                cur->score = 800000;
            }
        }
        else { // outofcheck
            Piece victim   = type_of(board.Mailbox[move.getTo()]);
            if (move.is_ep()) victim = PAWN; 
            if (victim) {
            Piece attacker = type_of(board.Mailbox[move.getFrom()]);
            cur->score     = mvv_lva[attacker][victim];
            } else {
                cur->score = history[board.get_piece_from_sq(move.getFrom())][move.getTo()];
            }

        }

        cur++; 
    }
    
    return cur; 
}

Move Scorer::next_move() {
topflag:
    switch(state) {
        case SHASH_MOVE: 
            ++state;
            if(!ttmove.is_empty() && board.pseudolegal(ttmove)) return ttmove;
            goto topflag;

        case SINIT_CAPTURES: {
            MoveList ml = MoveList(); 
            ml.generate_pseudolegals<CAPTURE>(board);

            sm_end = score<CAPTURE>(ml);

            insertion_sort(); 

            yield = sm_start; 
            
            ++state;
            goto topflag;
        }

        case SCAPTURES:
            if (yield < sm_end) {
                Move best_capture = yield->m; 
                yield++;                      
                return best_capture;              
            }
            
            ++state;
            goto topflag;

        case SINIT_QUIETS: {
            if(qsearch) return Move::empty_move();
            MoveList ml = MoveList(); 
            ml.generate_pseudolegals<QUIET>(board);
            sm_end = score<QUIET>(ml);
            insertion_sort(); 
            yield = sm_start;
            
            ++state;
            goto topflag;
        }

        case SQUIETS: 
            if (yield < sm_end) {
                Move next_quiet = yield->m;
                yield++;
                return next_quiet;
            }
            return Move::empty_move();

        case SINIT_OUTOFCHECK: {
            MoveList ml = MoveList(); 
            ml.generate_pseudolegals<GET_OUT_OF_CHECK>(board);
            sm_end = score<GET_OUT_OF_CHECK>(ml);

            insertion_sort(); 
            
            yield = sm_start;
            
            ++state;
            goto topflag;
        }

        case SOUTOFCHECK:
            if (yield < sm_end) {
                Move next_ev = yield->m;
                yield++;
                return next_ev;
            }
            ++state;

        default:
            return Move::empty_move();
    }
}