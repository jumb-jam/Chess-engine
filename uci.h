#pragma once
#include "board.h"
#include "engine.h"
#include <string>
 
class UCI {
public:
    void loop();
 
private:
    Board  board;
    Engine engine;
 
    void handle_position(const std::string& line);
    void handle_go(const std::string& line);
    Move parse_move(const std::string& moveStr);
};