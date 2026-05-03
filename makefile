all: 
	g++ -O3 main.cpp board.cpp bitboard.cpp attacks.cpp movegen.cpp uci.cpp evaluation.cpp search.cpp scorer.cpp -o fishinator

bench:
	g++ -O3 -DBENCH main.cpp board.cpp bitboard.cpp attacks.cpp movegen.cpp uci.cpp evaluation.cpp search.cpp scorer.cpp -o fishinator_bench