#include <iostream>
#include <vector>

#include "utils.h"
#include "board.h"
#include "evaluation.h"

// implemented prev states, now need to improve evaluation algo

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

    positionHistory.push_back(get_position_key());
}

void Board::print_board(){
    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){

            int piece = board[row][col];

            char symbol = '.';

            switch(abs(piece)){
                case 1:
                    symbol = 'p';
                    break;
                case 2:
                    symbol = 'b';
                    break;
                case 3:
                    symbol = 'n';
                    break;
                case 4:
                    symbol = 'r';
                    break;
                case 5:
                    symbol = 'q';
                    break;
                case 6:
                    symbol = 'k';
                    break;
            }

            if(piece < 0){
                symbol = toupper(symbol);
            }
            std::cout << symbol << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

std::string Board::get_position_key() const{

    std::string key;

    for(int row = 0; row < 8; row++){

        for(int col = 0; col < 8; col++){

            key += std::to_string(board[row][col]);
            key += ',';
        }
    }

    key += whiteTurn ? 'w' : 'b';

    key += std::to_string(enPassantRow);
    key += std::to_string(enPassantCol);

    key += whiteKingMoved ? '1' : '0';
    key += blackKingMoved ? '1' : '0';

    key += whiteKingsideRookMoved ? '1' : '0';
    key += whiteQueensideRookMoved ? '1' : '0';

    key += blackKingsideRookMoved ? '1' : '0';
    key += blackQueensideRookMoved ? '1' : '0';

    return key;
}

GameState Board::save_state() const{

    GameState state;

    state.whiteTurn = whiteTurn;

    state.whiteKingMoved = whiteKingMoved;
    state.blackKingMoved = blackKingMoved;

    state.whiteKingsideRookMoved = whiteKingsideRookMoved;

    state.whiteQueensideRookMoved = whiteQueensideRookMoved;

    state.blackKingsideRookMoved = blackKingsideRookMoved;

    state.blackQueensideRookMoved = blackQueensideRookMoved;

    state.enPassantRow = enPassantRow;
    state.enPassantCol = enPassantCol;

    state.fiftymoveClock=fiftymoveClock;

    return state;
}

void Board::restore_state(const GameState& state){

    whiteTurn = state.whiteTurn;

    whiteKingMoved = state.whiteKingMoved;

    blackKingMoved = state.blackKingMoved;

    whiteKingsideRookMoved = state.whiteKingsideRookMoved;

    whiteQueensideRookMoved = state.whiteQueensideRookMoved;

    blackKingsideRookMoved = state.blackKingsideRookMoved;

    blackQueensideRookMoved = state.blackQueensideRookMoved;

    enPassantRow = state.enPassantRow;
    enPassantCol = state.enPassantCol;

    fiftymoveClock=state.fiftymoveClock;
}

bool Board::is_white_turn(){
    return whiteTurn;
};

bool Board::is_checkmate(){

    bool white = is_white_turn();

    if(!is_in_check(white)){
        return false;
    }

    return generate_moves().empty();
}

bool Board::is_stalemate(){

    bool white = is_white_turn();

    if(is_in_check(white)){
        return false;
    }

    return generate_moves().empty();
}

bool Board::is_fifty_move_draw(){
    return fiftymoveClock>=100;
}

bool Board::is_threefold_repetition(){
    std::string current = get_position_key();
    int count = 0;

    for(const std::string& pos : positionHistory){

        if(pos == current){
            count++;
        }
    }

    return count >= 3;
}

int Board::get_piece(int row,int col){
    return board[row][col];
}

bool Board::make_move(Move& m){
    m.capturedPiece = 0;
    m.isCastle = false;
    m.isEnPassant = false;
    m.isPromotion = false;
    m.promotionPiece = 0;
    
    int piece = get_piece(m.fromRow,m.fromCol);

    if(piece == 0){
        return false;
    }

    if(abs(piece) == 1 || m.capturedPiece != 0){
        fiftymoveClock = 0;
    }
    else{
        fiftymoveClock++;
    }

    if(abs(piece) == 1){
        if((piece > 0 && m.toRow == 0) || (piece < 0 && m.toRow == 7)){
            m.isPromotion = true;

            if(m.promotionPiece == 0){
                m.promotionPiece = 5;
            }
        }
    }

    m.capturedPiece = get_piece(m.toRow, m.toCol);

    if(m.capturedPiece == 4){

        if(m.toRow == 7 && m.toCol == 0){
            whiteQueensideRookMoved = true;
        }

        if(m.toRow == 7 && m.toCol == 7){
            whiteKingsideRookMoved = true;
        }
    }

    if(m.capturedPiece == -4){

        if(m.toRow == 0 && m.toCol == 0){
            blackQueensideRookMoved = true;
        }

        if(m.toRow == 0 && m.toCol == 7){
            blackKingsideRookMoved = true;
        }
    }

    if(abs(piece) == 6 && abs(m.toCol - m.fromCol) == 2){  //castle check
        m.isCastle = true;
    }

    if(abs(piece) == 1 && m.toRow == enPassantRow && m.toCol == enPassantCol && m.capturedPiece == 0){  //enpassant check
        m.isEnPassant = true;

        if(piece > 0){
            m.capturedPiece = board[m.toRow + 1][m.toCol];

            board[m.toRow + 1][m.toCol] = 0;
        }
        else{
            m.capturedPiece = board[m.toRow - 1][m.toCol];

            board[m.toRow - 1][m.toCol] = 0;
        }
    }


   

    if(m.isPromotion){
        if(piece > 0)
            board[m.toRow][m.toCol] = m.promotionPiece;
        else
            board[m.toRow][m.toCol] = -m.promotionPiece;
    }
    else{
        board[m.toRow][m.toCol] = piece;
    }

    board[m.fromRow][m.fromCol] = 0;

    if(m.isCastle){
        if(piece > 0){
            if(m.toCol == 6){
                board[7][5] = 4;
                board[7][7] = 0;
            }
            else{
                board[7][3] = 4;
                board[7][0] = 0;
            }
        }
        else{
            if(m.toCol == 6){
                board[0][5] = -4;
                board[0][7] = 0;
            }
            else{
                board[0][3] = -4;
                board[0][0] = 0;
            }
        }
    }

    if(piece == 6){
        whiteKingMoved = true;
    }
    if(piece == -6){
        blackKingMoved = true;
    }
    if(piece == 4 && m.fromRow == 7 && m.fromCol == 0){
        whiteQueensideRookMoved = true;
    }
    if(piece == 4 && m.fromRow == 7 && m.fromCol == 7){
        whiteKingsideRookMoved = true;
    }
    if(piece == -4 && m.fromRow == 0 && m.fromCol == 0){
        blackQueensideRookMoved = true;
    }
    if(piece == -4 && m.fromRow == 0 && m.fromCol == 7){
        blackKingsideRookMoved = true;
    }

    enPassantRow = -1;
    enPassantCol = -1;

    if(abs(piece)==1 && abs(m.toRow-m.fromRow)==2){
        enPassantRow = (m.fromRow+m.toRow)/2;
        enPassantCol=m.fromCol;
    }

    whiteTurn = !whiteTurn;

    positionHistory.push_back(get_position_key());

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
        case 1:{
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

            if(abs(colDiff)==1 && rowDiff==direction && m.toRow==enPassantRow && m.toCol==enPassantCol){ 
                return true;
            }

            break;
        }

        case 2:{

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

        case 3:{
            if((abs(rowDiff) == 2 && abs(colDiff) == 1) || (abs(rowDiff) == 1 && abs(colDiff) == 2)){
                return true;
            }

            break;
        }

        case 4:{

            if(rowDiff != 0 && colDiff != 0){
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

        case 5:{

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

            if(abs(rowDiff) <= 1 && abs(colDiff) <= 1){
                return true;
            }

            if(piece > 0){   //checking castling legality

                if(!whiteKingMoved && rowDiff==0 && colDiff==2){
                   
                    if(m.toCol==6 &&
                    !whiteKingsideRookMoved &&
                    get_piece(7,7)==4 &&          
                    get_piece(7,5)==0 &&
                    get_piece(7,6)==0 &&
                    !is_square_attacked(7,4,false) &&
                    !is_square_attacked(7,5,false) &&
                    !is_square_attacked(7,6,false))
                    {
                        return true;
                    }
                   
                    if(m.toCol==2 &&
                    !whiteQueensideRookMoved &&
                    get_piece(7,0)==4 &&          
                    get_piece(7,1)==0 &&
                    get_piece(7,2)==0 &&
                    get_piece(7,3)==0 &&
                    !is_square_attacked(7,4,false) &&
                    !is_square_attacked(7,3,false) &&
                    !is_square_attacked(7,2,false))
                    {
                        return true;
                    }
                }
            }

            else{

                if(!blackKingMoved && rowDiff == 0 && colDiff == 2){

                    if(m.toCol==6 &&
                    !blackKingsideRookMoved &&
                    get_piece(0,7)==-4 &&       // NEW
                    get_piece(0,5)==0 &&
                    get_piece(0,6)==0 &&
                    !is_square_attacked(0,4,true) &&
                    !is_square_attacked(0,5,true) &&
                    !is_square_attacked(0,6,true))
                    {
                        return true;
                    }

                    // Queenside
                    if(m.toCol==2 &&
                    !blackQueensideRookMoved &&
                    get_piece(0,0)==-4 &&       // NEW
                    get_piece(0,1)==0 &&
                    get_piece(0,2)==0 &&
                    get_piece(0,3)==0 &&
                    !is_square_attacked(0,4,true) &&
                    !is_square_attacked(0,3,true) &&
                    !is_square_attacked(0,2,true))
                    {
                        return true;
                    }
                }
            }

            break;
        }
    }

    return false;
}

bool Board::is_square_attacked(int row, int col, bool byWhite){

    int pawn = byWhite ? 1 : -1;
    int knight = byWhite ? 3 : -3;
    int bishop = byWhite ? 2 : -2;
    int rook = byWhite ? 4 : -4;
    int queen = byWhite ? 5 : -5;
    int king = byWhite ? 6 : -6;

    int pawnDir = byWhite ? 1 : -1;

    if(in_bounds(row + pawnDir, col - 1) && get_piece(row + pawnDir, col - 1) == pawn){
        return true;
    }

    if(in_bounds(row + pawnDir, col + 1) && get_piece(row + pawnDir,col + 1) == pawn){
        return true;
    }

    int knightOffsets[8][2] = {
        {-2,-1},
        {-2, 1},
        {-1,-2},
        {-1, 2},
        { 1,-2},
        { 1, 2},
        { 2,-1},
        { 2, 1}
    };

    for(auto& offset : knightOffsets){
        int r = row + offset[0];
        int c = col + offset[1];

        if(in_bounds(r,c) && get_piece(r,c) == knight){
            return true;
        }
    }

    int bishopDirs[4][2] = {
        {-1,-1},
        {-1, 1},
        { 1,-1},
        { 1, 1}
    };

    for(auto& dir : bishopDirs){
        int r = row + dir[0];
        int c = col + dir[1];

        while(in_bounds(r,c)){
            int piece = get_piece(r,c);

            if(piece != 0){
                if(piece == bishop || piece == queen){
                    return true;
                }
                break;
            }

            r += dir[0];
            c += dir[1];
        }
    }

    int rookDirs[4][2] = {
        {-1,0},
        { 1,0},
        { 0,-1},
        { 0, 1}
    };

    for(auto& dir : rookDirs){
        int r = row + dir[0];
        int c = col + dir[1];

        while(in_bounds(r,c)){
            int piece = get_piece(r,c);

            if(piece != 0){
                if(piece == rook || piece == queen){
                    return true;
                }

                break;
            }
            r += dir[0];
            c += dir[1];
        }
    }

    for(int r = row - 1; r <= row + 1; r++){
        for(int c = col - 1;c <= col + 1; c++){

            if(r == row && c == col){
                continue;
            }
            if(in_bounds(r,c) && get_piece(r,c) == king){
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
    std::vector<Move> legalMoves;

    for(int row=0;row<8;row++){
        for(int col=0;col<8;col++){

            int piece=get_piece(row,col);

            if(piece==0)
                continue;

            if(whiteTurn && piece<0)
                continue;

            if(!whiteTurn && piece>0)
                continue;

            std::vector<Move> moves;

            switch(abs(piece)){

                case 1:{ // pawn
                    int dir=(piece>0)?-1:1;

                    Move m1{
                        row,col,
                        row+dir,col
                    };

                    if(in_bounds(m1.toRow,m1.toCol) && is_valid_move(m1))
                        moves.push_back(m1);

                    Move m2{
                        row,col,
                        row+2*dir,col
                    };

                    if(in_bounds(m2.toRow,m2.toCol) && is_valid_move(m2))
                        moves.push_back(m2);

                    Move c1{
                        row,col,
                        row+dir,col-1
                    };

                    if(in_bounds(c1.toRow,c1.toCol) && is_valid_move(c1))
                        moves.push_back(c1);

                    Move c2{
                        row,col,
                        row+dir,col+1
                    };

                    if(in_bounds(c2.toRow,c2.toCol) && is_valid_move(c2))
                        moves.push_back(c2);

                    break;
                }

                case 3:{ // knight

                    int offsets[8][2]={
                        {-2,-1},
                        {-2,1},
                        {-1,-2},
                        {-1,2},
                        {1,-2},
                        {1,2},
                        {2,-1},
                        {2,1}
                    };

                    for(auto& o:offsets){

                        Move m{
                            row,
                            col,
                            row+o[0],
                            col+o[1]
                        };

                        if(in_bounds(m.toRow,m.toCol) && is_valid_move(m))
                            moves.push_back(m);
                    }

                    break;
                }

                case 6:{ // king

                    for(int dr=-1;dr<=1;dr++){
                        for(int dc=-1;dc<=1;dc++){

                            if(dr==0 && dc==0)
                                continue;

                            Move m{
                                row,
                                col,
                                row+dr,
                                col+dc
                            };

                            if(in_bounds(m.toRow,m.toCol) && is_valid_move(m))
                                moves.push_back(m);
                        }
                    }

                    Move castleK{
                        row,col,row,col+2
                    };

                    Move castleQ{
                        row,col,row,col-2
                    };

                    if(is_valid_move(castleK))
                        moves.push_back(castleK);

                    if(is_valid_move(castleQ))
                        moves.push_back(castleQ);

                    break;
                }

                case 2:
                case 4:
                case 5:{ // bishop/rook/queen

                    std::vector<std::pair<int,int>>
                    dirs;

                    if(abs(piece)==2 ||
                    abs(piece)==5){

                        dirs.push_back({-1,-1});
                        dirs.push_back({-1,1});
                        dirs.push_back({1,-1});
                        dirs.push_back({1,1});
                    }

                    if(abs(piece)==4 ||
                    abs(piece)==5){

                        dirs.push_back({-1,0});
                        dirs.push_back({1,0});
                        dirs.push_back({0,-1});
                        dirs.push_back({0,1});
                    }

                    for(auto& d:dirs){

                        int r=row+d.first;
                        int c=col+d.second;

                        while(in_bounds(r,c)){

                            Move m{
                                row,
                                col,
                                r,
                                c
                            };

                            if(is_valid_move(m))
                                moves.push_back(m);

                            if(get_piece(r,c)!=0)
                                break;

                            r+=d.first;
                            c+=d.second;
                        }
                    }

                    break;
                }
            }

            for(Move& m:moves){
                bool movingWhite=get_piece(m.fromRow,m.fromCol)>0;

                GameState state=save_state();
                make_move(m);

                if(!is_in_check(movingWhite)){
                    legalMoves.push_back(m);
                }

                undo_move(m);
                restore_state(state);
            }
        }
    }

    return legalMoves;
}

int Board::get_pst_score(int piece,int row, int col){

    if(piece < 0){
        row = 7-row;
    }

    switch(abs(piece)){
        case 1:
            return pawnTable[row][col];
        case 2:
            return bishopTable[row][col];
        case 3:
            return knightTable[row][col];
        case 4:
            return rookTable[row][col];
        case 5:
            return queenTable[row][col];
        case 6:
            return kingTable[row][col];
    }

    return 0;
}

int Board::count_piece_mobility(int row, int col){
    int piece = get_piece(row,col);

    if(piece==0)
        return 0;

    int mobility=0;

    switch(abs(piece)){

        case 3:{ // knight

            int offsets[8][2]={
                {-2,-1},
                {-2,1},
                {-1,-2},
                {-1,2},
                {1,-2},
                {1,2},
                {2,-1},
                {2,1}
            };

            for(auto& o:offsets){

                int r=row+o[0];
                int c=col+o[1];

                if(in_bounds(r,c)){

                    int target=get_piece(r,c);

                    if(target==0 || !same_color(piece,target)){
                        mobility++;
                    }
                }
            }

            break;
        }

        case 2:
        case 4:
        case 5:{

            std::vector<std::pair<int,int>> dirs;

            if(abs(piece)==2 || abs(piece)==5){
                dirs.push_back({-1,-1});
                dirs.push_back({-1,1});
                dirs.push_back({1,-1});
                dirs.push_back({1,1});
            }

            if(abs(piece)==4 || abs(piece)==5){
                dirs.push_back({-1,0});
                dirs.push_back({1,0});
                dirs.push_back({0,-1});
                dirs.push_back({0,1});
            }

            for(auto& d:dirs){
                int r= row+d.first;

                int c= col+d.second;

                while(in_bounds(r,c)){

                    int target=get_piece(r,c);

                    if(target==0){
                        mobility++;
                    }
                    else{
                        if(!same_color(piece,target)){
                            mobility++;
                        }
                        break;
                    }

                    r+=d.first;
                    c+=d.second;
                }
            }
            break;
        }

        case 6:{ // king
            for(int dr=-1;dr<=1; dr++){
                for(int dc=-1;dc<=1;dc++){

                    if(dr==0 && dc==0){
                        continue;
                    }

                    int r=row+dr;
                    int c=col+dc;

                    if(in_bounds(r,c)){
                        int target=get_piece(r,c);

                        if(target==0 || !same_color(piece,target)){
                            mobility++;
                        }
                    }
                }
            }

            break;
        }

    }

    return mobility;
}

int Board::evaluate_position(){

    int score=0;

    for(int row=0;row<8;row++){
        for(int col=0;col<8;col++){

            int piece=get_piece(row,col);

            if(piece==0){
                continue;
            }

            int value=0;

            switch(abs(piece)){
                case 1:
                    value=100;
                    break;
                case 2:
                    value=330;
                    break;
                case 3:
                    value=320;
                    break;
                case 4:
                    value=500;
                    break;
                case 5:
                    value=900;
                    break;
                case 6:
                    value=20000;
                    break;
            }

            int pst=get_pst_score(piece,row,col);
            int mobility=count_piece_mobility(row,col);

            int mobilityBonus=0;

            switch(abs(piece)){
                case 2:
                case 3:
                    mobilityBonus=mobility*4;
                    break;
                case 4:
                    mobilityBonus=mobility*2;
                    break;
                case 5:
                    mobilityBonus=mobility;
                    break;
            }

            int total = value+pst+mobilityBonus;

            score += (piece>0) ? total:-total;
        }
    }

    return score;
}

void Board::undo_move(const Move& m){

    whiteTurn = !whiteTurn;
    int piece = get_piece(m.toRow, m.toCol);

    if(m.isPromotion){
        if(piece > 0)
            piece = 1;
        else
            piece = -1;
    }

    board[m.fromRow][m.fromCol] = piece;
    board[m.toRow][m.toCol] = m.capturedPiece;

    if(m.isEnPassant){
        board[m.toRow][m.toCol] = 0;

        if(piece > 0){
            board[m.toRow + 1][m.toCol] = -1;
        }

        else{
            board[m.toRow - 1][m.toCol] = 1;
        }
    }

    if(m.isCastle){
        if(piece > 0){

            if(m.toCol == 6){
                board[7][7] = 4;
                board[7][5] = 0;
            }
            else{
                board[7][0] = 4;
                board[7][3] = 0;
            }
        }
        else{
            if(m.toCol == 6){
                board[0][7] = -4;
                board[0][5] = 0;
            }
            else{
                board[0][0] = -4;
                board[0][3] = 0;
            }
        }
    }

    enPassantRow = -1;
    enPassantCol = -1;

    if(!positionHistory.empty()){
        positionHistory.pop_back();
    }
}
    


