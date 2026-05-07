#include "movegen.hh"
#include "attacks.hh"
#include "board.hh"

#include <sys/time.h>

constexpr int WHITE_KING_STARTING_SQ = e1;
constexpr int BLACK_KING_STARTING_SQ = e8;

constexpr int WHITE_KING_OO_SQ = g1;
constexpr int WHITE_KING_OOO_SQ = c1;
constexpr int BLACK_KING_OO_SQ = g8;
constexpr int BLACK_KING_OOO_SQ = c8;

void MoveList::print_movelist() const {
    int i = 0;
   for (const Move* ptr = Moves; ptr < last; ++ptr) {
        Move m = *ptr; 
        
        std::cout << "Move " << i << ": " << square_to_string[m.getFrom()] << " to " << square_to_string[m.getTo()] 
            << " with flag " << m.getFlags() << "\n";
        i++;
    }
}

// unsuitable for pawns
template<Piece P, Color ActiveColor, MoveType T>
void MoveList::generate_pseudolegals_for(const Board& board, Bitboard targets) {
    Bitboard piece_board = board.get_colored_piece_bb<P>(ActiveColor);
    int from_square;
    Bitboard attacks;
    MoveFlag flag = QUIET_F;
    if constexpr (T == CAPTURE) flag = CAPTURES_F;

    while(piece_board) {
        from_square = pop_lsb(piece_board);
        attacks = Attacks::get_attack_of<P>(from_square, board.ColorBB[BOTH]) & targets;

        while(attacks) {
            *last++ = Move(from_square, pop_lsb(attacks), flag); // to_square = pop_lsb(attacks);
        }
    }
}

template<MoveFlag Flag>
void MoveList::add_pawn_moves(Bitboard moves_to, Direction push_dir) {
    int to_square;
    while(moves_to) {
        to_square = pop_lsb(moves_to);
        *last++ = Move(to_square - push_dir, to_square, Flag);
    }
}

template<MoveType T, Color ActiveColor>
void MoveList::generate_pawn_pseudolegals(const Board& board, Bitboard targets) {
    constexpr Direction Push = ActiveColor == WHITE ? NORTH : SOUTH;
    constexpr Direction PushAttackLeft = ActiveColor == WHITE ? NORTH_WEST : SOUTH_WEST;
    constexpr Direction PushAttackRight = ActiveColor == WHITE ? NORTH_EAST : SOUTH_EAST;

    Bitboard pawn_bb = board.get_colored_piece_bb<PAWN>(ActiveColor);
    Bitboard free_squares = ~board.ColorBB[BOTH];
    Bitboard enemies = board.ColorBB[ActiveColor^1];
    constexpr Bitboard ThirdRank = ActiveColor == WHITE ? Rank3_const : Rank6_const;
    constexpr Bitboard SixthRank = ActiveColor == WHITE ? Rank7_const : Rank2_const;

    Bitboard promotion_bb = pawn_bb & SixthRank;
    Bitboard normal_move_bb = pawn_bb & ~SixthRank;

    // quiets
    if constexpr (T != CAPTURE) {
        Bitboard one_ahead = shift<Push>(normal_move_bb) & free_squares;
        Bitboard two_ahead = shift<Push>(one_ahead & ThirdRank) & free_squares;

        if constexpr (T == GET_OUT_OF_CHECK) {
            one_ahead &= targets;
            two_ahead &= targets;
        }

        add_pawn_moves<QUIET_F>(one_ahead, Push); 
        add_pawn_moves<DOUBLE_PAWN_PUSH_F>(two_ahead, Direction(Push*2));

        // quiet promotions
        Bitboard promotions = shift<Push>(promotion_bb) & free_squares;
        if(T == GET_OUT_OF_CHECK) {
            promotions &= targets;
        }
        if (promotions) {
            add_pawn_moves<KNIGHT_PROM_F>(promotions, Push);
            add_pawn_moves<BISHOP_PROM_F>(promotions, Push);
            add_pawn_moves<ROOK_PROM_F>(promotions, Push);
            add_pawn_moves<QUEEN_PROM_F>(promotions, Push);
        }
    }

    // captures
    if constexpr (T != QUIET) {

        Bitboard right_captures = shift<PushAttackRight>(normal_move_bb) & enemies;
        Bitboard left_captures = shift<PushAttackLeft>(normal_move_bb) & enemies;

        if constexpr (T == GET_OUT_OF_CHECK) {
            right_captures &= targets;
            left_captures &= targets;
        }

        add_pawn_moves<CAPTURES_F>(right_captures, PushAttackRight); 
        add_pawn_moves<CAPTURES_F>(left_captures, PushAttackLeft); 

        // capture promotions
        Bitboard right_prom = shift<PushAttackRight>(promotion_bb) & enemies;
        Bitboard left_prom = shift<PushAttackLeft>(promotion_bb) & enemies;

        if constexpr (T == GET_OUT_OF_CHECK) {
            right_prom &= targets;
            left_prom &= targets;
        }

        if (right_prom) {
            add_pawn_moves<KNIGHT_PROM_CAPTURE_F>(right_prom, PushAttackRight); 
            add_pawn_moves<BISHOP_PROM_CAPTURE_F>(right_prom, PushAttackRight); 
            add_pawn_moves<ROOK_PROM_CAPTURE_F>(right_prom, PushAttackRight); 
            add_pawn_moves<QUEEN_PROM_CAPTURE_F>(right_prom, PushAttackRight); 
        }

        if (left_prom) {
            add_pawn_moves<KNIGHT_PROM_CAPTURE_F>(left_prom, PushAttackLeft); 
            add_pawn_moves<BISHOP_PROM_CAPTURE_F>(left_prom, PushAttackLeft); 
            add_pawn_moves<ROOK_PROM_CAPTURE_F>(left_prom, PushAttackLeft); 
            add_pawn_moves<QUEEN_PROM_CAPTURE_F>(left_prom, PushAttackLeft); 
        }

        // en passant capture
        if(board.get_ep() != ILLEGAL_SQ) {
            Bitboard enpassant_attackers = normal_move_bb & Attacks::get_pawn_attack(board.get_ep(), Color(ActiveColor ^ 1));

            if (T == GET_OUT_OF_CHECK 
                && (targets & set_bit(0ULL, board.get_ep() + Push))) // if enemy pawn push resulted in discovered check, en passant won't block it
                enpassant_attackers = 0ULL;

            while(enpassant_attackers) {
                *last++ = Move(pop_lsb(enpassant_attackers), board.get_ep(), EP_CAPTURE_F);
            }
        }
    }
}

template<Color ActiveColor>
void MoveList::generate_castling(const Board& board) {
    if constexpr (ActiveColor == WHITE) {
        if ((board.get_castling_rights() & WHITE_OO) && board.can_castle<WHITE_OO>()) 
            *last++ = Move(WHITE_KING_STARTING_SQ, WHITE_KING_OO_SQ, KING_CASTLE_F); 
        if ((board.get_castling_rights() & WHITE_OOO) && board.can_castle<WHITE_OOO>()) 
            *last++ = Move(WHITE_KING_STARTING_SQ, WHITE_KING_OOO_SQ, QUEEN_CASTLE_F); 
    } 
    else { // ActiveColor == BLACK
        if ((board.get_castling_rights() & BLACK_OO) && board.can_castle<BLACK_OO>()) 
            *last++ = Move(BLACK_KING_STARTING_SQ, BLACK_KING_OO_SQ, KING_CASTLE_F); 
        if ((board.get_castling_rights() & BLACK_OOO) && board.can_castle<BLACK_OOO>()) 
            *last++ = Move(BLACK_KING_STARTING_SQ, BLACK_KING_OOO_SQ, QUEEN_CASTLE_F); 
    }
}

template<MoveType T, Color ActiveColor>
void MoveList::generate_pseudolegals(const Board& board) {
    Bitboard targets;
    Bitboard king_attackers = board.get_checkers(ActiveColor, board.ColorBB[BOTH]);

    if constexpr (T == QUIET) {
        targets = ~board.ColorBB[BOTH];
    } 
    else if constexpr (T == CAPTURE) {
        targets = board.ColorBB[ActiveColor ^ 1];
    } 
    else if constexpr (T == QUIET_AND_CAPTURE) {
        targets = ~board.ColorBB[ActiveColor];
    } 
    else if constexpr (T == GET_OUT_OF_CHECK) {
        int attacker_sq = bit_scan_forward(king_attackers); 
        
        targets = Attacks::get_between_sq_bb(board.get_king_sq(ActiveColor), attacker_sq) 
                | set_bit(0ULL, attacker_sq); 
    }
                    ;
    Bitboard king_targets = (T == GET_OUT_OF_CHECK) ? ~board.ColorBB[ActiveColor] : targets;
    if constexpr (T == CAPTURE || T == QUIET) {
        generate_pseudolegals_for<KING, ActiveColor, T>(board, king_targets);
    } 
    else {
        Bitboard king_captures = king_targets & board.ColorBB[ActiveColor ^ 1];
        Bitboard king_quiets   = king_targets & ~board.ColorBB[BOTH];
        
        generate_pseudolegals_for<KING, ActiveColor, CAPTURE>(board, king_captures);
        generate_pseudolegals_for<KING, ActiveColor, QUIET>(board, king_quiets);
    }

    //////////////////////
    if (more_than_one(king_attackers)) return; // double check cannot be resolved by any moves save the king's
    /////////////////////

    if constexpr (T != CAPTURE && T != GET_OUT_OF_CHECK) generate_castling<ActiveColor>(board);

    generate_pawn_pseudolegals<T, ActiveColor>(board, targets);
    if constexpr (T == CAPTURE || T == QUIET) {
        generate_pseudolegals_for<KNIGHT, ActiveColor, T>(board, targets);
        generate_pseudolegals_for<ROOK, ActiveColor, T>(board, targets);
        generate_pseudolegals_for<BISHOP, ActiveColor, T>(board, targets);
        generate_pseudolegals_for<QUEEN, ActiveColor, T>(board, targets);
    }
    else {
        Bitboard capture_targets = targets & board.get_color_bb(~ActiveColor);
        generate_pseudolegals_for<KNIGHT, ActiveColor, CAPTURE>(board, capture_targets);
        generate_pseudolegals_for<ROOK, ActiveColor, CAPTURE>(board, capture_targets);
        generate_pseudolegals_for<BISHOP, ActiveColor, CAPTURE>(board, capture_targets);
        generate_pseudolegals_for<QUEEN, ActiveColor, CAPTURE>(board, capture_targets);

        Bitboard q_targets = targets & ~board.get_color_bb(BOTH);
        generate_pseudolegals_for<KNIGHT, ActiveColor, QUIET>(board, q_targets);
        generate_pseudolegals_for<ROOK, ActiveColor, QUIET>(board, q_targets);
        generate_pseudolegals_for<BISHOP, ActiveColor, QUIET>(board, q_targets);
        generate_pseudolegals_for<QUEEN, ActiveColor, QUIET>(board, q_targets);
    }
}

template<MoveType T>
void MoveList::generate_pseudolegals(const Board& board) {
    if (board.ActiveColor == WHITE) generate_pseudolegals<T, WHITE>(board);
    else generate_pseudolegals<T, BLACK>(board);
}

void MoveList::generate_all_legals(const Board& board) {
    Color side = board.ActiveColor;
    int king_sq = board.get_king_sq(side);

    if (board.king_in_check()) generate_pseudolegals<GET_OUT_OF_CHECK>(board); // don't waste time generating extra moves in case of check
    else generate_pseudolegals<QUIET_AND_CAPTURE>(board);

    Move* cur = Moves;

    while (cur != last) {
        bool is_pinned = board.is_pinned(cur->getFrom(), side);
        
        if ((is_pinned || cur->getFrom() == king_sq || cur->is_ep())  // short-circuit eval
            && !board.legal(*cur)) {

            *cur = *(--last); 
        } 
        else {
            ++cur;
        }
    }
}

/**********************************\
==================================

                perft

==================================
\**********************************/

int get_time_ms()
{
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

template<bool isRoot>
uint64_t Perft(Board& board, int depth)
{
    if (depth == 0) {
    return 1ULL;
    }

    MoveList ml = MoveList();
    uint64_t nodes = 0, count = 0;

    int us = board.ActiveColor;   
    int them = us ^ 1;    

#ifdef BENCH
    int start_time = 0;
    if (isRoot) {
        start_time = get_time_ms();
    }
#endif

    ml.generate_all_legals(board);

    if (depth == 1 && !isRoot) 
        return ml.last-ml.Moves;

    for (const Move* ptr = ml.Moves; ptr < ml.last; ++ptr) {
        Move current_move = *ptr;
        // assert(board.pseudolegal(current_move));

        BoardState state = BoardState(); 
        board.make_move(current_move, state);
        
        count = Perft<false>(board, depth - 1);
        nodes += count;

    #ifndef BENCH
        if(isRoot) std::cout << current_move.move_to_str(us) << ": " << count << std::endl;
    #endif
        
        board.unmake_move(current_move);
    }

#ifdef BENCH
    if (isRoot) {
        int elapsed = get_time_ms() - start_time;
        
        std::cout << "depth       : " << depth << "\n";
        std::cout << "time (ms)   : " << elapsed << "\n";

        if (elapsed > 0) {
            uint64_t nps = (nodes * 1000ULL) / elapsed;
            std::cout << "nps         : " << nps << "\n";
        }
    }
#endif
  
    return nodes;
}

/**********************************\
==================================

        explicit instantiations

==================================
\**********************************/

template void MoveList::add_pawn_moves<QUIET_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<KNIGHT_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<BISHOP_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<ROOK_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<QUEEN_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<KNIGHT_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<BISHOP_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<ROOK_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<QUEEN_PROM_CAPTURE_F>(Bitboard, Direction);

template void MoveList::generate_castling<WHITE>(const Board& board);
template void MoveList::generate_castling<BLACK>(const Board& board);


#define INSTANTIATE_PSEUDOLEGALS(COLOR) \
    template void MoveList::generate_pseudolegals<QUIET, COLOR>(const Board&); \
    template void MoveList::generate_pseudolegals<CAPTURE, COLOR>(const Board&); \
    template void MoveList::generate_pseudolegals<QUIET_AND_CAPTURE, COLOR>(const Board&); \
    template void MoveList::generate_pseudolegals<GET_OUT_OF_CHECK, COLOR>(const Board&);

INSTANTIATE_PSEUDOLEGALS(WHITE)
INSTANTIATE_PSEUDOLEGALS(BLACK)

#undef INSTANTIATE_PSEUDOLEGALS

#define INSTANTIATE_PAWN_PSEUDOLEGALS(COLOR) \
    template void MoveList::generate_pawn_pseudolegals<QUIET, COLOR>(const Board&, Bitboard); \
    template void MoveList::generate_pawn_pseudolegals<CAPTURE, COLOR>(const Board&, Bitboard); \
    template void MoveList::generate_pawn_pseudolegals<QUIET_AND_CAPTURE, COLOR>(const Board&, Bitboard); \
    template void MoveList::generate_pawn_pseudolegals<GET_OUT_OF_CHECK, COLOR>(const Board&, Bitboard);

INSTANTIATE_PAWN_PSEUDOLEGALS(WHITE)
INSTANTIATE_PAWN_PSEUDOLEGALS(BLACK)

#undef INSTANTIATE_PAWN_PSEUDOLEGALS

template void MoveList::generate_pseudolegals<QUIET>(const Board&);
template void MoveList::generate_pseudolegals<CAPTURE>(const Board&);
template void MoveList::generate_pseudolegals<QUIET_AND_CAPTURE>(const Board&);
template void MoveList::generate_pseudolegals<GET_OUT_OF_CHECK>(const Board&);

#define INSTANTIATE_MOVE_TYPES(PIECE) \
    template void MoveList::generate_pseudolegals_for<PIECE, WHITE, QUIET>(const Board&, Bitboard); \
    template void MoveList::generate_pseudolegals_for<PIECE, WHITE, CAPTURE>(const Board&, Bitboard); \
    template void MoveList::generate_pseudolegals_for<PIECE, BLACK, QUIET>(const Board&, Bitboard); \
    template void MoveList::generate_pseudolegals_for<PIECE, BLACK, CAPTURE>(const Board&, Bitboard);

INSTANTIATE_MOVE_TYPES(KNIGHT)
INSTANTIATE_MOVE_TYPES(BISHOP)
INSTANTIATE_MOVE_TYPES(ROOK)
INSTANTIATE_MOVE_TYPES(QUEEN)
INSTANTIATE_MOVE_TYPES(KING)

#undef INSTANTIATE_MOVE_TYPES

template uint64_t Perft<true>(Board&, int);
template uint64_t Perft<false>(Board&, int);