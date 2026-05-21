#include <iostream>
#include "engine.h"
#include <algorithm>

int Engine::get_mvv_lva(Board& board,const Move& m){
    int attacker =abs(board.get_piece(m.fromRow,m.fromCol));
    int victim =abs(board.get_piece( m.toRow,m.toCol));

    if(victim==0){
        return 0;
    }

    return pieceValue[victim]*10 - pieceValue[attacker];
}

int Engine::alphabeta(Board& board, int depth, int alpha, int beta){
    nodes++;

    uint64_t key=board.get_hash();

    int alphaOrig=alpha;
    int betaOrig=beta;

    auto it=tt.find(key);
    if(it!=tt.end()){

        TTEntry& entry=it->second;

        if(entry.depth>=depth){

            if(entry.flag==EXACT)
                return entry.score;

            if(entry.flag==LOWERBOUND)
                alpha=std::max(alpha,entry.score);

            if(entry.flag==UPPERBOUND)
                beta=std::min(beta,entry.score);

            if(alpha>=beta)
                return entry.score;
        }
    }

    

    if(depth == 0){
        return quiescence(board,alpha,beta,3);
        //return board.evaluate_position();
    }

    if(board.is_fifty_move_draw()){
        return 0;
    }

    if(board.is_threefold_repetition()){
        return 0;
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

    

    

    if(it!=tt.end()){
        Move ttMove=it->second.bestMove;

        for(int i=0;i<moves.size();i++){
            Move&m=moves[i];

            if(m.fromRow==ttMove.fromRow && m.fromCol==ttMove.fromCol && m.toRow==ttMove.toRow && m.toCol==ttMove.toCol){
                std::swap(moves[0],moves[i]);
                break;
            }
        }
    }

    


    /*
    std::sort(moves.begin(),moves.end(),[](const Move& a,const Move& b){
        return a.capturedPiece != 0 && b.capturedPiece == 0;
    });
    
    std::stable_sort(moves.begin(),moves.end(),[&](const Move& a,const Move& b){
        return get_mvv_lva(board,a) > get_mvv_lva(board,b);
    });
    */

    for(Move& m : moves){
        int target   = abs(board.get_piece(m.toRow,   m.toCol));
        int attacker = abs(board.get_piece(m.fromRow, m.fromCol));
        if(target != 0)
            m.score = 10000 + pieceValue[target] * 10 - pieceValue[attacker];
        else if(m.isPromotion)
            m.score = 9000;
        else
            m.score = 0;
    }
    std::sort(moves.begin()+1, moves.end(), [](const Move& a, const Move& b){
        return a.score > b.score;
    });

    bool maximizingPlayer = board.is_white_turn();
    Move bestMove;
    int bestScore=0;

    if(maximizingPlayer){
        bestScore = -999999;

        for(Move& m : moves){
            Undo u;
            board.make_move(m,u);

            int score = alphabeta(board, depth - 1, alpha, beta);

            board.undo_move(m,u);

            if(score > bestScore){
                bestScore = score;
                bestMove=m;
            }
            if(score > alpha){
                alpha = score;
            }
            if(alpha >= beta){
                break;
            }
        }
    }

    else{
        bestScore = 999999;

        for(Move& m : moves){
            Undo u;
            board.make_move(m,u);

            int score = alphabeta(board, depth - 1, alpha, beta);

            board.undo_move(m,u);

            if(score < bestScore){
                bestScore = score;
                bestMove=m;
            }
            if(score < beta){
                beta = score;
            }
            if(alpha >= beta){
                break;
            }
        
        }
    }

    TTEntry entry;
    entry.depth=depth;
    entry.score=bestScore;
    entry.bestMove=bestMove;

    if (bestScore <= alphaOrig) entry.flag = UPPERBOUND;
    else if (bestScore >= betaOrig) entry.flag = LOWERBOUND;
    else  entry.flag = EXACT;

    tt[key] = entry;
    return bestScore;

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

    const int delta = 900; 
    if(standPat < alpha - delta){
        return alpha;
    }

    alpha = std::max(alpha,standPat);

    std::vector<Move> captures;

    for(Move& m : board.generate_captures()){

        int attacker =abs(board.get_piece(m.fromRow,m.fromCol));
        int victim = abs(board.get_piece(m.toRow,m.toCol));

        if(pieceValue[victim] < pieceValue[attacker] - 300) continue;

        captures.push_back(m);
    }

    std::sort(captures.begin(),captures.end(),[&](const Move& a,const Move& b){  
        return get_mvv_lva(board,a) > get_mvv_lva(board,b);
    });

    for(Move& m : captures){
        Undo u;
        board.make_move(m,u);

        int score= quiescence(board,alpha,beta,depth-1);

        board.undo_move(m,u);

        if(score>=beta){
            return beta;
        }

        alpha=std::max(alpha,score);
    }

    return alpha;
}

Move Engine::find_best_move(Board& board, int maxDepth){
    nodes = 0;

    BookMove bm;
    if(openingBook.probe(board.get_hash(), bm)){
        Move m;
        m.fromRow = bm.fromRow; m.fromCol = bm.fromCol;
        m.toRow   = bm.toRow;   m.toCol   = bm.toCol;
        std::cout << "Book move\n";
        return m;
    }
    
    
    std::vector<Move> moves = board.generate_moves();
    if(moves.empty()) return Move{};

    bool maximizingPlayer = board.is_white_turn();
    Move bestMove = moves[0];
    Move bestMoveThisIteration = moves[0];

    for(int depth = 1; depth <= maxDepth; depth++){

        // pin previous best at index 0
        if(hasPreviousBest){
            bool found = false;
            for(int i = 0; i < (int)moves.size(); i++){
                Move& m = moves[i];
                if(m.fromRow == bestMoveOverall.fromRow &&
                   m.fromCol == bestMoveOverall.fromCol &&
                   m.toRow   == bestMoveOverall.toRow   &&
                   m.toCol   == bestMoveOverall.toCol){
                    std::swap(moves[0], moves[i]);
                    found=true;
                    break;
                }
            }

            if(!found) hasPreviousBest = false;
        }
        int sortStart = hasPreviousBest ? 1 : 0;
        std::stable_sort(moves.begin() + sortStart, moves.end(), [&](const Move& a, const Move& b){
            auto scoreMove = [&](const Move& m) -> int {
                int target   = abs(board.get_piece(m.toRow,   m.toCol));
                int attacker = abs(board.get_piece(m.fromRow, m.fromCol));
                if(target != 0)   return 10000 + pieceValue[target] * 10 - pieceValue[attacker];
                if(m.isPromotion) return 9000;
                return 0;
            };
            return scoreMove(a) > scoreMove(b);
        });

        int bestScore = maximizingPlayer ? -999999 : 999999;

        for(Move& m : moves){
            Undo u;
            board.make_move(m,u);

            int score = alphabeta(board, depth - 1, -999999, 999999);

            board.undo_move(m,u);

            if(maximizingPlayer && score > bestScore){
                bestScore = score;
                bestMoveThisIteration = m;
            }
            if(!maximizingPlayer && score < bestScore){
                bestScore = score;
                bestMoveThisIteration = m;
            }
        }

        bestMoveOverall = bestMoveThisIteration;
        hasPreviousBest = true;
        bestMove = bestMoveThisIteration;

        std::cout << "Depth " << depth << " | nodes: " << nodes << " | eval: " << bestScore << "\n";
    }

    return bestMove;
}