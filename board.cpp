#include "board.hh"
#include <cstring>
#include <sstream>
#include <cctype>
#include <string>
#include "attacks.hh"

constexpr std::string_view EncodedPieces("PNBRQKxxpnbrqk");

void Board::clear() {
    ActiveColor = WHITE;
    Ply = 0;

    root_state = BoardState();
    bs = &root_state;

    std::fill(Mailbox, Mailbox + SQ_AMOUNT, NO_PIECE);
    std::fill(PieceBB, PieceBB + ALL_PIECES, 0ULL);
    std::fill(ColorBB, ColorBB + COLOR_NB, 0ULL);
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
bool Board::is_attacked(int sq, Bitboard blockers, Color color) {
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
    ss >> pieces >> color >> castling >> enPassant >> Ply;
    
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
}

void Board::move_piece(int from_square, int to_square, ColoredPiece p) {
    set_piece<REMOVE_PIECE>(p, from_square);
    set_piece<ADD_PIECE>(p, to_square);
}

void Board::make_promotion(int from_square, int to_square, ColoredPiece piece_to) {
    set_piece<REMOVE_PIECE>(make_colored_piece(ActiveColor, PAWN), from_square);
    set_piece<ADD_PIECE>(piece_to, to_square);
}

void Board::castle(bool kingside) {
    if (kingside) {
        if(ActiveColor == WHITE) move_piece(h1, f1, W_ROOK);

        else move_piece(h8, f8, B_ROOK);
    }

    else { // queenside
        if(ActiveColor == WHITE) move_piece(a1, d1, W_ROOK);

        else move_piece(a8, d8, B_ROOK);
    }
}

// TODO: repetiotion checks, 50-move counter
void Board::make_move(Move& move, BoardState& new_state) {

    int from_square = move.getFrom();
    int to_square = move.getTo();
    int flag = move.getFlags();
    ColoredPiece moved_piece = Mailbox[from_square];
    ColoredPiece captured_piece = move.is_ep()? make_colored_piece(Color(ActiveColor^1), PAWN) : Mailbox[to_square];
    bool capture = (captured_piece != NO_PIECE);

    new_state = *bs;
    new_state.Previous = bs;
    bs = &new_state;
    new_state.Castling = CastlingRights(uint8_t(new_state.Castling) & CastlingMasks[from_square] & CastlingMasks[to_square]);
    new_state.CapturedPiece = captured_piece;
    new_state.EnPassant = ILLEGAL_SQ;

    // premove routine
    if(!capture) {
        if (move.is_double_push()) { // if move was a double push, additionally update EnPassant state
            Direction push = ActiveColor == WHITE ? NORTH : SOUTH;
            new_state.EnPassant = to_square - push;
        }

        if (move.is_castle()) {
            castle(move.is_king_castle()); // moves rook
        }
    }
    else { // capture
        if(move.is_ep()) {
            Direction push = ActiveColor == WHITE ? NORTH : SOUTH;
            set_piece<REMOVE_PIECE>(captured_piece, to_square-push);
        }
        else set_piece<REMOVE_PIECE>(captured_piece, to_square);
    }

    // the move itself
    if (!move.is_promotion()) move_piece(from_square, to_square, moved_piece);
    else {
        ColoredPiece prom_to = move.get_promotion_type(ActiveColor);
        make_promotion(from_square, to_square, prom_to);
    }

    ActiveColor = Color(ActiveColor ^ 1);
    Ply++;
}

void Board::unmake_move(Move& move) {
    int from_square = move.getTo();
    int to_square = move.getFrom();

    ActiveColor = Color(ActiveColor ^ 1);
    bs = bs->Previous;
    Ply--;
}