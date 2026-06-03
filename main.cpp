#include "board.hh"
#include "bitboard.hh"
#include <iostream>
#include <bitset>
#include <cstdint>
#include "prng.hh"
#include "attacks.hh"
#include "movegen.hh"
#include <sys/time.h>
#include "uci.hh"
#include "evaluation.hh"
#include "search.hh"
#include "scorer.hh"

// template<bool isRoot>
// uint64_t Perft_scorer(Board& board, int depth)
// {
//     if (depth == 0) {
//         return 1ULL;
//     }

//     Scorer sc = Scorer(board, false);
//     uint64_t nodes = 0, count = 0;

//     int us = board.ActiveColor;   

// #ifdef BENCH
//     int start_time = 0;
//     if (isRoot) {
//         start_time = get_time_ms(); 
//     }
// #endif

//     Move current_move;

//     while ((current_move = sc.next_move()) != Move::empty_move()) {
        
//         if (!board.legal(current_move))
//           continue;
//         CopyMake state = CopyMake(); 
//         board.make_move(current_move, state);
        
//         count = Perft_scorer<false>(board, depth - 1);
//         nodes += count;

//     #ifndef BENCH
//         if (isRoot) {
//             std::cout << current_move.move_to_str(us) << ": " << count << std::endl;
//         }
//     #endif
        
//         board.unmake_move(current_move);
//     }

// #ifdef BENCH
//     if (isRoot) {
//         int elapsed = get_time_ms() - start_time;
        
//         std::cout << "depth       : " << depth << "\n";
//         std::cout << "time (ms)   : " << elapsed << "\n";

//         if (elapsed > 0) {
//             uint64_t nps = (nodes * 1000ULL) / elapsed;
//             std::cout << "nps         : " << nps << "\n";
//         }
//     }
// #endif
  
//     return nodes;
// }

// template uint64_t Perft_scorer<true>(Board&, int);
// template uint64_t Perft_scorer<false>(Board&, int);



int main() {
  Attacks::init();
  Evaluator::init();
  Zobrist::init();

  //  Board board = Board();
  //   board.parse_FEN("8/qpKP3k/8/2P1N3/8/8/8/8 w - - 0 1");
  //  Move move = Move(c5, b6, EP_CAPTURE_F);
  //  cout << board.pseudolegal(move);
//   MoveList ml = MoveList();
//   ml.generate_all_legals(board);
//   print_movelist(board, ml);
 // cout << Perft_scorer<true>(board, 5);

  //cout << Evaluator::evaluate(board);

   UCI::loop();
}