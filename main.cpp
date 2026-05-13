#include <iostream>
#include "engine.h"
#include "board.h"


int main(){
    Board b;
    b.init_board();

    Engine engine;

    for(int i = 0; i < 100; i++){

        b.print_board();

        Move bestMove =
            engine.find_best_move(b, 3);

        b.make_move(bestMove);
    }

    if(b.is_white_turn()){
        std::cout<< "White to move\n";
    }
    else{
        std::cout<< "Black to move\n";
    }
    std::cout << "Eval Score: " << b.evaluate_position();

    return 0;
}