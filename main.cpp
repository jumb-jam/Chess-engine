#include <iostream>
#include <string>

#include "board.h"
#include "engine.h"
#include "zobrist.h"

int main(){

    Zobrist::init();

    Board board;
    board.init_board();
    std::cout << board.evaluate_position() << '\n';

    
    
    Undo u;
    Engine engine;
    //engine.openingBook.init();

    for(int i=0;i<50;i++){
        board.print_board();
        Move m=engine.find_best_move(board,7);
        board.make_move(m,u);
        //std::cout << "Eval: " << board.evaluate_position() << "\n";
    }
    

    return 0;
}