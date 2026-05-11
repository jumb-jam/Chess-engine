#include "engine.h"

int Engine::minimax(Board& board, int depth){

    if(depth == 0){
        return board.evaluate_position();
    }

    std::vector<Move> moves = board.generate_moves();

    if(moves.empty()){
        return board.evaluate_position();
    }

    bool maximizingPlayer = board.is_white_turn();

    if(maximizingPlayer){

        int bestScore = -999999;

        for(Move& m : moves){

            board.make_move(m);

            int score = minimax(board, depth - 1);

            board.undo_move(m);

            if(score > bestScore){
                bestScore = score;
            }
        }

        return bestScore;
    }

    else{

        int bestScore = 999999;

        for(Move& m : moves){

            board.make_move(m);

            int score = minimax(board, depth - 1);

            board.undo_move(m);

            if(score < bestScore){
                bestScore = score;
            }
        }

        return bestScore;
    }
}

Move Engine::find_best_move(Board& board, int depth){

    std::vector<Move> moves = board.generate_moves();

    if(moves.empty()){
        return Move{};
    }

    Move bestMove = moves[0];

    bool maximizingPlayer = board.is_white_turn();

    int bestScore;

    if(maximizingPlayer)
        bestScore = -999999;
    else
        bestScore = 999999;

    for(Move& m : moves){

        board.make_move(m);

        int score = minimax(board, depth - 1);

        board.undo_move(m);

        if(maximizingPlayer && score > bestScore){

            bestScore = score;
            bestMove = m;
        }

        if(!maximizingPlayer && score < bestScore){

            bestScore = score;
            bestMove = m;
        }
    }

    return bestMove;
}