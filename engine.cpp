#include <iostream>
#include "engine.h"
#include <algorithm>
#include <cstring>

static TTEntry g_tt[1 << 22];

bool Engine::check_time(){
    if(timeLimitMs < 0) return false;
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - searchStart).count();
    return elapsed >= timeLimitMs;
}

int Engine::get_mvv_lva(Board& board, const Move& m){
    int attacker = abs(board.get_piece(m.fromRow, m.fromCol));
    int victim   = abs(board.get_piece(m.toRow,   m.toCol));

    if(victim == 0) return 0;

    return pieceValue[victim] * 10 - pieceValue[attacker];
}

bool Engine::sameMove(const Move& a, const Move& b){
    return a.fromRow == b.fromRow && a.fromCol == b.fromCol
        && a.toRow   == b.toRow   && a.toCol   == b.toCol;
}

Engine::Engine(){
    std::fill(&historyTable[0][0][0], &historyTable[0][0][0] + 2*64*64, 0);

    for(int i = 0; i < max_ply; i++){
        killerMoves[i][0] = Move{};
        killerMoves[i][1] = Move{};
    }

    tt = g_tt;
    std::memset(tt, 0, sizeof(TTEntry) * (1 << 22));
}

int Engine::alphabeta(Board& board, int depth, int alpha, int beta, int ply, bool isNullMove){
    if(nodes % 2048 == 0 && check_time()){
        outOfTime = true;
        return 0;
    }
    nodes++;
    ply = std::min(ply, max_ply - 1);

    

    uint64_t key = board.get_hash();

    int alphaOrig = alpha;
    int betaOrig  = beta;

    
    int ttIdx = key % TT_SIZE;
    TTEntry& ttEntry = tt[ttIdx];
    bool ttHit = (ttEntry.key == key);

    if(ttHit && ttEntry.depth >= depth && ttEntry.generation == currentgeneration){
        if(ttEntry.flag == EXACT)
            return ttEntry.score;
        if(ttEntry.flag == LOWERBOUND)
            alpha = std::max(alpha, ttEntry.score);
        if(ttEntry.flag == UPPERBOUND)
            beta  = std::min(beta,  ttEntry.score);
        if(alpha >= beta)
            return ttEntry.score;
    }

    
    if(depth == 0)
        return quiescence(board, alpha, beta);

    
    if(board.is_fifty_move_draw())    return 0;
    if(board.is_threefold_repetition()) return 0;

    
    bool inCheck = board.is_in_check(board.is_white_turn());
    if(!isNullMove && !inCheck && depth >= 3){
        Undo u;
        board.make_null_move(u);
        
        int score = -alphabeta(board, depth - 3, -beta, -beta + 1, ply + 1, true);
        board.undo_null_move(u);

        if(score >= beta){
            nullMoveCuts++;
            return beta;
        }
    }

   
    std::vector<Move> moves = board.generate_moves();

    if(moves.empty()){
        if(inCheck)
            return -100000 + ply;   
        return 0;                  
    }

    
    for(int i = 0; i < (int)moves.size(); i++){
        Move& m = moves[i];
        
        if(ttHit && sameMove(m, ttEntry.bestMove)){
            m.score = 1000000;
            continue;
        }

        int target   = abs(board.get_piece(m.toRow,   m.toCol));
        int attacker = abs(board.get_piece(m.fromRow, m.fromCol));

        if(target != 0){
            int seeScore = board.see(m.toRow, m.toCol, target, m.fromRow, m.fromCol);
            if(seeScore > 0)
                m.score = 100000 + seeScore;  
            else if(seeScore == 0)
                m.score = 90000;              
            else
                m.score = seeScore;           
        }
        else if(m.isPromotion){
            m.score = 88000;
        }
        else if(sameMove(m, killerMoves[ply][0])){
            m.score = 95000;  
        }
        else if(sameMove(m, killerMoves[ply][1])){
            m.score = 94000;
        }
        else{
            int color = board.is_white_turn() ? 0 : 1;
            int from  = m.fromRow * 8 + m.fromCol;
            int to    = m.toRow   * 8 + m.toCol;
            m.score   = historyTable[color][from][to];
        }
    }
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b){
        return a.score > b.score;
    });

   
    int  bestScore = -999999;
    Move bestMove  = moves[0];

    int moveIndex = 0;

    for(Move& m : moves){
        Undo u;
        board.make_move(m, u);

        int score;

        bool doLMR = moveIndex >= 3
            && depth >= 3
            && m.capturedPiece == 0
            && !sameMove(m, killerMoves[ply][0])
            && !sameMove(m, killerMoves[ply][1])
            && !inCheck
            && !board.is_in_check(board.is_white_turn());
        
        if(doLMR){
            
            int reduction = 1;
            if(moveIndex >= 6) reduction = 2; 

            score = -alphabeta(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, false);
            
            if(score > alpha){
                score = -alphabeta(board, depth - 1, -beta, -alpha, ply + 1, false);
            }
        }
        else{
            score = -alphabeta(board, depth - 1, -beta, -alpha, ply + 1, false);
        }

        

        board.undo_move(m, u);

        if(score > bestScore){
            bestScore = score;
            bestMove  = m;
        }
        if(score > alpha)
            alpha = score;

        if(alpha >= beta){
            if(m.capturedPiece == 0){
                if(!sameMove(m, killerMoves[ply][0])){
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = m;
                }
                int color = board.is_white_turn() ? 1 : 0; 
                int from = m.fromRow * 8 + m.fromCol;
                int to  = m.toRow * 8 + m.toCol;
                historyTable[color][from][to] += depth * depth;
                if(historyTable[color][from][to] > 65000) historyTable[color][from][to] = 65000;
            }
            break;
        }
    }

    TTEntry& store = tt[ttIdx];
    
    if(!ttHit || depth >= store.depth){
        store.key=key;
        store.depth=depth;
        store.score=bestScore;
        store.bestMove=bestMove;
        store.generation = currentgeneration;

        if(bestScore <= alphaOrig) store.flag = UPPERBOUND;
        else if(bestScore >= betaOrig) store.flag = LOWERBOUND;
        else store.flag = EXACT;
    }

    return bestScore;
}

int Engine::quiescence(Board& board, int alpha, int beta){
    qnodes++;
    
    int standPat = board.evaluate_position();
    if(!board.is_white_turn()) standPat = -standPat;

    if(standPat >= beta) return beta;

    alpha = std::max(alpha, standPat);

    std::vector<Move> captures = board.generate_captures();

    std::sort(captures.begin(), captures.end(), [&](const Move& a, const Move& b){
        return get_mvv_lva(board, a) > get_mvv_lva(board, b);
    });

    for(Move& m : captures){
        bool isEP = (abs(board.get_piece(m.fromRow, m.fromCol)) == 1 &&m.toCol != m.fromCol && board.get_piece(m.toRow, m.toCol) == 0);

        if(!isEP){
            int seeScore = board.see(m.toRow, m.toCol,abs(board.get_piece(m.toRow, m.toCol)),m.fromRow, m.fromCol);
            if(seeScore < 0) continue;
        }
        Undo u;
        board.make_move(m, u);

        int score = -quiescence(board, -beta, -alpha);

        board.undo_move(m, u);

        if(score >= beta)  return beta;
        alpha = std::max(alpha, score);
    }

    return alpha;
}

Move Engine::find_best_move(Board& board, int maxDepth){
    currentgeneration++;
    nullMoveCuts = 0;
    
   
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

    Move bestMove = moves[0];
    Move bestMoveThisIteration = moves[0];


    auto iterStart = Clock::now();
    for(int depth = 1; depth <= maxDepth; depth++){
        nodes = 0;
        qnodes=0;
        for(int c = 0; c < 2; c++)
            for(int f = 0; f < 64; f++)
                for(int t = 0; t < 64; t++)
                    historyTable[c][f][t] /= 2;

        
        if(hasPreviousBest){
            bool found = false;
            for(int i = 0; i < (int)moves.size(); i++){
                if(sameMove(moves[i], bestMoveOverall)){
                    std::swap(moves[0], moves[i]);
                    found = true;
                    break;
                }
            }
            if(!found) hasPreviousBest = false;
        }

        int sortStart = hasPreviousBest ? 1 : 0;
        std::sort(moves.begin() + sortStart, moves.end(), [&](const Move& a, const Move& b){
            int ta = abs(board.get_piece(a.toRow, a.toCol));
            int aa = abs(board.get_piece(a.fromRow, a.fromCol));
            int tb = abs(board.get_piece(b.toRow, b.toCol));
            int ab = abs(board.get_piece(b.fromRow, b.fromCol));

            int scoreA = ta ? 10000 + pieceValue[ta] * 10 - pieceValue[aa] : 0;
            int scoreB = tb ? 10000 + pieceValue[tb] * 10 - pieceValue[ab] : 0;

            return scoreA > scoreB;
        });

        
        int bestScore = -999999;

        for(Move& m : moves){
            Undo u;
            board.make_move(m, u);

            int score = -alphabeta(board, depth - 1, -999999, 999999, 0, false);

            board.undo_move(m, u);

            if(score > bestScore){
                bestScore = score;
                bestMoveThisIteration = m;
            }
        }

        bestMoveOverall = bestMoveThisIteration;
        hasPreviousBest = true;
        bestMove = bestMoveThisIteration;

        int displayEval = board.is_white_turn() ? bestScore : -bestScore;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - iterStart).count();

        std::cout << "Depth " << depth
                << " | nodes: "    << nodes
                << " | qnodes: "   << qnodes
                << " | eval: "     << displayEval
                << " | time: "     << elapsed << "ms"
                << " | nps: "      << (elapsed > 0 ? (nodes + qnodes) * 1000 / elapsed : 0)
                << "\n";
    }

    return bestMove;
}

Move Engine::find_best_move_timed(Board& board, int limitMs){
    searchStart  = Clock::now();
    timeLimitMs  = limitMs;
    outOfTime    = false;
 
    currentgeneration++;
    nullMoveCuts = 0;
    qnodes=0;
   
    BookMove bm;
    if(openingBook.probe(board.get_hash(), bm)){
        Move m;
        m.fromRow = bm.fromRow; 
        m.fromCol = bm.fromCol;
        m.toRow = bm.toRow;
        m.toCol   = bm.toCol;
        return m;
    }
 
    std::vector<Move> moves = board.generate_moves();
    if(moves.empty()) return Move{};
 
    Move bestMove = moves[0];
    Move bestMoveThisIteration = moves[0];
 
    for(int depth = 1; depth <= 100; depth++){ 
        nodes = 0;
 
        if(check_time()) break;
 
        
        for(int c = 0; c < 2; c++)
            for(int f = 0; f < 64; f++)
                for(int t = 0; t < 64; t++)
                    historyTable[c][f][t] /= 2;
 
        currentgeneration++;
 
      
        if(hasPreviousBest){
            for(int i = 0; i < (int)moves.size(); i++){
                if(sameMove(moves[i], bestMoveOverall)){
                    std::swap(moves[0], moves[i]);
                    break;
                }
            }
        }
 
        int sortStart = hasPreviousBest ? 1 : 0;
        std::sort(moves.begin() + sortStart, moves.end(), [&](const Move& a, const Move& b){
            int ta = abs(board.get_piece(a.toRow, a.toCol));
            int aa = abs(board.get_piece(a.fromRow, a.fromCol));
            int tb = abs(board.get_piece(b.toRow, b.toCol));
            int ab = abs(board.get_piece(b.fromRow, b.fromCol));
            int sA = ta ? 10000 + pieceValue[ta]*10 - pieceValue[aa] : 0;
            int sB = tb ? 10000 + pieceValue[tb]*10 - pieceValue[ab] : 0;
            return sA > sB;
        });
 
        int bestScore = -999999;
        outOfTime = false;
 
        for(Move& m : moves){
            Undo u;
            board.make_move(m, u);
            int score = -alphabeta(board, depth - 1, -999999, 999999, 0, false);
            board.undo_move(m, u);
 
            if(outOfTime) break; 
 
            if(score > bestScore){
                bestScore = score;
                bestMoveThisIteration = m;
            }
        }
 
        if(!outOfTime){
            bestMoveOverall = bestMoveThisIteration;
            bestMove        = bestMoveThisIteration;
            hasPreviousBest = true;
 
           
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - searchStart).count();
            int displayEval = bestScore;    
 
            std::cout << "info depth " << depth
                      << " score cp "  << displayEval
                      << " nodes "     << nodes
                      << " time "      << elapsed
                      << " nps "       << (elapsed > 0 ? nodes * 1000 / elapsed : 0)
                      << "\n";
            std::cout.flush();
        }
 
        if(check_time()) break;
    }
 
    timeLimitMs = -1;
    return bestMove;
}