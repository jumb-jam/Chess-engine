#pragma once
#include "board.h"
#include "book.h"
#include <unordered_map>

#include <chrono>
using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

#define max_ply 64



enum NodeType{
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

struct TTEntry{
    int depth;
    int score;
    int generation;
    NodeType flag;

    Move bestMove;
};

class Engine{
private:
    TimePoint searchStart;

    int       timeLimitMs = -1;

    bool      outOfTime   = false;

    bool      check_time();

    int nullMoveCuts = 0;

    int get_mvv_lva(Board& board,const Move& m);

    std::unordered_map<uint64_t,TTEntry> tt; //transposition table

    Move killerMoves[max_ply][2];

    int historyTable[2][64][64];

    int currentgeneration=0;
    
public:

    bool sameMove(const Move&a, const Move&b);

    Engine();

    OpeningBook openingBook;    

    long long nodes=0;
    long long qnodes=0;

    int alphabeta(Board& board, int depth, int alpha, int beta, int ply, bool isNullMove);

    int quiescence(Board& board, int alpha, int beta);

    Move bestMoveOverall{};
    bool hasPreviousBest = false;

    Move find_best_move(Board& board, int depth);

    Move find_best_move_timed(Board& board, int timeLimitMs);

    
};
