#include <iostream>
#include <string>

#include "board.h"
#include "engine.h"

int main(){

    Board board;
    board.init_board();

    Engine engine;

    for(int i=0;i<50;i++){
        //board.print_board();
        Move m=engine.find_best_move(board,3);
        board.make_move(m);
        std::cout << "Eval: " << board.evaluate_position() << "\n";
    }
    

    return 0;
}