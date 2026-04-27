#pragma once

#include "bitboard.hh"
#include "attacks.hh"
#include <iostream>
#include <algorithm>
#include <cassert>
#include "move.hh"

typedef uint64_t PositionKey;

enum SetPieceSwitch {
    ADD_PIECE, REMOVE_PIECE
};

enum MoveSwitch {
    FORWARD, BACK
};

struct Zobrist {
    static PositionKey piece_on_sq[NO_CPIECE][ILLEGAL_SQ];
    static PositionKey black_to_move;
    static PositionKey castling[CASTLING_STATES_NUM];
    static PositionKey enpassant[FILE_NUM];

    static void init();
};

// irreversible stuff for Copy-Make
struct BoardState {
    PositionKey Key;
    CastlingRights Castling;
    int EnPassant;
    ColoredPiece CapturedPiece;

    int HalfmoveClock;
    int Repetition;

    BoardState* Previous; 

    BoardState() {
        Key = 0; HalfmoveClock = 0; Repetition = 0;
        EnPassant = ILLEGAL_SQ;
        Castling = NO_CASTLING;
        Previous = nullptr;
        CapturedPiece = NO_CPIECE;
    }
};

class Board {
    BoardState root_state;
public:
    // piece boards 
    Bitboard PieceBB[ALL_PIECES];
    Bitboard ColorBB[COLOR_NB];
    ColoredPiece Mailbox[SQ_AMOUNT];

    Color ActiveColor;
    BoardState* bs;

    int Ply;

    mutable Bitboard pinners[2]; // pinners[BLACK] represents black pieces pinning some white pieces
    mutable Bitboard blockers[2]; // blockers[BLACK] represents black pieces being pinned by some white pieces
    mutable bool pins_calculated[2];


    void clear();

    Board() {
        clear();
    }

    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    void print_board_state();

    template<SetPieceSwitch sw>
    inline void set_piece(ColoredPiece p, int sq) { 
        if constexpr (sw == ADD_PIECE) {
            assert(Mailbox[sq] == NO_CPIECE);
            Mailbox[sq] = p;
        }
        else {
            assert(Mailbox[sq] == p);
            Mailbox[sq] = NO_CPIECE; // sw == REMOVE_PIECE
        }

        Bitboard square_bb = set_bit(0ULL, sq);

        PieceBB[type_of(p)] ^= square_bb;
        ColorBB[color_of(p)] ^= square_bb;

        ColorBB[BOTH] ^=  square_bb;
    }

    void parse_FEN(const std::string& fenStr);

    inline Bitboard get_color_bb(Color C) const{
        return ColorBB[C];
    }

    template<Piece P>
    inline Bitboard get_colored_piece_bb(Color C) const{
        return PieceBB[P] & ColorBB[C];
    }

    template<Piece P>
    inline Bitboard get_piece_bb() const{
        return PieceBB[P];
    }

    template<typename... Pieces>
    inline Bitboard get_colored_joint_piece_bb(Color C, Pieces... pts) const {
        return ColorBB[C] & (PieceBB[pts] | ...);
    }

    template<typename... Pieces>
    inline Bitboard get_joint_piece_bb(Pieces... pts) const {
        return (PieceBB[pts] | ...);
    }

    Bitboard attackers_of(int sq, Bitboard blockers) const;
    inline Bitboard enemy_attackers_of(int sq, Bitboard blockers, Color Color) const {
        return attackers_of(sq, blockers) & ColorBB[Color ^ 1];
    }

    inline Bitboard get_checkers(Color KingColor, Bitboard blockers) const {
        return enemy_attackers_of(get_king_sq(KingColor), blockers, KingColor);
    }

    bool is_attacked(int sq, Bitboard blockers, Color color) const;

    template<CastlingRights Cr>
    inline bool can_castle() const {
        if constexpr (Cr == WHITE_OO)
            return !(ColorBB[BOTH] & WHITE_OO_BLOCKERS);
            
        else if constexpr (Cr == WHITE_OOO)
            return !(ColorBB[BOTH] & WHITE_OOO_BLOCKERS);
            
        else if constexpr (Cr == BLACK_OO)
            return !(ColorBB[BOTH] & BLACK_OO_BLOCKERS);
            
        else if constexpr (Cr == BLACK_OOO)
            return !(ColorBB[BOTH] & BLACK_OOO_BLOCKERS);
            
        return false;
    }

    inline int get_king_sq(Color color) const { 
        return bit_scan_forward(PieceBB[KING] & ColorBB[color]);
    }

    inline ColoredPiece get_piece_from_sq(int sq) {
        return Mailbox[sq];
    }

    inline CastlingRights get_castling_rights() const{
        return bs->Castling;
    }

    inline int get_ep() const{
        return bs->EnPassant;
    }

    inline ColoredPiece get_captured_piece() const{
        return bs->CapturedPiece;
    }

    inline int get_castling_rook_sq(bool IsKingside) const {
        if (IsKingside) {
            return ActiveColor==WHITE? h1 : h8;
        }
        return ActiveColor==WHITE? a1 : a8;
    }

    int is_pinned_by_old(int sq);


    inline Bitboard get_blockers(Color side) const {
        if(!pins_calculated[side]) {
            set_pinners_and_blockers(side);
        }
        return blockers[side];
    }

    inline bool is_pinned(int sq, Color side) const {
        return get_blockers(side) & set_bit(0ULL, sq);
    }

    void set_pinners_and_blockers(Color ColorToUpdate) const;

    // update pinners and blockers for both sides
    inline void set_pinners_and_blockers() {
        set_pinners_and_blockers(WHITE);
        set_pinners_and_blockers(BLACK);
    }

    bool castling_path_is_safe(int king_sq, int rook_sq, bool IsKingside) const;

    inline bool king_in_check() const {
        return is_attacked(get_king_sq(ActiveColor), ColorBB[BOTH], Color(ActiveColor^1));
    }

    inline int pawn_push_direction(Color c) const {
        return c == WHITE ? NORTH : SOUTH;
    }

    template<MoveSwitch sw>
    void move_piece(int sq_from, int sq_to, ColoredPiece p);

    template<MoveSwitch sw>
    void make_promotion(int sq_from, int sq_to, ColoredPiece piece_to);

    template<MoveSwitch sw>
    void castle(ColoredPiece& crook, int& rook_sq_from, int& rook_sq_to, bool kingside);

    void make_move(Move& move, BoardState& new_state);
    void unmake_move(Move& move);

    bool legal(Move& move) const;
};