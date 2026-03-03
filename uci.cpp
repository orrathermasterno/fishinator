#include "uci.hh"

/**********************************\
==================================

        !!! revisit later

==================================
\**********************************/

string UCI::to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](auto c) { return std::tolower(c); });

    return str;
}

Move UCI::parse_move(Board& board, std::string move) {
    move = to_lower(move);

    MoveList ml = MoveList();
    if (board.ActiveColor == WHITE) {
        ml.generate_pseudolegals<QUIET_AND_CAPTURE, WHITE>(board);
    }
    else {
        ml.generate_pseudolegals<QUIET_AND_CAPTURE, BLACK>(board);
    }

    for (const auto& m : ml)
        if (move == m.move_to_str(board.ActiveColor))
            return m;

    return Move::empty_move();
}

void UCI::set_position(Board& board, const std::string& fen, const std::vector<std::string>& moves) {
    if (!fen.empty()) {
        board.parse_FEN(fen);
    }

    state_history.clear();
    state_history.reserve(moves.size());

    for (const auto& move : moves)
    {
        Move m = parse_move(board, move);

        if (m.is_empty())
            break;

        state_history.push_back(BoardState());
        
        board.make_move(m, state_history.back());
    }
}

void UCI::parse_position(istringstream& is) {
    string command, fen;

    is >> command;

    if (command == "startpos")
    {
        fen = START_FEN;
        is >> command;  
    }
    else if (command == "fen")
        while (is >> command && command != "moves")
            fen += command + " ";
    else
        return;

    vector<std::string> moves;

    while (is >> command)
    {
        moves.push_back(command);
    }

    set_position(board, fen, moves);
}

void UCI::loop() {
    string line, command;

    do {
        if (!std::getline(std::cin, line)) {
            command = "quit";
            break; 
        }

        istringstream is(line);

        command.clear();
        is >> skipws >> command;

        if (command == "uci") {
            cout << "id name fishinator" << endl;
            cout << "uciok" << endl;
        }

        else if (command == "position") {
            parse_position(is);
            board.print_board_state();
        }

        else if (command == "isready")
            cout << "readyok" << endl;

        else if (command == "go") {
            is >> skipws >> command;
            if (command == "perft") {
                int depth = 4;
                is >> skipws >> depth;

                uint64_t perft = Perft<true>(board, depth);
                cout << "total nodes: " << perft << endl;

            }
            else 
                cout << "bestmove e2e4" << endl;
        }

    } while (command != "quit");
}