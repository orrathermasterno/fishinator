#pragma once
#include "board.hh"
#include "evaluation.hh"
#include <cstring>

constexpr int HASHSIZE = 4096;
constexpr int HASHMASK = HASHSIZE-1;

enum NodeType: uint8_t {
    PV_NODE, // Score is Exact
    ALL_NODE, // Score is Upper Bound
    CUT_NODE,  // Score is Lower Bound
    NONENODE
};

struct TTEntry {
    PositionKey Key;
    Score Value;
    NodeType Type;
    uint8_t Depth;
    Move BestMove;

    TTEntry() : Key(0), Value(0), Type(NONENODE), Depth(0), BestMove(Move::empty_move()) {}

    TTEntry(
        PositionKey key,
        int score,
        NodeType nodetype,
        int depth,
        Move bestmove
    ): Key(key), Value(score), Type(nodetype), Depth(depth), BestMove(bestmove) {}
};

struct TTBucket {
    TTEntry DepthSchemed;   
    TTEntry AlwaysSchemed;
};

class TTable {
    TTBucket Table[HASHSIZE];

public:
    inline void clear() {
        std::memset(Table, 0, sizeof(Table)); 
    }

    inline bool get_entry(PositionKey key, TTEntry& result) {
        int index = key & HASHMASK;
        if (Table[index].DepthSchemed.Key == key) {
            result = Table[index].DepthSchemed;
            return true;
        }
        if (Table[index].AlwaysSchemed.Key == key) {
            result = Table[index].AlwaysSchemed;
            return true;
        }

        return false;
    }

    inline void store_entry(const TTEntry& e) {
        int index = e.Key & HASHMASK;

        if (Table[index].DepthSchemed.Depth <= e.Depth) {
            Table[index].DepthSchemed = e;
        }
        else {
            Table[index].AlwaysSchemed = e;
        }
        return;
    }
};