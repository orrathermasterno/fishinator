#include "board.hh"
#include <cstring>
#include <sstream>
#include <cctype>
#include <string>
#include "attacks.hh"

Bitboard Board::PieceBB[ALL_PIECES];
Bitboard Board::ColorBB[INVALID_COLOR];

constexpr std::string_view EncodedPieces("PNBRQKxxpnbrqk");

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
                if (get_bit(Board::PieceBB[bb_piece], square)) { // ugly
                    piece = bb_piece;
                    if (get_bit(Board::ColorBB[BLACK], square))
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
    
    printf("En passant: %i\n", EnPassant);
    
    printf("Castling: %c%c%c%c\n\n", (Castling & WHITE_OO) ? 'K' : '-',
                                           (Castling & WHITE_OOO) ? 'Q' : '-',
                                           (Castling & BLACK_OO) ? 'k' : '-',
                                               (Castling & BLACK_OOO) ? 'q' : '-');

    printf("Ply: %i\n", Ply);

    printf("FullMove: %i\n", FullMove);
}

// to be improved 
void Board::set_piece(ColoredPiece p, int sq) {

    Bitboard square_bb = set_bit(0ULL, sq);

    PieceBB[type_of(p)] |= square_bb;
    ColorBB[color_of(p)] |= square_bb;

    ColorBB[BOTH] |=  square_bb;
}

// returns a bitboard containing pieces eyeing target square 
// to get actual attackers one still has to intersect with color boards 
Bitboard Board::attackers_of(int sq, Bitboard blockers) {
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
    
    std::memset(reinterpret_cast<char*>(this), 0, sizeof(Board));

    int sq = a8;
    size_t piece;

    std::stringstream ss(fen);
    std::string pieces, color, castling, enPassant; 

    // 5-6.
    ss >> pieces >> color >> castling >> enPassant >> Ply >> FullMove;
    
    // 1.
    for (char token : pieces) {
        if(isdigit(token)) {
            sq += (token - '0') * EAST;
        }

        else if (token == '/') {
            sq += 2 * SOUTH;
        }
        else if ((piece = EncodedPieces.find(token)) != std::string::npos) {
            set_piece(ColoredPiece(piece), sq);
            sq++;
        }
    }
    
    // 2.
    ActiveColor = color == "w" ? WHITE : BLACK; 

    // 3.
    Castling = CastlingRights(0); // Clear it first!

    for (char token : castling) {
        switch (token) {
            case 'K': Castling = CastlingRights(Castling | WHITE_OO); break; 
            case 'Q': Castling = CastlingRights(Castling | WHITE_OOO); break; 
            case 'k': Castling = CastlingRights(Castling | BLACK_OO); break;
            case 'q': Castling = CastlingRights(Castling | BLACK_OOO); break; 
            case '-': break; 
        }
    }

    // 4.
    if (enPassant != "-") {
        int file = enPassant.at(0) - 'a';
        int rank = enPassant.at(1) - '1';
        EnPassant = Square(rank*8 + file);
    }
}