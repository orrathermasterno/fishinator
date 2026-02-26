#include "movegen.hh"
#include "attacks.hh"

constexpr int WHITE_KING_STARTING_SQ = e1;
constexpr int BLACK_KING_STARTING_SQ = e8;

constexpr int WHITE_KING_OO_SQ = g1;
constexpr int WHITE_KING_OOO_SQ = c1;
constexpr int BLACK_KING_OO_SQ = g8;
constexpr int BLACK_KING_OOO_SQ = c8;

void MoveList::print_movelist() const {
   for (const Move* ptr = Moves; ptr < last; ++ptr) {
        Move m = *ptr; 
        
        std::cout << "Move: " << square_to_string[m.getFrom()] << " to " << square_to_string[m.getTo()] 
            << " with flag " << m.getFlags() << "\n";
    }
}

// unsuitable for pawns
// // unsure how to handle capture flag rn, subject to change 
// // // what if i just discard them completely for now what then 
// template<Piece P, CaptureFlag IsCapture>
template<Piece P, Color ActiveColor>
void MoveList::generate_pseudolegals_for(const Board& board, Bitboard targets) {
    Bitboard piece_board = board.get_colored_piece_bb<P>(ActiveColor);
    int from_square;
    Bitboard attacks;
    // MoveFlag flag = IsCapture ? CAPTURES : QUIET;

    while(piece_board) {
        from_square = pop_lsb(piece_board);
        attacks = Attacks::get_attack_of<P>(from_square, board.ColorBB[BOTH]) & targets;

        while(attacks) {
            *last++ = Move(from_square, pop_lsb(attacks)); // to_square = pop_lsb(attacks);
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
        add_pawn_moves<QUIET_F>(two_ahead, Direction(Push*2));

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
        targets = 0ULL; // todo !!!!!!
    }
                    ;
    Bitboard king_targets = (T == GET_OUT_OF_CHECK) ? ~board.ColorBB[ActiveColor] : targets;
    generate_pseudolegals_for<KING, ActiveColor>(board, king_targets);

    if constexpr (T == GET_OUT_OF_CHECK) {
        Bitboard king_attackers = board.enemy_attackers_of(board.get_king_sq(ActiveColor), board.ColorBB[BOTH], ActiveColor);

        if (king_attackers & (king_attackers-1)) return; // double check cannot be resolved by any moves save the king's
    }

    if constexpr (T != CAPTURE && T != GET_OUT_OF_CHECK) generate_castling<ActiveColor>(board);

    generate_pawn_pseudolegals<T, ActiveColor>(board, targets);
    generate_pseudolegals_for<KNIGHT, ActiveColor>(board, targets);
    generate_pseudolegals_for<ROOK, ActiveColor>(board, targets);
    generate_pseudolegals_for<BISHOP, ActiveColor>(board, targets);
    generate_pseudolegals_for<QUEEN, ActiveColor>(board, targets);
}

// explicit instantiations
template void MoveList::add_pawn_moves<QUIET_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<KNIGHT_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<BISHOP_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<ROOK_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<QUEEN_PROM_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<KNIGHT_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<BISHOP_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<ROOK_PROM_CAPTURE_F>(Bitboard, Direction);
template void MoveList::add_pawn_moves<QUEEN_PROM_CAPTURE_F>(Bitboard, Direction);


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

#define INSTANTIATE_MOVE_TYPES(PIECE, BOARD, BB, COLOR) \
    template void MoveList::generate_pseudolegals_for<PIECE, WHITE>(BOARD, BB); \
    template void MoveList::generate_pseudolegals_for<PIECE, BLACK>(BOARD, BB);

INSTANTIATE_MOVE_TYPES(KNIGHT, const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(BISHOP, const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(ROOK,   const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(QUEEN,  const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(KING,  const Board&, Bitboard, Color)

#undef INSTANTIATE_MOVE_TYPES