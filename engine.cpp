#include <iostream>
#include "engine.h"
#include <algorithm>

int Engine::get_mvv_lva(Board& board,const Move& m){
    int pieceValue[7] =
    {
        0,
        100,   // pawn
        330,   // bishop
        320,   // knight
        500,   // rook
        900,   // queen
        20000  // king
    };

    int attacker =abs(board.get_piece(m.fromRow,m.fromCol));

    int victim =abs(board.get_piece( m.toRow,m.toCol));

    if(victim==0){
        return 0;
    }

    return pieceValue[victim]*10 - pieceValue[attacker];
}

int Engine::alphabeta(Board& board, int depth, int alpha, int beta){
    nodes++;

    if(depth == 0){
        return quiescence(board,alpha,beta,3);
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
    /*
    std::stable_sort(moves.begin(),moves.end(),[&](const Move& a,const Move& b){
        return get_mvv_lva(board,a) > get_mvv_lva(board,b);
    });
    */

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

        int attacker =abs(board.get_piece(m.fromRow,m.fromCol));
        int victim = abs(board.get_piece(m.toRow,m.toCol));

        if(victim==0){
            continue;
        }

        if(victim < attacker){
            continue;
        }

        captures.push_back(m);
    }

    std::sort(captures.begin(),captures.end(),[&](const Move& a,const Move& b){  
        return get_mvv_lva(board,a) > get_mvv_lva(board,b);
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

    /*
    std::stable_sort(moves.begin(),moves.end(),[&](const Move& a,const Move& b){
        return get_mvv_lva(board,a) > get_mvv_lva(board,b);
    });
    */

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