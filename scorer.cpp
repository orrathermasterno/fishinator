#include "scorer.hh"

// mvv_lva[attacker][victim]
constexpr int mvv_lva[ALL_PIECES][ALL_PIECES-1] = {
// ( Attacker ↓ / Victim → )
//  Pawn Knight Bishop   Rook  Queen   King
    105, 205,   305,    405,    505,     
    104, 204,   304,    404,    504,    
    103, 203,   303,    403,    503,   
    102, 202,   302,    402,    502,    
    101, 201,   301,    401,    501,   
    100, 200,   300,    400,    500,   
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
            cur->score     = mvv_lva[attacker][victim];
        } 
        else if constexpr (T == QUIET) {
            cur->score = 0; 
        }
        else { // outofcheck
            Piece victim   = type_of(board.Mailbox[move.getTo()]);
            if (victim) {
            Piece attacker = type_of(board.Mailbox[move.getFrom()]);
            cur->score     = mvv_lva[attacker][victim];
            } else {
                cur->score = 0;
            }

        }

        cur++; 
    }
    
    return cur; 
}

Move Scorer::next_move() {
topflag:
    switch(state) {
        case SHASH_MOVE: // imp later
            ++state;

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
            //insertion_sort(); 
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