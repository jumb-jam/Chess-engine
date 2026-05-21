#pragma once
#include "board.h"
#include "book.h"
#include <unordered_map>

enum NodeType{
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TTEntry{
    int depth;
    int score;
    NodeType flag;

    Move bestMove;
};

class Engine{
private:

    int get_mvv_lva(Board& board,const Move& m);

    std::unordered_map<uint64_t,TTEntry> tt; //transposition table   
    
public:

    OpeningBook openingBook;    

    long long nodes=0;

    int alphabeta(Board& board, int depth, int alpha, int beta);

    int quiescence(Board& board, int alpha, int beta, int depth);

    Move bestMoveOverall{};
    bool hasPreviousBest = false;

    Move find_best_move(Board& board, int depth);

    
};
