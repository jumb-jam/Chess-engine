#pragma once
#include "board.h"

class Engine{
public:

    int alphabeta(Board& board, int depth, int alpha, int beta);

    Move find_best_move(Board& board, int depth);
};