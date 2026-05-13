#include "engine.h"
#include <algorithm>

int Engine::alphabeta(Board& board, int depth, int alpha, int beta){

    if(depth == 0){
        return board.evaluate_position();
    }

    std::vector<Move> moves = board.generate_moves();

    if(board.is_checkmate()){

        if(board.is_white_turn())
            return -100000 + depth;

        else
            return 100000 - depth;
    }

    if(board.is_stalemate()){
        return 0;
    }

    if(board.is_fifty_move_draw()){
        return 0;
    }

    if(board.is_threefold_repetition()){
        return 0;
    }

    std::sort(moves.begin(),moves.end(),[](const Move& a,const Move& b){
        return a.capturedPiece != 0 && b.capturedPiece == 0;
    });

    bool maximizingPlayer = board.is_white_turn();

    if(maximizingPlayer){

        int bestScore = -999999;

        for(Move& m : moves){

            GameState state = board.save_state();
            board.make_move(m);

            int score = alphabeta(board, depth - 1, alpha, beta);

            board.undo_move(m);
            board.restore_state(state);

            if(score > bestScore){
                bestScore = score;
            }
            if(score > alpha){
                alpha = score;
            }
            if(alpha >= beta){
                break;
            }
        }

        return bestScore;
    }

    else{

        int bestScore = 999999;

        for(Move& m : moves){

            GameState state = board.save_state();
            board.make_move(m);

            int score = alphabeta(board, depth - 1, alpha, beta);

            board.undo_move(m);
            board.restore_state(state);

            if(score < bestScore){
                bestScore = score;
            }
            if(score < beta){
                beta = score;
            }
            if(alpha >= beta){
                break;
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
        GameState state = board.save_state();
        board.make_move(m);

        int score = alphabeta(board, depth - 1,-999999,999999);

        board.undo_move(m);
        board.restore_state(state);

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