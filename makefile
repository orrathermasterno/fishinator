all: 
	g++ -Ofast main.cpp board.cpp bitboard.cpp attacks.cpp movegen.cpp uci.cpp -o fishinator