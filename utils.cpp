# include "utils.h"

bool is_white(int piece) {
        return piece > 0;
    }

bool is_black(int piece) {
    return piece < 0;
}

bool in_bounds(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

bool same_color(int a, int b) {

    if(a > 0 && b > 0){
        return true;
    }

    if(a < 0 && b < 0){
        return true;
    }
    return false;   
}   