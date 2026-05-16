#pragma once
#include "board.h"

class Engine{
public:
    long long nodes=0;

    int alphabeta(Board& board, int depth, int alpha, int beta);

    int quiescence(Board& board, int alpha, int beta, int depth);

    Move find_best_move(Board& board, int depth);
};