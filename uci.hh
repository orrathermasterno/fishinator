#pragma once
#include <string>    
#include <sstream>   
#include <iostream>
#include "board.hh"
#include "movegen.hh"
#include <vector>

using std::cout, std::skipws, std::istringstream, std::string, std::vector, std::endl;

class UCI {
    inline static Board board;
    inline static vector<BoardState> state_history;
public:

    static void loop();

    static void parse_position(istringstream& is);
    static void set_position(Board& board, const std::string& fen, const std::vector<std::string>& moves);
    static Move parse_move(Board& board, std::string move);
    static string to_lower(std::string str);

};