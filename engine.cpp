#include <iostream>
#include "engine.h"
#include <algorithm>

int Engine::alphabeta(Board& board, int depth, int alpha, int beta){
    nodes++;

    if(depth == 0){
        return quiescence(board,alpha,beta,4);
        //return board.evaluate_position();
    }

    std::vector<Move> moves = board.generate_moves();

    if(moves.empty()){
        if(board.is_in_check(board.is_white_turn())){

            if(board.is_white_turn())
                return -100000 + depth;
            else
                return 100000 - depth;
        }

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

int Engine::quiescence(Board& board, int alpha, int beta, int depth){
    nodes++;

    int standPat = board.evaluate_position();

    if(depth <= 0){
        return standPat;
    }

    if(standPat >= beta){
        return beta;
    }

    alpha = std::max(alpha,standPat);

    std::vector<Move> captures;

    for(Move& m : board.generate_moves()){
        if(m.capturedPiece==0){
            continue;
        }

        int attacker =abs(board.get_piece(m.fromRow,m.fromCol));
        int victim = abs(m.capturedPiece);

        if(victim < attacker){
            continue;
        }

        captures.push_back(m);
    }

    std::sort(captures.begin(),captures.end(),[&](const Move& a,const Move& b){  // MVV-LVA ordering
        int scoreA = abs(a.capturedPiece)*10 - abs(board.get_piece(a.fromRow,a.fromCol)); 
        int scoreB = abs(b.capturedPiece)*10 - abs(board.get_piece(b.fromRow,b.fromCol));

        return scoreA > scoreB;
    });

    for(Move& m : captures){
        GameState state= board.save_state();
        board.make_move(m);

        int score= quiescence(board,alpha,beta,depth-1);

        board.undo_move(m);
        board.restore_state(state);

        if(score>=beta){
            return beta;
        }

        alpha=std::max(alpha,score);
    }

    return alpha;
}

Move Engine::find_best_move(Board& board, int depth){
    nodes=0;

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

    std::cout << "Nodes searched: " << nodes << '\n';
    return bestMove;
}