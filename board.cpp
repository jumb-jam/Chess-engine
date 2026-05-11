#include <iostream>
#include <vector>
#include "utils.h"

#include "board.h"

// must implement castling and en passant, then model is completel at a basic level, do test run.

void Board::init_board(){
    for(int i=0;i<8;i++){
        board[1][i]=-1;  
        board[6][i]=1;
    }
    board[0][4]=-6;
    board[7][4]=6;

    board[0][3]=-5;
    board[7][3]=5;

    board[0][2]=-2;
    board[7][2]=2;
    board[0][5]=-2;
    board[7][5]=2;

    board[0][1]=-3;
    board[7][1]=3;
    board[0][6]=-3;
    board[7][6]=3;

    board[0][0]=-4;
    board[7][0]=4;
    board[0][7]=-4;
    board[7][7]=4;
}

bool Board::is_white_turn(){
    return whiteTurn;
};

int Board::get_piece(int row,int col){
    return board[row][col];
}

bool Board::make_move(Move m){

    int piece = get_piece(m.fromRow,m.fromCol);

    if(piece == 0){
        return false;
    }

    m.capturedPiece = get_piece(m.toRow, m.toCol);

    board[m.toRow][m.toCol] = piece;
    board[m.fromRow][m.fromCol] = 0;

    whiteTurn = !whiteTurn;

    return true;
}

bool Board::is_valid_move(const Move& m){


    if(!in_bounds(m.fromRow, m.fromCol) || !in_bounds(m.toRow, m.toCol)) {
        return false;
    }

    int rowDiff = m.toRow - m.fromRow;
    int colDiff = m.toCol - m.fromCol;

    if(rowDiff == 0 && colDiff == 0) {
        return false;
    }

    int piece = get_piece(m.fromRow, m.fromCol);
    int target = get_piece(m.toRow, m.toCol);

    if(piece == 0) {
        return false;
    }

    if(target != 0 && same_color(piece, target)) {
        return false;
    }

    if(whiteTurn && piece < 0)
        return false;

    if(!whiteTurn && piece > 0)
        return false;

    int type = abs(piece);

    switch(type){

        case 1: {

            int direction = (piece > 0) ? -1 : 1;

            if(colDiff == 0 && target == 0 && rowDiff == direction) {
                return true;
            }

            if(colDiff == 0 && target == 0 && rowDiff == 2 * direction){

                if(piece > 0 && m.fromRow == 6 && get_piece(m.fromRow - 1, m.fromCol) == 0) {
                    return true;
                }

                if(piece < 0 && m.fromRow == 1 && get_piece(m.fromRow + 1, m.fromCol) == 0) {
                    return true;
                }
            }

            if(abs(colDiff) == 1 && rowDiff == direction && target != 0 &&  !same_color(piece, target)) {
                return true;
            }

            break;
        }

        case 2: {

            if(abs(rowDiff) != abs(colDiff)) {
                return false;
            }

            int rowStep = (rowDiff > 0) ? 1 : -1;
            int colStep = (colDiff > 0) ? 1 : -1;

            int r = m.fromRow + rowStep;
            int c = m.fromCol + colStep;

            while(r != m.toRow || c != m.toCol){

                if(get_piece(r, c) != 0) {
                    return false;
                }

                r += rowStep;
                c += colStep;
            }

            return true;
        }

        case 3: {

            if((abs(rowDiff) == 2 && abs(colDiff) == 1) ||
            (abs(rowDiff) == 1 && abs(colDiff) == 2)) {
                return true;
            }

            break;
        }

        case 4: {

            if(rowDiff != 0 && colDiff != 0) {
                return false;
            }

            int rowStep = 0;
            int colStep = 0;

            if(rowDiff != 0)
                rowStep = (rowDiff > 0) ? 1 : -1;

            if(colDiff != 0)
                colStep = (colDiff > 0) ? 1 : -1;

            int r = m.fromRow + rowStep;
            int c = m.fromCol + colStep;

            while(r != m.toRow || c != m.toCol){

                if(get_piece(r, c) != 0) {
                    return false;
                }

                r += rowStep;
                c += colStep;
            }

            return true;
        }

        case 5: {

            if(rowDiff == 0 || colDiff == 0){

                int rowStep = 0;
                int colStep = 0;

                if(rowDiff != 0)
                    rowStep = (rowDiff > 0) ? 1 : -1;

                if(colDiff != 0)
                    colStep = (colDiff > 0) ? 1 : -1;

                int r = m.fromRow + rowStep;
                int c = m.fromCol + colStep;

                while(r != m.toRow || c != m.toCol){

                    if(get_piece(r, c) != 0) {
                        return false;
                    }

                    r += rowStep;
                    c += colStep;
                }

                return true;
            }

            if(abs(rowDiff) == abs(colDiff)){

                int rowStep = (rowDiff > 0) ? 1 : -1;
                int colStep = (colDiff > 0) ? 1 : -1;

                int r = m.fromRow + rowStep;
                int c = m.fromCol + colStep;

                while(r != m.toRow || c != m.toCol){

                    if(get_piece(r, c) != 0) {
                        return false;
                    }

                    r += rowStep;
                    c += colStep;
                }

                return true;
            }

            break;
        }

        case 6: {

            if(abs(rowDiff) <= 1 &&
            abs(colDiff) <= 1) {
                return true;
            }

            break;
        }
    }

    return false;
}

bool Board::is_square_attacked(int row, int col, bool byWhite){

    for(int r = 0; r < 8; r++){

        for(int c = 0; c < 8; c++){

            int piece = get_piece(r, c);

            if(piece == 0)
                continue;

            if(byWhite && piece < 0)
                continue;

            if(!byWhite && piece > 0)
                continue;

            Move m{
                r,
                c,
                row,
                col
            };

            bool originalTurn = whiteTurn;

            whiteTurn = byWhite;

            bool attacks = is_valid_move(m);

            whiteTurn = originalTurn;

            if(attacks){
                return true;
            }
        }
    }

    return false;
}

bool Board::is_in_check(bool whiteKing){
    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){
            int piece = get_piece(row, col);

            if(whiteKing && piece == 6){
                return is_square_attacked(row,col,false);
            }

            if(!whiteKing && piece == -6){
                return is_square_attacked(row,col,true);
            }
        }
    }

    return false;
}

std::vector<Move> Board::generate_moves(){

    std::vector<Move> moves;
    std::vector<Move> legalMoves;

    for(int fromRow = 0; fromRow < 8; fromRow++){

        for(int fromCol = 0; fromCol < 8; fromCol++){

            int piece = get_piece(fromRow, fromCol);

            if(piece == 0)
                continue;

            if(whiteTurn && piece < 0)
                continue;

            if(!whiteTurn && piece > 0)
                continue;

            for(int toRow = 0; toRow < 8; toRow++){

                for(int toCol = 0; toCol < 8; toCol++){

                    Move m{
                        fromRow,
                        fromCol,
                        toRow,
                        toCol
                    };

                    if(is_valid_move(m)){
                        m.capturedPiece = get_piece(toRow, toCol);

                        moves.push_back(m);
                    }
                }
            }
        }
    }

    for(Move& m : moves){

        bool movingWhite = get_piece(m.fromRow,m.fromCol) > 0;
        make_move(m);

        if(!is_in_check(movingWhite)){
            legalMoves.push_back(m);
        }

        undo_move(m);
    }

    return legalMoves;

}

int Board::evaluate_position(){

    int score = 0;

    for(int row = 0; row < 8; row++){

        for(int col = 0; col < 8; col++){

            int piece = get_piece(row, col);

            switch(abs(piece)){

                case 1:
                    score += (piece > 0) ? 100 : -100;
                    break;

                case 2:
                    score += (piece > 0) ? 330 : -330;
                    break;

                case 3:
                    score += (piece > 0) ? 320 : -320;
                    break;

                case 4:
                    score += (piece > 0) ? 500 : -500;
                    break;

                case 5:
                    score += (piece > 0) ? 900 : -900;
                    break;

                case 6:
                    score += (piece > 0) ? 20000 : -20000;
                    break;
            }
        }
    }

    return score;
}

void Board::undo_move(const Move& m){

        int piece = get_piece(m.toRow, m.toCol);

        board[m.fromRow][m.fromCol] = piece;

        board[m.toRow][m.toCol] = m.capturedPiece;

        whiteTurn = !whiteTurn;
    }


