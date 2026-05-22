#include "uci.h"
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>


void UCI::loop(){

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string line;
    while(std::getline(std::cin, line)){
        if(line.empty()) continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if(token == "uci"){
            std::cout << "id name MyEngine\n";
            std::cout << "id author You\n";
            std::cout << "uciok\n";
        }

        else if(token == "isready"){
            std::cout << "readyok\n";
        }

        else if(token == "ucinewgame"){
            board = Board();
            board.init_board();
            engine = Engine();
        }

        else if(token == "position"){
            handle_position(line);
        }

        else if(token == "go"){
            handle_go(line);
        }

        else if(token == "quit"){
            break;
        }

        std::cout.flush();
    }
}


void UCI::handle_position(const std::string& line){
    std::istringstream iss(line);
    std::string token;
    iss >> token; 

    board = Board();
    board.init_board();

    iss >> token;
    if(token == "startpos"){
        iss >> token; 
    }
    else if(token == "fen"){
        std::string fen, part;
        while(iss >> part){
            if(part == "moves"){ token = "moves"; break; }
            fen += (fen.empty() ? "" : " ") + part;
        }
        board.load_fen(fen);
        if(part != "moves") iss >> token; 
    }

    if(token != "moves") return;

    std::string moveStr;
    while(iss >> moveStr){
        Move m = parse_move(moveStr);
        Undo u;
        board.make_move(m, u);
    }
}

void UCI::handle_go(const std::string& line){
    std::istringstream iss(line);
    std::string token;
    iss >> token; 

    int wtime = -1;
    int btime = -1;
    int winc = 0;
    int binc = 0;
    int movestogo = 30;
    int movetime = -1;
    int depth = 7; 

    while(iss >> token){
        if     (token == "wtime")     iss >> wtime;
        else if(token == "btime")     iss >> btime;
        else if(token == "winc")      iss >> winc;
        else if(token == "binc")      iss >> binc;
        else if(token == "movestogo") iss >> movestogo;
        else if(token == "movetime")  iss >> movetime;
        else if(token == "depth")     iss >> depth;
    }

    int timeLimitMs = -1;

    if(movetime > 0){
        timeLimitMs = movetime - 50; 
    }
    else if(wtime > 0 || btime > 0){
        int myTime = board.is_white_turn() ? wtime : btime;
        int myInc  = board.is_white_turn() ? winc  : binc;
       
        timeLimitMs = (myTime / movestogo) + (myInc / 2);
       
        timeLimitMs = std::min(timeLimitMs, myTime / 4);
        
        timeLimitMs = std::max(timeLimitMs, 50);
    }

    Move best;
    if(timeLimitMs > 0){
        best = engine.find_best_move_timed(board, timeLimitMs);
    } else {
        best = engine.find_best_move(board, depth);
    }

    
    std::string moveStr;
    moveStr += (char)('a' + best.fromCol);
    moveStr += (char)('1' + (7 - best.fromRow));
    moveStr += (char)('a' + best.toCol);
    moveStr += (char)('1' + (7 - best.toRow));

    
    if(best.promotionPiece != 0){
        switch(abs(best.promotionPiece)){
            case 5: moveStr += 'q'; break;
            case 4: moveStr += 'r'; break;
            case 2: moveStr += 'b'; break;
            case 3: moveStr += 'n'; break;
        }
    }

    std::cout << "bestmove " << moveStr << "\n";
    std::cout.flush();
}

Move UCI::parse_move(const std::string& s){
    Move m{};

    if(s.size() < 4) return m;

    m.fromCol = s[0] - 'a';
    m.fromRow = 7 - (s[1] - '1');
    m.toCol   = s[2] - 'a';
    m.toRow   = 7 - (s[3] - '1');

    // Promotion piece
    if(s.size() >= 5){
        bool isWhite = board.get_piece(m.fromRow, m.fromCol) > 0;
        switch(s[4]){
            case 'q': m.promotionPiece = isWhite ?  5 : -5; break;
            case 'r': m.promotionPiece = isWhite ?  4 : -4; break;
            case 'b': m.promotionPiece = isWhite ?  2 : -2; break;
            case 'n': m.promotionPiece = isWhite ?  3 : -3; break;
        }
    }

    return m;
}