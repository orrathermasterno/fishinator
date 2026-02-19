#pragma once

#include <cstdint>
#include <cmath>

typedef uint64_t Bitboard;
inline constexpr uint64_t ONE = 1;
#define C64(constant)  ((uint64_t)constant)
constexpr size_t SQ_AMOUNT = 64;

enum Color {
  WHITE, BLACK, BOTH = 2
};

enum Piece: std::uint8_t {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES
};

// Little-Endian Rank-File Mapping
enum Square: int {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
  ILLEGAL_SQ
};


  // noWe         nort         noEa
  //         +7    +8    +9
  //             \  |  /
  // west    -1 <-  0 -> +1    east
  //             /  |  \
  //         -9    -8    -7
  // soWe         sout         soEa
enum Direction: int {
  NORTH =  8,
  EAST  =  1,
  SOUTH = -NORTH,
  WEST  = -EAST,

  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST
};

enum File: int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank: int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

constexpr Bitboard FileA_const = 0x0101010101010101ULL;
constexpr Bitboard FileB_const = FileA_const << 1;
constexpr Bitboard FileC_const = FileA_const << 2;
constexpr Bitboard FileD_const = FileA_const << 3;
constexpr Bitboard FileE_const = FileA_const << 4;
constexpr Bitboard FileF_const = FileA_const << 5;
constexpr Bitboard FileG_const = FileA_const << 6;
constexpr Bitboard FileH_const = FileA_const << 7;

constexpr Bitboard FileAB_const = FileA_const | FileB_const;
constexpr Bitboard FileGH_const = FileG_const | FileH_const;

constexpr Bitboard Rank1_const = 0xFF;
constexpr Bitboard Rank2_const = Rank1_const << (8 * 1);
constexpr Bitboard Rank3_const = Rank1_const << (8 * 2);
constexpr Bitboard Rank4_const = Rank1_const << (8 * 3);
constexpr Bitboard Rank5_const = Rank1_const << (8 * 4);
constexpr Bitboard Rank6_const = Rank1_const << (8 * 5);
constexpr Bitboard Rank7_const = Rank1_const << (8 * 6);
constexpr Bitboard Rank8_const = Rank1_const << (8 * 7);

constexpr Bitboard MAIN_DIAG = 0x8040201008040201ULL; // a1-h8 diagonal
constexpr Bitboard MAIN_ANTIDIAG = 0x0102040810204080ULL; // h1-a8 antidiagonal



constexpr Bitboard set_bit(Bitboard bitboard, int square) { return (bitboard |= ONE << square); }
constexpr Bitboard get_bit(Bitboard bitboard, int square) { return (bitboard & ONE << square); }
constexpr Bitboard pop_bit(Bitboard bitboard, int square) { return (bitboard &= ~(ONE << square)); }  

// Brian Kernighan's way
constexpr Bitboard Kernighan_population_count(Bitboard bitboard) {
  int count = 0;
  while (bitboard) {
      count++;
      bitboard &= bitboard - 1;
  }
  return count;
}
// compiler intrinsic
inline int population_count(Bitboard bb) {
    return __builtin_popcountll(bb); 
}
// compiler intrinsic
// simplest loop implementation: use (bitboard & -bitboard) - 1 with population_count
inline int bit_scan_forward(Bitboard bb) {
    if (!bb) return ILLEGAL_SQ; // __builtin_ctzll is undefined at 0
    return __builtin_ctzll(bb);
}

constexpr Rank get_rank(Square sq) { return Rank(sq >> 3); }
constexpr File get_file(Square sq) { return File(sq & 7); }
constexpr Bitboard get_rank_bb(Rank r) { return Rank1_const << (8 * r); }
constexpr Bitboard get_file_bb(File f) { return FileA_const << f; }

constexpr int sign_mask(int num) { return num >> 31; } // -1 if num<0, else 0



void print_bitboard(Bitboard bitboard);

template<Direction D>
constexpr Bitboard shift(Bitboard bb) {
    return D == NORTH         ? bb << 8
         : D == SOUTH         ? bb >> 8
         : D == NORTH + NORTH + WEST ? (bb & ~FileA_const) << 15
         : D == NORTH + NORTH + EAST ? (bb & ~FileH_const) << 17
         : D == SOUTH + SOUTH + EAST ? (bb & ~FileH_const) >> 15
         : D == SOUTH + SOUTH + WEST ? (bb & ~FileA_const) >> 17
         : D == EAST          ? (bb & ~FileH_const) << 1
         : D == WEST          ? (bb & ~FileA_const) >> 1
         : D == WEST + WEST + NORTH   ? (bb & ~FileAB_const) << 6
         : D == WEST + WEST + SOUTH   ? (bb & ~FileAB_const) >> 10
         : D == EAST + EAST + NORTH   ? (bb & ~FileGH_const) << 10
         : D == EAST + EAST + SOUTH   ? (bb & ~FileGH_const) >> 6
         : D == NORTH_EAST    ? (bb & ~FileH_const) << 9
         : D == NORTH_WEST    ? (bb & ~FileA_const) << 7
         : D == SOUTH_EAST    ? (bb & ~FileH_const) >> 7
         : D == SOUTH_WEST    ? (bb & ~FileA_const) >> 9
                              : 0;
}

template<Color C>
constexpr Bitboard generate_pawn_attacks(Bitboard bb){
    return C == WHITE ? shift<NORTH_WEST>(bb) | shift<NORTH_EAST>(bb)
                        : shift<SOUTH_WEST>(bb) | shift<SOUTH_EAST>(bb);
}

constexpr Bitboard generate_king_attacks(Bitboard bb){
    return shift<NORTH_WEST>(bb) | shift<NORTH>(bb) | shift<NORTH_EAST>(bb) | shift<SOUTH_EAST>(bb) | 
        shift<SOUTH>(bb) | shift<SOUTH_WEST>(bb) | shift<EAST>(bb) | shift<WEST>(bb);
}


//         noNoWe    noNoEa
//             +15  +17
//              |     |
// noWeWe  +6 __|     |__+10  noEaEa
//               \   /
//                >0<
//            __ /   \ __
// soWeWe -10   |     |   -6  soEaEa
//              |     |
//             -17  -15
//         soSoWe    soSoEa
constexpr Bitboard generate_knight_attacks(Bitboard bb){
    return shift<static_cast<Direction>(NORTH + NORTH + WEST)>(bb) |
      shift<static_cast<Direction>(NORTH + NORTH + EAST)>(bb) |  
      shift<static_cast<Direction>(SOUTH + SOUTH + WEST)>(bb) |
      shift<static_cast<Direction>(SOUTH + SOUTH + EAST)>(bb) | 
      shift<static_cast<Direction>(WEST + WEST + NORTH)>(bb) | 
      shift<static_cast<Direction>(WEST + WEST + SOUTH)>(bb) | 
      shift<static_cast<Direction>(EAST + EAST + NORTH)>(bb) | 
      shift<static_cast<Direction>(EAST + EAST + SOUTH)>(bb);
}


Bitboard rank_mask(Square sq);
Bitboard file_mask(Square sq);
Bitboard diag_mask(Square sq);
Bitboard anti_diag_mask(Square sq);


constexpr bool is_valid_square(Square sq) { return sq>=a1 && sq<=h8; }
constexpr bool is_safe_shift(Square sq, Direction d) {
  Square target_sq = (Square)(sq+d);
  return is_valid_square(target_sq) 
          && abs(get_file(sq) - get_file(target_sq)) <= 1;
}

// Bitboard generate_sliding_attacks(Piece pt, Square sq, Bitboard blockers);
// // generates rook/bishop occupancy masks. generate_sliding_attacks(...) wrapper
// Bitboard generate_sliding_mask(Piece piece, Square sq);
