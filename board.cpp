#include "board.hh"
#include <cstring>
#include <sstream>
#include <cctype>
#include <string>
#include "attacks.hh"
#include "prng.hh"
#include "movegen.hh"

static constexpr ColoredPiece ColoredPieces[] = {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                                   B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING};


void Board::clear() {
    ActiveColor = WHITE;
    Ply = 0;

    root_state = BoardState();
    bs = &root_state;

    std::fill(Mailbox, Mailbox + SQ_AMOUNT, NO_CPIECE);
    std::fill(PieceBB, PieceBB + ALL_PIECES, 0ULL);
    std::fill(ColorBB, ColorBB + COLOR_NB, 0ULL);

    pins_calculated[WHITE] = 0; pins_calculated[BLACK] = 0; 
}


// Zobrist initialization:
//  1. One number for each piece at each square
//  2. One number to indicate the side to move is black
//  3. Sixteen numbers for castling rights
//  4. Eight numbers to indicate the file of a valid En passant square, if any
PositionKey Zobrist::piece_on_sq[NO_CPIECE][ILLEGAL_SQ];
PositionKey Zobrist::black_to_move;
PositionKey Zobrist::castling[CASTLING_STATES_NUM];
PositionKey Zobrist::enpassant[FILE_NUM];

void Zobrist::init(){
    Xorshift prng = Xorshift(10227);
    // 1
    for (ColoredPiece pc : ColoredPieces) {
        for (int s = a1; s <= h8; ++s)
            piece_on_sq[pc][s] = prng.rand64();
    }
    // 2
    black_to_move = prng.rand64();
    // 3
    for (int cr = NO_CASTLING; cr < CASTLING_STATES_NUM; cr++)
        castling[cr] = prng.rand64();
    // 4
    for (int file = FILE_A; file <= FILE_H; file++)
        enpassant[file] = prng.rand64();
}

void Board::print_board_state()
{
    printf("\n");
    for (int rank = RANK_8; rank >= RANK_1; --rank)
    {
        printf("%d  ", rank + 1);
        for (int file = FILE_A; file <= FILE_H; ++file)
        {
            int square = rank * 8 + file;

            int piece = -1;
            
            for (int bb_piece = PAWN; bb_piece <= KING; bb_piece++) 
            {
                if (get_bit(PieceBB[bb_piece], square)) { // ugly
                    piece = bb_piece;
                    if (get_bit(ColorBB[BLACK], square))
                        piece += 8;
                    break;
                }
            }
            printf(" %c", (piece == -1) ? '.' : EncodedPieces[piece]);
        }
        
        printf("\n");
    }

    printf("\n    a b c d e f g h\n\n");

    printf("ActiveColor: %s\n", !ActiveColor ? "white" : "black");
    
    printf("En passant: %i\n", bs->EnPassant);
    
    printf("Castling: %c%c%c%c\n\n", (bs->Castling & WHITE_OO) ? 'K' : '-',
                                           (bs->Castling & WHITE_OOO) ? 'Q' : '-',
                                           (bs->Castling & BLACK_OO) ? 'k' : '-',
                                               (bs->Castling & BLACK_OOO) ? 'q' : '-');

    printf("Ply: %i\n", Ply);
}

// returns a bitboard containing pieces eyeing target square 
// to get actual attackers one still has to intersect with color boards 
Bitboard Board::attackers_of(int sq, Bitboard blockers) const {
    return (Attacks::get_pawn_attack(sq, WHITE) & get_colored_piece_bb<PAWN>(BLACK))
        | (Attacks::get_pawn_attack(sq, BLACK) & get_colored_piece_bb<PAWN>(WHITE))
        | (Attacks::get_knight_attack(sq) & get_piece_bb<KNIGHT>())
        | (Attacks::get_king_attack(sq) & get_piece_bb<KING>())
        | (Attacks::get_rook_attack(blockers, sq) & get_joint_piece_bb(ROOK, QUEEN))
        | (Attacks::get_bishop_attack(blockers, sq) & get_joint_piece_bb(BISHOP, QUEEN))
    ;
}

// like Attacked attacked, not just eyed 
// color param stands for attacker color
// ideally checks should be ordered according to probability of a piece giving check but rn they're sorted based on vibes
bool Board::is_attacked(int sq, Bitboard blockers, Color color) const {
    return (Attacks::get_rook_attack(blockers, sq) & get_colored_joint_piece_bb(color, ROOK, QUEEN))
        || (Attacks::get_bishop_attack(blockers, sq) & get_colored_joint_piece_bb(color, BISHOP, QUEEN))
        || (Attacks::get_knight_attack(sq) & get_colored_piece_bb<KNIGHT>(color))
        || (Attacks::get_pawn_attack(sq, Color(!color)) & get_colored_piece_bb<PAWN>(color))
        || (Attacks::get_king_attack(sq) & get_colored_piece_bb<KING>(color))
    ;
}

void Board::parse_FEN(const std::string& fen) {
    // Example FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    // 1. Piece placement on squares (A8 B8 .. G1 H1) 
    //    Each piece is identified by a letter taken from the standard English names (white upper-case, black lower-case). 
    //    Blank squares are noted using digits 1 through 8 (the number of blank squares), and "/" separate ranks.
    // 2. Active color. "w" means white moves next, "b" means black.
    // 3. Castling availability. Either - if no side can castle or a letter (K,Q,k,q) for each side and castle possibility.
    // 4. En passant target square in algebraic notation or "-".
    // 5. Halfmove clock: This is the number of halfmoves since the last pawn advance or capture.
    // 6. Fullmove number: The number of the current full move.
    
    clear();

    int sq = a8;
    size_t piece;

    std::stringstream ss(fen);
    std::string pieces, color, castling, enPassant; 

    // 5-6.
    ss >> pieces >> color >> castling >> enPassant >> bs->HalfmoveClock >> Ply;
    
    // 1.
    for (char token : pieces) {
        if(isdigit(token)) {
            sq += (token - '0') * EAST;
        }

        else if (token == '/') {
            sq += 2 * SOUTH;
        }
        else if ((piece = EncodedPieces.find(token)) != std::string::npos) {
            set_piece<ADD_PIECE>(ColoredPiece(piece), sq);
            sq++;
        }
    }
    
    // 2.
    ActiveColor = color == "w" ? WHITE : BLACK; 
    Ply = std::max(2 * (Ply - 1), 0) + (ActiveColor == BLACK);

    // 3.
    bs->Castling = CastlingRights(0);

    for (char token : castling) {
        switch (token) {
            case 'K': bs->Castling = CastlingRights(bs->Castling | WHITE_OO); break; 
            case 'Q': bs->Castling = CastlingRights(bs->Castling | WHITE_OOO); break; 
            case 'k': bs->Castling = CastlingRights(bs->Castling | BLACK_OO); break;
            case 'q': bs->Castling = CastlingRights(bs->Castling | BLACK_OOO); break; 
            case '-': break; 
        }
    }

    // 4.
    if (enPassant != "-") {
        int file = enPassant.at(0) - 'a';
        int rank = enPassant.at(1) - '1';
        bs->EnPassant = Square(rank*8 + file);
    }

    set_pinners_and_blockers();

    // compute position key
    Bitboard all_pieces_bb = get_color_bb(BOTH);
    while(all_pieces_bb) {
        int sq = pop_lsb(all_pieces_bb);
        int c_piece = Mailbox[sq];

        bs->Key ^= Zobrist::piece_on_sq[c_piece][sq];
    }

    if (ActiveColor == BLACK) bs->Key ^= Zobrist::black_to_move;
    if (bs->EnPassant != ILLEGAL_SQ) bs->Key ^= Zobrist::enpassant[get_file(bs->EnPassant)];
    bs->Key ^= Zobrist::castling[bs->Castling];
}

template<MoveSwitch sw>
void Board::move_piece(int from_square, int to_square, ColoredPiece p) {
    if constexpr (sw == BACK) {
        std::swap(to_square, from_square);
    }

    set_piece<REMOVE_PIECE>(p, from_square);
    set_piece<ADD_PIECE>(p, to_square);
}

template<MoveSwitch sw>
void Board::make_promotion(int from_square, int to_square, ColoredPiece piece_to) {
    if constexpr (sw == FORWARD) {
        set_piece<REMOVE_PIECE>(make_colored_piece(ActiveColor, PAWN), from_square);
        set_piece<ADD_PIECE>(piece_to, to_square);
    }
    else {
        set_piece<REMOVE_PIECE>(piece_to, to_square);
        set_piece<ADD_PIECE>(make_colored_piece(ActiveColor, PAWN), from_square);
    }
}

template<MoveSwitch sw>
void Board::castle(ColoredPiece& crook, int& rook_sq_from, int& rook_sq_to, bool kingside) {
    crook = ActiveColor == WHITE ? W_ROOK : B_ROOK;

    int rank_offset = ActiveColor * 56; 

    if (kingside) {
        rook_sq_from = h1 + rank_offset;
        rook_sq_to   = f1 + rank_offset;
    } 
    else { // queenside
        rook_sq_from = a1 + rank_offset;
        rook_sq_to   = d1 + rank_offset;
    }

    move_piece<sw>(rook_sq_from, rook_sq_to, crook);
}

bool Board::castling_path_is_safe(int king_sq, int rook_sq, bool IsKingside) const {
    Bitboard in_between = Attacks::get_between_sq_bb(king_sq, rook_sq);

    if(!IsKingside) {
        pop_lsb(in_between);
    } 

    int btw_sq;
    while (in_between) {
        btw_sq = pop_lsb(in_between);
        
        if(is_attacked(btw_sq, ColorBB[BOTH], Color(ActiveColor^1))) {
            return false; 
        }
    }

    return true; 
}

int Board::is_pinned_by_old(int sq) { 
    int king_sq = get_king_sq(ActiveColor);

    Bitboard slider_attackers = enemy_attackers_of(king_sq, ColorBB[~ActiveColor], ActiveColor)
                                & get_colored_joint_piece_bb(Color(~ActiveColor), ROOK, QUEEN, BISHOP);
    int attacker_sq;
    Bitboard pinned_bb = set_bit(0ULL, sq);
    Bitboard path;

    while(slider_attackers) {
        attacker_sq = pop_lsb(slider_attackers);
        path = Attacks::get_between_sq_bb(king_sq, attacker_sq);

        if((path & pinned_bb) && !more_than_one(path & ColorBB[ActiveColor])) 
            return attacker_sq;
    }

    return ILLEGAL_SQ;
}

void Board::set_pinners_and_blockers(Color ColorToUpdate) const {
    pinners[~ColorToUpdate] = 0ULL;
    blockers[ColorToUpdate] = 0ULL;
    pins_calculated[ColorToUpdate] = true;

    int king_sq = get_king_sq(ColorToUpdate);

    Bitboard slider_attackers = enemy_attackers_of(king_sq, ColorBB[~ColorToUpdate], ColorToUpdate)
                                & get_colored_joint_piece_bb(Color(~ColorToUpdate), ROOK, QUEEN, BISHOP);

    int attacker_sq; Bitboard path, blockers_bb;

    while(slider_attackers) {
        attacker_sq = pop_lsb(slider_attackers);
        path = Attacks::get_between_sq_bb(king_sq, attacker_sq);
        blockers_bb = path & ColorBB[ColorToUpdate];

        if (population_count(blockers_bb) == 1) {
            pinners[~ColorToUpdate] |= set_bit(0ULL, attacker_sq); 
            blockers[ColorToUpdate]|= blockers_bb;
        }
    }
}

void Board::make_move(Move& move, BoardState& new_state) {

    int from_square = move.getFrom();
    int to_square = move.getTo();
    int captured_square = to_square; // changes in case of ep

    ColoredPiece moved_piece = Mailbox[from_square];
    ColoredPiece captured_piece = move.is_ep()? make_colored_piece(Color(ActiveColor^1), PAWN) : Mailbox[to_square];
    bool capture = move.is_capture();

    new_state = *bs;
    new_state.Previous = bs;
    bs = &new_state;

    new_state.Castling = CastlingRights(uint8_t(new_state.Castling) & CastlingMasks[from_square] & CastlingMasks[to_square]);
    new_state.CapturedPiece = captured_piece;
    new_state.EnPassant = ILLEGAL_SQ;
    pins_calculated[WHITE]=false; pins_calculated[BLACK]=false;
    new_state.HalfmoveClock = (capture || type_of(moved_piece) == PAWN) ? 0 : new_state.Previous->HalfmoveClock+1;
    new_state.Repetition = 0;

    PositionKey new_key = new_state.Previous->Key ^ Zobrist::black_to_move;

    // update castling and ep for new key
    new_key ^= Zobrist::castling[new_state.Previous->Castling];
    new_key ^= Zobrist::castling[new_state.Castling];
    if (new_state.Previous->EnPassant != ILLEGAL_SQ) 
        new_key ^= Zobrist::enpassant[get_file(new_state.Previous->EnPassant)];

    // premove routine
    if(!capture) {
        if (move.is_double_push()) { // if move was a double push, additionally update EnPassant state
            int push = pawn_push_direction(ActiveColor);
            new_state.EnPassant = to_square - push;
            new_key ^= Zobrist::enpassant[get_file(new_state.EnPassant)];
        }

        if (move.is_castle()) {
            int rook_sq_from, rook_sq_to;
            ColoredPiece crook;
            castle<FORWARD>(crook, rook_sq_from, rook_sq_to, move.is_king_castle()); // moves rook
            new_key ^= Zobrist::piece_on_sq[crook][rook_sq_from] ^ Zobrist::piece_on_sq[crook][rook_sq_to]; 
        }
    }
    else { // capture
        if(move.is_ep()) {
            int push =  pawn_push_direction(ActiveColor);
            captured_square = to_square-push;
            set_piece<REMOVE_PIECE>(captured_piece, captured_square);
        }
        else set_piece<REMOVE_PIECE>(captured_piece, to_square);

        new_key ^= Zobrist::piece_on_sq[captured_piece][captured_square];
    }

    // the move itself
    new_key ^= Zobrist::piece_on_sq[moved_piece][from_square];
    if (!move.is_promotion()) move_piece<FORWARD>(from_square, to_square, moved_piece);
    else {
        // ColoredPiece prom_to = move.get_promotion_type(ActiveColor);
        moved_piece = move.get_promotion_type(ActiveColor);
        make_promotion<FORWARD>(from_square, to_square, moved_piece);
    }
    new_key ^= Zobrist::piece_on_sq[moved_piece][to_square];

    ActiveColor = Color(ActiveColor ^ 1);
    Ply++;
    new_state.Key = new_key;

    // repetition eager eval
    if (bs->HalfmoveClock >= 4) { // position cannot repeat in less than 4 moves
        BoardState* prevprev_state = bs->Previous->Previous;
        for (int i = 4; i <= bs->HalfmoveClock; i += 2) {
            prevprev_state = prevprev_state->Previous->Previous; // i++ going from current move backwards
            if(prevprev_state->Key == bs->Key) {
                bs->Repetition = prevprev_state->Repetition ? -1 : i; // possible technical debt when it comes to tt
                break;
            }
        }
    }
}

void Board::unmake_move(Move& move) {
    ActiveColor = Color(ActiveColor ^ 1);
    Ply--;

    int from_square = move.getFrom();
    int to_square = move.getTo();

    ColoredPiece moved_piece = Mailbox[to_square];
    ColoredPiece captured_piece = bs->CapturedPiece;
    bool capture = (captured_piece != NO_CPIECE);

    // the move itself
    if (!move.is_promotion()) move_piece<BACK>(from_square, to_square, moved_piece);
    else {
        ColoredPiece prom_to = move.get_promotion_type(ActiveColor);
        make_promotion<BACK>(from_square, to_square, prom_to);
    }

    // premove routine
    if (!capture && move.is_castle()) {
        int dummy1, dummy2; ColoredPiece dummy3;
        castle<BACK>(dummy3, dummy1, dummy2, move.is_king_castle());
    }
    else if (capture) {
        if(move.is_ep()) {
            int push = pawn_push_direction(ActiveColor);
            set_piece<ADD_PIECE>(captured_piece, to_square-push);
        }
        else set_piece<ADD_PIECE>(captured_piece, to_square);
    }

    bs = bs->Previous;
    pins_calculated[WHITE] = false;
    pins_calculated[BLACK] = false;
}

bool Board::legal(Move& move) const { 
    int from_sq = move.getFrom();
    int to_sq = move.getTo();
    int king_sq = get_king_sq(ActiveColor);

    Bitboard to_sq_bb = set_bit(0ULL, to_sq);

    // en passant requires special horizontal pin test of both involved pawns, which disappear from the same rank 
    if (move.is_ep()) {
        int capsq = to_sq - pawn_push_direction(ActiveColor);
        
        Bitboard occupied = (ColorBB[BOTH] ^ set_bit(0ULL, from_sq) ^ set_bit(0ULL, capsq)) | to_sq_bb;

        return !(Attacks::get_rook_attack(occupied, king_sq) & get_colored_joint_piece_bb(Color(ActiveColor^1), QUEEN, ROOK))
            && !(Attacks::get_bishop_attack(occupied, king_sq) & get_colored_joint_piece_bb(Color(ActiveColor^1), QUEEN, BISHOP));
    }

    // king shall not step onto attacked sqs
    if(type_of(Mailbox[from_sq]) == KING) {
        if (move.is_castle()) { 
            if (king_in_check()) return false;
            
            return castling_path_is_safe(from_sq, get_castling_rook_sq(move.is_king_castle()), move.is_king_castle());
        }
        else {
            Bitboard occ_without_king = ColorBB[BOTH] ^ set_bit(0ULL, from_sq);
            if(is_attacked(to_sq, occ_without_king, Color(ActiveColor^1))) return false;
            else return true;
        }
    }

    // the moving piece is not absolutely pinned on its move direction
    if (is_pinned(from_sq, ActiveColor) && !(Attacks::get_ray_sq_bb(king_sq, from_sq) & to_sq_bb)) return false;
    
    // non-king evasions
    // // double check handled by movegen
    else if (king_in_check()) { 
        Bitboard attackers = enemy_attackers_of(king_sq, ColorBB[BOTH], ActiveColor);

        // capture checker?
        if (attackers & to_sq_bb) return true;
        
        // block checker?
        int attacker_sq = pop_lsb(attackers); 
        if (Attacks::get_between_sq_bb(attacker_sq, king_sq) & to_sq_bb) return true;
        
        return false; // neither
    }

    return true;
}

bool Board::pseudolegal(Move& move) const {
    Color us = ActiveColor;
    int from = move.getFrom();
    int to = move.getTo();
    ColoredPiece moved_piece = get_piece_from_sq(from);
    Bitboard to_bb = set_bit(0ULL, to);

    if ((get_color_bb(us) & to_bb) 
        || !(get_color_bb(us) & set_bit(0ULL, from)))
        return false;

    if ((move.is_ep() || move.is_promotion()) && type_of(moved_piece) != PAWN) 
        return false;

    if (move.is_ep()) {
        if (get_ep() == ILLEGAL_SQ || to != get_ep()) return false;
        goto checkflag;
    }

    if (move.is_promotion()) {
        if ((relative_rank(us, get_rank(from)) != RANK_7) 
            || (relative_rank(us, get_rank(to)) != RANK_8)) return false;
        goto checkflag;
    }

    if (move.is_castle()) {
        if (king_in_check()) return false;
        MoveList ml = MoveList(); 
        
        if (us == WHITE) ml.generate_castling<WHITE>(*this);
        else ml.generate_castling<BLACK>(*this);
        
        for (const auto& m : ml)
            if (move == m)
                return true;
                
        return false;
    }

    if (type_of(moved_piece) == PAWN) {
        if ((Rank1_const | Rank8_const) & to_bb)
            return false;

        const bool captures   = bool(Attacks::get_pawn_attack(from, us) & get_color_bb(~us) & to_bb);
        const bool singlepush = (from + pawn_push_direction(us) == to) && !sq_occupied(to);
        const bool doublepush = (from + 2 * pawn_push_direction(us) == to)
                               && (relative_rank(us, get_rank(from)) == RANK_2) && !sq_occupied(to)
                               && !sq_occupied(to - pawn_push_direction(us));

        if (!(captures || singlepush || doublepush))
            return false;
    }
    else if (!(Attacks::get_attack_of(type_of(moved_piece), from, get_color_bb(BOTH)) & to_bb))
        return false;

checkflag:
    if (king_in_check()) {
        Bitboard king_attackers = get_checkers(us, ColorBB[BOTH]);
        int king_sq = get_king_sq(us);
        
        if (type_of(moved_piece) != KING) {
            if (more_than_one(king_attackers)) return false; 

            int attacker_sq = pop_lsb(king_attackers);
            Bitboard targets = set_bit(Attacks::get_between_sq_bb(attacker_sq, king_sq), attacker_sq);

            if (move.is_ep()) {
                int captured_pawn_sq = to - pawn_push_direction(us);
                if (!(targets & set_bit(0ULL, captured_pawn_sq))) return false;
            } else {
                if (!(targets & to_bb)) return false; 
            }
        }
        else {
            Bitboard occupancy_without_king = get_color_bb(BOTH) ^ set_bit(0ULL, from);
            if (is_attacked(to, occupancy_without_king, ~us)) return false;
        }
    }

    return true;
}


// explicit instantiations
template void Board::move_piece<FORWARD>(int, int, ColoredPiece);
template void Board::move_piece<BACK>(int, int, ColoredPiece);

template void Board::make_promotion<FORWARD>(int, int, ColoredPiece);
template void Board::make_promotion<BACK>(int, int, ColoredPiece);

template void Board::castle<FORWARD>(ColoredPiece&, int&, int&, bool);
template void Board::castle<BACK>(ColoredPiece&, int&, int&, bool);