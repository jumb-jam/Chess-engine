#pragma once
#include "board.h"

class Engine{
public:

    int minimax(Board& board, int depth);

    Move find_best_move(Board& board, int depth);
};