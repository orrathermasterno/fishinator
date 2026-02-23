#include "movegen.hh"
#include "attacks.hh"

void MoveList::print_movelist() const {
   for (const Move* ptr = Moves; ptr < last; ++ptr) {
        Move m = *ptr; 
        
        std::cout << "Move: " << square_to_string[m.getFrom()] << " to " << square_to_string[m.getTo()] << "\n";
    }
}

// unsuitable for kings and pawns
// // unsure how to handle capture flag rn, subject to change 
// // // what if i just discard them completely for now what then 
// template<Piece P, CaptureFlag IsCapture>
template<Piece P>
void MoveList::generate_pseudolegals_for(const Board& board, Bitboard targets, Color ActiveColor) {
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

template<MoveType T, Color ActiveColor>
void MoveList::generate_pawn_pseudolegals(const Board& board, Bitboard targets) {

}



// god i wish there was an easier way to do this (c)
#define INSTANTIATE_MOVE_TYPES(PIECE, BOARD, BB, COLOR) \
    template void MoveList::generate_pseudolegals_for<PIECE>(BOARD, BB, COLOR);

INSTANTIATE_MOVE_TYPES(KNIGHT, const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(BISHOP, const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(ROOK,   const Board&, Bitboard, Color)
INSTANTIATE_MOVE_TYPES(QUEEN,  const Board&, Bitboard, Color)

#undef INSTANTIATE_MOVE_TYPES