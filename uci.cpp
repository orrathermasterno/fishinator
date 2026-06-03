#include "uci.hh"
#include "search.hh"

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
    ml.generate_all_legals(board);

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

        state_history.push_back(CopyMake());
        
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

void UCI::clear() {
    Searcher::clear();
}

void UCI::loop() {
    string line, command, command2;

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
        }

        else if (command == "isready")
            cout << "readyok" << endl;

        else if (command == "go") {
            string token;
            int depth = DEF_DEPTH;
            bool is_perft = false;

            while (is >> skipws >> token) {
                if (token == "depth") {
                    is >> skipws >> depth;
                } 
                else if (token == "perft") {
                    is_perft = true;
                    is >> skipws >> depth; 
                }
            }

            if (is_perft) {
                uint64_t nodes = Perft<true>(board, depth);
                cout << "total nodes: " << nodes << endl;
            } 
            else {
                Move bestmove = Move::empty_move();
                for (int d = 1; d <= depth; d++) {
                    bestmove = Searcher::root_alphabeta(board, d);
                }
                cout << "bestmove " << bestmove.move_to_str(board.ActiveColor) << endl;
            }
        }

        else if (command == "ucinewgame") {
            clear();
        }

    } while (command != "quit");
}