#include <iostream>
#include <vector>

#include "utils.h"
#include "board.h"
#include "evaluation.h"
#include "zobrist.h"

#include <sstream>



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

    initialize_hash();
    repetitionHistory.clear();
    repetitionHistory.push_back(zobristKey);
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

int Board::pieceIndex(int piece){

    switch(piece){

        case 1: return 0;
        case 2: return 1;
        case 3: return 2;
        case 4: return 3;
        case 5: return 4;
        case 6: return 5;

        case -1: return 6;
        case -2: return 7;
        case -3: return 8;
        case -4: return 9;
        case -5: return 10;
        case -6: return 11;
    }

    return -1;
}

void Board::initialize_hash(){

    zobristKey=0;

    for(int row=0;row<8;row++){
        for(int col=0;col<8;col++){

            int piece=board[row][col];

            if(piece==0)
                continue;

            int square=row*8+col;

            zobristKey^=Zobrist::pieces[pieceIndex(piece)][square];
        }
    }

    if(!whiteTurn)
        zobristKey^=Zobrist::sideToMove;

    int castleRights=0;

    if(!whiteKingsideRookMoved)
        castleRights|=1;

    if(!whiteQueensideRookMoved)
        castleRights|=2;

    if(!blackKingsideRookMoved)
        castleRights|=4;

    if(!blackQueensideRookMoved)
        castleRights|=8;

    zobristKey^=Zobrist::castle[castleRights];

    if(enPassantCol!=-1){

        zobristKey^=Zobrist::enPassant[enPassantCol];
    }
}

uint64_t Board::get_hash() const{
    return zobristKey;
}

void Board::load_fen(const std::string& fen){
    for(int r = 0; r < 8; r++)
        for(int c = 0; c < 8; c++)
            board[r][c] = 0;

    std::istringstream iss(fen);
    std::string placement, side, castling, ep;
    int halfmove = 0, fullmove = 1;

    iss >> placement >> side >> castling >> ep >> halfmove >> fullmove;

    int row = 0, col = 0;
    for(char c : placement){
        if(c == '/'){
            row++;
            col = 0;
        }
        else if(isdigit(c)){
            col += c - '0';
        }
        else{
            int piece = 0;
            switch(tolower(c)){
                case 'p': piece = 1; break;
                case 'b': piece = 2; break;
                case 'n': piece = 3; break;
                case 'r': piece = 4; break;
                case 'q': piece = 5; break;
                case 'k': piece = 6; break;
            }
            if(islower(c)) piece = -piece; 
            board[row][col] = piece;
            col++;
        }
    }

    whiteTurn = (side == "w");

    whiteKingMoved            = true;
    blackKingMoved            = true;
    whiteKingsideRookMoved    = true;
    whiteQueensideRookMoved   = true;
    blackKingsideRookMoved    = true;
    blackQueensideRookMoved   = true;

    if(castling != "-"){
        for(char c : castling){
            switch(c){
                case 'K': whiteKingMoved = false; whiteKingsideRookMoved  = false; break;
                case 'Q': whiteKingMoved = false; whiteQueensideRookMoved = false; break;
                case 'k': blackKingMoved = false; blackKingsideRookMoved  = false; break;
                case 'q': blackKingMoved = false; blackQueensideRookMoved = false; break;
            }
        }
    }

    enPassantRow = -1;
    enPassantCol = -1;
    if(ep != "-"){
        enPassantCol = ep[0] - 'a';          
        enPassantRow = 7 - (ep[1] - '1');   
    }

    fiftymoveClock = halfmove;

    initialize_hash();

    repetitionHistory.clear();
    repetitionHistory.push_back(zobristKey);
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
    int count=0;

    for(uint64_t h : repetitionHistory){
        if(h==zobristKey){
            count++;
        }
    }

    return count>=3;
}

int Board::get_piece(int row,int col){
    return board[row][col];
}

int Board::castleRights() {

    int rights=0;

    if(!whiteKingsideRookMoved)
        rights|=1;

    if(!whiteQueensideRookMoved)
        rights|=2;

    if(!blackKingsideRookMoved)
        rights|=4;

    if(!blackQueensideRookMoved)
        rights|=8;

    return rights;
}

bool Board::make_move(Move& m, Undo& u){

    m.isCastle = false;
    m.isEnPassant = false;
    m.isPromotion = false;
    m.promotionPiece = 0;

    int piece = get_piece(m.fromRow,m.fromCol);
    m.capturedPiece = get_piece(m.toRow, m.toCol);

    if(piece == 0){
        return false;
    }
    

    u.oldHash=zobristKey;

    u.oldEnPassantRow=enPassantRow;
    u.oldEnPassantCol=enPassantCol;

    u.oldWhiteKingMoved=whiteKingMoved;
    u.oldBlackKingMoved=blackKingMoved;

    u.oldWKRookMoved=whiteKingsideRookMoved;
    u.oldWQRookMoved=whiteQueensideRookMoved;

    u.oldBKRookMoved=blackKingsideRookMoved;
    u.oldBQRookMoved=blackQueensideRookMoved;

    u.oldFiftyMove=fiftymoveClock;

    if(enPassantCol!=-1){
        zobristKey ^= Zobrist::enPassant[enPassantCol];
    }
    zobristKey ^= Zobrist::castle[castleRights()];
    


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

    if(abs(piece)==1 && m.toRow==enPassantRow && m.toCol==enPassantCol && m.capturedPiece==0){
        m.isEnPassant=true;

        if(piece>0)
            m.capturedPiece=board[m.toRow+1][m.toCol];
        else
            m.capturedPiece=board[m.toRow-1][m.toCol];
    }


    

    int fromSq=m.fromRow*8+m.fromCol;
    int toSq=m.toRow*8+m.toCol;

    zobristKey^=Zobrist::pieces[pieceIndex(piece)][fromSq];

    if(m.capturedPiece!=0){

        int captureSq = toSq;

        if(m.isEnPassant){
            if(piece>0){
                captureSq=(m.toRow+1)*8+m.toCol;
                board[m.toRow+1][m.toCol]=0;
            }
            else{
                captureSq=(m.toRow-1)*8+m.toCol;
                board[m.toRow-1][m.toCol]=0;
            }
        }

        zobristKey ^=Zobrist::pieces[pieceIndex(m.capturedPiece)][captureSq];
    }
    if(m.isPromotion){

        int promoted=(piece>0)? m.promotionPiece:-m.promotionPiece;
        board[m.toRow][m.toCol]=promoted;

        zobristKey^= Zobrist::pieces[pieceIndex(promoted)][toSq];
    }
    else{
        board[m.toRow][m.toCol]=piece;
        zobristKey^=Zobrist::pieces[pieceIndex(piece)][toSq];
    }

    if(m.isCastle){

        if(piece==6){
            if(m.toCol==6){
                board[7][5]=4;
                board[7][7]=0;

                zobristKey ^= Zobrist::pieces[pieceIndex(4)][63];
                zobristKey ^=Zobrist::pieces[pieceIndex(4)][61];
            }
            else{
                board[7][3]=4;
                board[7][0]=0;

                zobristKey ^=Zobrist::pieces[pieceIndex(4)][56];
                zobristKey ^=Zobrist::pieces[pieceIndex(4)][59];
            }
        }

        else if(piece==-6){
            if(m.toCol==6){
                board[0][5]=-4;
                board[0][7]=0;

                zobristKey ^=Zobrist::pieces[pieceIndex(-4)][7];
                zobristKey ^=Zobrist::pieces[pieceIndex(-4)][5];
            }

            else{
                board[0][3]=-4;
                board[0][0]=0;

                zobristKey ^=Zobrist::pieces[pieceIndex(-4)][0];
                zobristKey ^=Zobrist::pieces[pieceIndex(-4)][3];
            }
        }
    }

    
   
    board[m.fromRow][m.fromCol] = 0;

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

    if(enPassantCol!=-1){
        zobristKey ^= Zobrist::enPassant[enPassantCol];
    }
    zobristKey^=Zobrist::castle[castleRights()];

    whiteTurn = !whiteTurn;

    zobristKey^=Zobrist::sideToMove;
    repetitionHistory.push_back(zobristKey);
    

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

                if(!whiteKingMoved && rowDiff==0 && abs(colDiff)==2){
                   
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

                if(!blackKingMoved && rowDiff == 0 && abs(colDiff) == 2){

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

    int pawnDir = byWhite ? -1 : 1;

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
                            int target = get_piece(r,c);
                            if(target != 0){
                                if(!same_color(piece, target)){
                                    Move m={row, col, r, c};
                                    m.capturedPiece = target;
                                    moves.push_back(m);
                                }
                                break;
                            }
                            moves.push_back(Move{row, col, r, c});
                            r += d.first; c += d.second;
                        }
                    }

                    break;
                }
            }

            for(Move& m:moves){
                bool movingWhite=get_piece(m.fromRow,m.fromCol)>0;
                
                Undo u;
                make_move(m,u);

                if(!is_in_check(movingWhite)){
                    legalMoves.push_back(m);
                }

                undo_move(m,u);
            }
        }
    }

    return legalMoves;
}

std::vector<Move> Board::generate_captures(){
    std::vector<Move> legalMoves;

    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){

            int piece = get_piece(row, col);

            if(piece == 0) continue;
            if(whiteTurn  && piece < 0) continue;
            if(!whiteTurn && piece > 0) continue;

            std::vector<Move> moves;

            switch(abs(piece)){

                case 1:{ // pawn — only diagonal captures + en passant
                    int dir = (piece > 0) ? -1 : 1;

                    // left capture
                    Move c1{ row, col, row + dir, col - 1 };
                    if(in_bounds(c1.toRow, c1.toCol)){
                        int target = get_piece(c1.toRow, c1.toCol);
                        bool isEnPassant = (c1.toRow == enPassantRow && c1.toCol == enPassantCol);
                        if((target != 0 && !same_color(piece, target)) || isEnPassant)
                            if(is_valid_move(c1)) moves.push_back(c1);
                    }

                    // right capture
                    Move c2{ row, col, row + dir, col + 1 };
                    if(in_bounds(c2.toRow, c2.toCol)){
                        int target = get_piece(c2.toRow, c2.toCol);
                        bool isEnPassant = (c2.toRow == enPassantRow && c2.toCol == enPassantCol);
                        if((target != 0 && !same_color(piece, target)) || isEnPassant)
                            if(is_valid_move(c2)) moves.push_back(c2);
                    }

                    break;
                }

                case 3:{ // knight
                    int offsets[8][2] = {
                        {-2,-1},{-2,1},{-1,-2},{-1,2},
                        {1,-2},{1,2},{2,-1},{2,1}
                    };

                    for(auto& o : offsets){
                        Move m{ row, col, row + o[0], col + o[1] };
                        if(in_bounds(m.toRow, m.toCol)){
                            int target = get_piece(m.toRow, m.toCol);
                            if(target != 0 && !same_color(piece, target))
                                if(is_valid_move(m)) moves.push_back(m);
                        }
                    }
                    break;
                }

                case 6:{ // king — only squares with enemy pieces (no castling)
                    for(int dr = -1; dr <= 1; dr++){
                        for(int dc = -1; dc <= 1; dc++){
                            if(dr == 0 && dc == 0) continue;
                            Move m{ row, col, row + dr, col + dc };
                            if(in_bounds(m.toRow, m.toCol)){
                                int target = get_piece(m.toRow, m.toCol);
                                if(target != 0 && !same_color(piece, target))
                                    if(is_valid_move(m)) moves.push_back(m);
                            }
                        }
                    }
                    break;
                }

                case 2:
                case 4:
                case 5:{ // bishop / rook / queen — slide until blocker, only add if enemy
                    std::vector<std::pair<int,int>> dirs;

                    if(abs(piece) == 2 || abs(piece) == 5){
                        dirs.push_back({-1,-1}); dirs.push_back({-1,1});
                        dirs.push_back({ 1,-1}); dirs.push_back({ 1,1});
                    }
                    if(abs(piece) == 4 || abs(piece) == 5){
                        dirs.push_back({-1,0}); dirs.push_back({1,0});
                        dirs.push_back({0,-1}); dirs.push_back({0,1});
                    }

                    for(auto& d : dirs){
                        int r = row + d.first;
                        int c = col + d.second;

                        while(in_bounds(r, c)){
                            int target = get_piece(r, c);

                            if(target != 0){
                                // blocker found — add only if enemy
                                if(!same_color(piece, target)){
                                    Move m{ row, col, r, c };
                                    if(is_valid_move(m)) moves.push_back(m);
                                }
                                break; // stop sliding regardless
                            }

                            r += d.first;
                            c += d.second;
                        }
                    }
                    break;
                }
            }

            // same legality filter as generate_moves
            for(Move& m : moves){
                bool movingWhite = get_piece(m.fromRow, m.fromCol) > 0;
                
                Undo u;
                make_move(m,u);

                if(!is_in_check(movingWhite)){
                    legalMoves.push_back(m);
                }

                undo_move(m,u);
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

    const int PHASE_LIMIT = 12; 
    int phase = 0;
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            int p = abs(get_piece(r, c));
            if(p == 2 || p == 3) phase += 1;
            if(p == 4)           phase += 2;
            if(p == 5)           phase += 4;
        }
    }
    bool endgame = (phase <= PHASE_LIMIT);

   
    int whitePawns[8] = {};
    int blackPawns[8] = {};
  
    int whitePawnRow[8]; 
    int blackPawnRow[8];
    for(int c = 0; c < 8; c++){
        whitePawnRow[c] = 7; 
        blackPawnRow[c] = 0;
    }

    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            int p = get_piece(r, c);
            if(p ==  1){ whitePawns[c]++; whitePawnRow[c] = std::min(whitePawnRow[c], r); }
            if(p == -1){ blackPawns[c]++; blackPawnRow[c] = std::max(blackPawnRow[c], r); }
        }
    }

    int score = 0;

    int whiteBishops = 0;
    int blackBishops = 0;

    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){
            int piece = get_piece(row, col);
            if(piece == 0) continue;

            bool isWhite = piece > 0;
            int  type    = abs(piece);

          
            int value = 0;
            switch(type){
                case 1: value = 100;   break;
                case 2: value = 330;   break;
                case 3: value = 320;   break;
                case 4: value = 500;   break;
                case 5: value = 900;   break;
                case 6: value = 20000; break;
            }

            
            int pstRow = isWhite ? row : 7 - row;
            int pst = 0;
            switch(type){
                case 1: pst = pawnTable[pstRow][col];   break;
                case 2: pst = bishopTable[pstRow][col]; break;
                case 3: pst = knightTable[pstRow][col]; break;
                case 4: pst = rookTable[pstRow][col];   break;
                case 5: pst = queenTable[pstRow][col];  break;
                case 6: pst = endgame
                            ? kingEndgameTable[pstRow][col]
                            : kingTable[pstRow][col];   break;
            }

           
            int mobility     = count_piece_mobility(row, col);
            int mobilityBonus = 0;
            switch(type){
                case 2: case 3: mobilityBonus = mobility * 4; break;
                case 4:         mobilityBonus = mobility * 2; break;
                case 5:         mobilityBonus = mobility * 1; break;
            }

           
            if(type == 2){
                if(isWhite) whiteBishops++;
                else        blackBishops++;
            }

            
            int pawnBonus = 0;
            if(type == 1){
                
                if(isWhite && whitePawns[col] > 1) pawnBonus -= 20;
                if(!isWhite && blackPawns[col] > 1) pawnBonus -= 20;

                
                bool isolated = true;
                if(col > 0 && (isWhite ? whitePawns[col-1] : blackPawns[col-1]) > 0) isolated = false;
                if(col < 7 && (isWhite ? whitePawns[col+1] : blackPawns[col+1]) > 0) isolated = false;
                if(isolated) pawnBonus -= 15;

               
                bool passed = true;
                if(isWhite){
                   
                    for(int fc = std::max(0, col-1); fc <= std::min(7, col+1); fc++){
                        
                        if(blackPawns[fc] > 0 && blackPawnRow[fc] <= row){
                            passed = false;
                            break;
                        }
                    }
                } else {
                    for(int fc = std::max(0, col-1); fc <= std::min(7, col+1); fc++){
                        if(whitePawns[fc] > 0 && whitePawnRow[fc] >= row){
                            passed = false;
                            break;
                        }
                    }
                }
                if(passed){
                  
                    int rank = isWhite ? (7 - row) : row; 
                    const int passedBonus[8] = {0, 10, 20, 30, 50, 75, 100, 0};
                    pawnBonus += passedBonus[rank];
                }
            }

           
            int rookBonus = 0;
            if(type == 4){
                bool noFriendlyPawn = (isWhite ? whitePawns[col] : blackPawns[col]) == 0;
                bool noEnemyPawn    = (isWhite ? blackPawns[col] : whitePawns[col]) == 0;

                if(noFriendlyPawn && noEnemyPawn) rookBonus += 20; 
                else if(noFriendlyPawn)           rookBonus += 10; 
            }

            int total = value + pst + mobilityBonus + pawnBonus + rookBonus;
            score += isWhite ? total : -total;
        }
    }

   
    if(whiteBishops >= 2) score += 30;
    if(blackBishops >= 2) score -= 30;

  
    if(!endgame){
        
        int whiteKingRow = -1, whiteKingCol = -1;
        int blackKingRow = -1, blackKingCol = -1;
        for(int r = 0; r < 8; r++){
            for(int c = 0; c < 8; c++){
                if(get_piece(r,c) ==  6){ whiteKingRow = r; whiteKingCol = c; }
                if(get_piece(r,c) == -6){ blackKingRow = r; blackKingCol = c; }
            }
        }

        
        auto pawnShield = [&](int kingRow, int kingCol, bool kingIsWhite) -> int {
            int shield = 0;
            int dir    = kingIsWhite ? -1 : 1; 
            int pawn   = kingIsWhite ?  1 : -1;

            for(int dc = -1; dc <= 1; dc++){
                int c = kingCol + dc;
                if(c < 0 || c > 7) continue;
               
                int r1 = kingRow + dir;
                if(r1 >= 0 && r1 < 8 && get_piece(r1, c) == pawn) shield += 10;
                
                int r2 = kingRow + 2*dir;
                if(r2 >= 0 && r2 < 8 && get_piece(r2, c) == pawn) shield += 5;
            }
            return shield;
        };

        if(whiteKingRow != -1) score += pawnShield(whiteKingRow, whiteKingCol, true);
        if(blackKingRow != -1) score -= pawnShield(blackKingRow, blackKingCol, false);

        
        auto kingOpenFilePenalty = [&](int kingCol, bool kingIsWhite) -> int {
            int penalty = 0;
            for(int dc = -1; dc <= 1; dc++){
                int c = kingCol + dc;
                if(c < 0 || c > 7) continue;
                bool noFriendly = (kingIsWhite ? whitePawns[c] : blackPawns[c]) == 0;
                bool noEnemy    = (kingIsWhite ? blackPawns[c] : whitePawns[c]) == 0;
                if(noFriendly && noEnemy) penalty += 20; 
                else if(noFriendly)      penalty += 10; 
            }
            return penalty;
        };

        if(whiteKingRow != -1) score -= kingOpenFilePenalty(whiteKingCol, true);
        if(blackKingRow != -1) score += kingOpenFilePenalty(blackKingCol, false);
    }

    return score;
}

void Board::undo_move(const Move& m, Undo& u){

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

        board[m.toRow][m.toCol]=0;

        if(piece>0)
            board[m.toRow+1][m.toCol]=m.capturedPiece;
        else
            board[m.toRow-1][m.toCol]=m.capturedPiece;
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
    
    if(!repetitionHistory.empty()){
        repetitionHistory.pop_back();
    }

    whiteKingMoved = u.oldWhiteKingMoved;
    blackKingMoved = u.oldBlackKingMoved;

    whiteKingsideRookMoved = u.oldWKRookMoved;
    whiteQueensideRookMoved = u.oldWQRookMoved;

    blackKingsideRookMoved = u.oldBKRookMoved;
    blackQueensideRookMoved = u.oldBQRookMoved;

    fiftymoveClock = u.oldFiftyMove;

    enPassantRow = u.oldEnPassantRow;
    enPassantCol = u.oldEnPassantCol;

    zobristKey=u.oldHash;
}
    
void Board::make_null_move(Undo& u) {
    u.oldEnPassantRow = enPassantRow;
    u.oldEnPassantCol = enPassantCol;
    u.oldHash = zobristKey;

    if(enPassantCol != -1)
        zobristKey ^= Zobrist::enPassant[enPassantCol];

    enPassantRow = -1;
    enPassantCol = -1;

    whiteTurn = !whiteTurn;
    zobristKey ^= Zobrist::sideToMove;

    repetitionHistory.push_back(zobristKey);
}

void Board::undo_null_move(Undo& u) {
    repetitionHistory.pop_back();
    whiteTurn = !whiteTurn;
    enPassantRow = u.oldEnPassantRow;
    enPassantCol = u.oldEnPassantCol;
    zobristKey = u.oldHash;
}

