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

    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            if(board[r][c] ==  6){ whiteKingRow = r; whiteKingCol = c; }
            if(board[r][c] == -6){ blackKingRow = r; blackKingCol = c; }
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

    int start = std::max(0, (int)repetitionHistory.size() - fiftymoveClock - 1);
    for(int i = start; i < (int)repetitionHistory.size(); i++){
        if(repetitionHistory[i] == zobristKey) count++;
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
    //m.promotionPiece = 0;

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

    u.oldWhiteKingRow = whiteKingRow;
    u.oldWhiteKingCol = whiteKingCol;
    u.oldBlackKingRow = blackKingRow;
    u.oldBlackKingCol = blackKingCol;

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

        if(m.promotionPiece == 0)
            m.promotionPiece = 5;

        int promoted=(piece>0)? abs(m.promotionPiece):-abs(m.promotionPiece);
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
        whiteKingRow = m.toRow; 
        whiteKingCol = m.toCol;
    }
    if(piece == -6){
        blackKingMoved = true;
        blackKingRow = m.toRow; 
        blackKingCol = m.toCol;
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
    if(whiteKing)
        return is_square_attacked(whiteKingRow, whiteKingCol, false);
    else
        return is_square_attacked(blackKingRow, blackKingCol, true);
}

std::vector<Move> Board::generate_moves(){
    std::vector<Move> legalMoves;
    legalMoves.reserve(64);

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
                    int dir = (piece > 0) ? -1 : 1;

                    int r1 = row + dir;
                    if(in_bounds(r1, col) && get_piece(r1, col) == 0){
                        moves.push_back({row, col, r1, col});
                    }

                    int r2 = row + 2*dir;
                    int startRow = (piece > 0) ? 6 : 1;
                    if(row == startRow && get_piece(r1, col) == 0 && get_piece(r2, col) == 0){
                        moves.push_back({row, col, r2, col});
                    }

                    for(int dc : {-1, 1}){
                        int c = col + dc;
                        if(!in_bounds(r1, c)) continue;
                        int target = get_piece(r1, c);
                        bool isEP = (r1 == enPassantRow && c == enPassantCol);
                        if((target != 0 && !same_color(piece, target)) || isEP)
                            moves.push_back({row, col, r1, c});
                    }
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
                        int r = row + o[0], c = col + o[1];
                        if(in_bounds(r, c) && !same_color(piece, get_piece(r, c))){
                            moves.push_back({row, col, r, c});
                        }
                    }

                    break;
                }

                case 6:{ // king

                    for(int dr=-1;dr<=1;dr++){
                        for(int dc=-1;dc<=1;dc++){

                            if(dr==0 && dc==0){
                                continue;
                            }

                            int r = row + dr, c = col + dc;
                            if(in_bounds(r, c) && !same_color(piece, get_piece(r, c))){
                                moves.push_back({row, col, r, c});
                            }
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

                    static const int bishopDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
                    static const int rookDirs[4][2]   = {{-1,0},{1,0},{0,-1},{0,1}};

                    int numDirs = 0;
                    const int (*dirs)[2] = nullptr;

                    static const int queenDirs[8][2] = {
                        {-1,-1},{-1,1},{1,-1},{1,1},
                        {-1,0},{1,0},{0,-1},{0,1}
                    };

                    if(abs(piece) == 2){
                        dirs = bishopDirs; numDirs = 4;
                    } else if(abs(piece) == 4){
                        dirs = rookDirs;   numDirs = 4;
                    } else {
                        dirs = queenDirs;  numDirs = 8;
                    }

                    for(int d = 0; d < numDirs; d++){
                        int r = row + dirs[d][0];
                        int c = col + dirs[d][1];

                        while(in_bounds(r, c)){
                            int target = get_piece(r, c);
                            if(target != 0){
                                if(!same_color(piece, target))
                                    moves.push_back({row, col, r, c});
                                break;
                            }
                            moves.push_back({row, col, r, c});
                            r += dirs[d][0];
                            c += dirs[d][1];
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
    legalMoves.reserve(32);

    static const int bishopDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    static const int rookDirs[4][2]   = {{-1,0},{1,0},{0,-1},{0,1}};
    static const int queenDirs[8][2]  = {
        {-1,-1},{-1,1},{1,-1},{1,1},
        {-1,0},{1,0},{0,-1},{0,1}
    };
    static const int knightOffsets[8][2] = {
        {-2,-1},{-2,1},{-1,-2},{-1,2},
        {1,-2},{1,2},{2,-1},{2,1}
    };

    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){

            int piece = get_piece(row, col);
            if(piece == 0) continue;
            if(whiteTurn  && piece < 0) continue;
            if(!whiteTurn && piece > 0) continue;

            int type = abs(piece);
            std::vector<Move> moves;

            switch(type){

                case 1:{ 
                    int dir = (piece > 0) ? -1 : 1;
                    int r1  = row + dir;

                    for(int dc : {-1, 1}){
                        int c = col + dc;
                        if(!in_bounds(r1, c)) continue;

                        int target  = get_piece(r1, c);
                        bool isEP   = (r1 == enPassantRow && c == enPassantCol);

                        if((target != 0 && !same_color(piece, target)) || isEP){
                            Move m{row, col, r1, c};
                            m.capturedPiece = isEP ? (piece > 0 ? -1 : 1) : target;
                            moves.push_back(m);
                        }
                    }
                    break;
                }

                case 3:{ 
                    for(auto& o : knightOffsets){
                        int r = row + o[0], c = col + o[1];
                        if(!in_bounds(r, c)) continue;
                        int target = get_piece(r, c);
                        if(target != 0 && !same_color(piece, target)){
                            Move m{row, col, r, c};
                            m.capturedPiece = target;
                            moves.push_back(m);
                        }
                    }
                    break;
                }

                case 6:{ 
                    for(int dr = -1; dr <= 1; dr++){
                        for(int dc = -1; dc <= 1; dc++){
                            if(dr == 0 && dc == 0) continue;
                            int r = row + dr, c = col + dc;
                            if(!in_bounds(r, c)) continue;
                            int target = get_piece(r, c);
                            if(target != 0 && !same_color(piece, target)){
                                Move m{row, col, r, c};
                                m.capturedPiece = target;
                                moves.push_back(m);
                            }
                        }
                    }
                    break;
                }

                case 2:
                case 4:
                case 5:{ 
                    const int (*dirs)[2];
                    int numDirs;

                    if(type == 2){ dirs = bishopDirs; numDirs = 4; }
                    else if(type == 4){ dirs = rookDirs; numDirs = 4; }
                    else { dirs = queenDirs; numDirs = 8; }

                    for(int d = 0; d < numDirs; d++){
                        int r = row + dirs[d][0];
                        int c = col + dirs[d][1];

                        while(in_bounds(r, c)){
                            int target = get_piece(r, c);
                            if(target != 0){
                                if(!same_color(piece, target)){
                                    Move m{row, col, r, c};
                                    m.capturedPiece = target;
                                    moves.push_back(m);
                                }
                                break;
                            }
                            r += dirs[d][0];
                            c += dirs[d][1];
                        }
                    }
                    break;
                }
            }

            for(Move& m : moves){
                bool movingWhite = piece > 0;
                Undo u;
                make_move(m, u);

                if(!is_in_check(movingWhite))
                    legalMoves.push_back(m);

                undo_move(m, u);
            }
        }
    }

    return legalMoves;
}

int Board::evaluate_position(){
    int mgScore = 0;
    int egScore = 0;
    int phase   = 0;


    int whitePawns[8] = {0};  
    int blackPawns[8] = {0};
 

    int whitePawnRow[8];  
    int blackPawnRow[8];  
    for(int i = 0; i < 8; i++){
        whitePawnRow[i] = 7; 
        blackPawnRow[i] = 0;
    }
 
    int whiteBishops = 0;
    int blackBishops = 0;
 
   
    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){
 
            int piece = board[row][col];
            if(piece == 0) continue;
 
            bool isWhite = piece > 0;
            int  type    = abs(piece);
 
 
            int sq = isWhite ? (row * 8 + col) : ((7 - row) * 8 + col);
 
            int mg = mgValue[type] + mgPST[type][sq];
            int eg = egValue[type] + egPST[type][sq];
 
            if(isWhite){ mgScore += mg; egScore += eg; }
            else        { mgScore -= mg; egScore -= eg; }
 
            phase += phaseWeight[type];
 
   
            if(type == 1){
                if(isWhite){
                    whitePawns[col]++;
                    if(row < whitePawnRow[col]) whitePawnRow[col] = row;
                } else {
                    blackPawns[col]++;
                    if(row > blackPawnRow[col]) blackPawnRow[col] = row;
                }
            }
            if(type == 2){
                if(isWhite) whiteBishops++;
                else        blackBishops++;
            }
        }
    }
 
    if(phase > 24) phase = 24;
 
    int structureScore = 0;
 
    // Passed pawn bonuses — scale by advancement rank
    static const int passedMG[8] = {  0,  10,  20,  30,  50,  75, 100,   0 };
    static const int passedEG[8] = {  0,  20,  40,  60,  90, 130, 180,   0 };
 
    // Doubled pawn penalty
    static const int doubledMG = -12;
    static const int doubledEG = -20;
 
    // Isolated pawn penalty
    static const int isolatedMG = -15;
    static const int isolatedEG = -20;
 
    // Bishop pair bonus
    static const int bishopPairMG = 25;
    static const int bishopPairEG = 50;
 
    // Rook on open/semi-open file bonus
    static const int rookOpenMG   = 20;
    static const int rookSemiMG   = 10;
    static const int rookOpenEG   = 15;
    static const int rookSemiEG   =  8;
 
    // Pawn shield 
    static const int shieldClose  = 10;  
    static const int shieldFar    =  5;  
    static const int openFilePenalty = 15; 
 
    int mgStructure = 0;
    int egStructure = 0;
 
    for(int col = 0; col < 8; col++){
 
        // ── White pawns ──
        if(whitePawns[col] > 0){
 
            // Doubled
            if(whitePawns[col] > 1){
                mgStructure += (whitePawns[col] - 1) * doubledMG;
                egStructure += (whitePawns[col] - 1) * doubledEG;
            }
 
            // Isolated
            bool leftEmpty  = (col == 0 || whitePawns[col-1] == 0);
            bool rightEmpty = (col == 7 || whitePawns[col+1] == 0);
            if(leftEmpty && rightEmpty){
                mgStructure += isolatedMG;
                egStructure += isolatedEG;
            }
 
            // Passed
            bool passed = true;
            int advRow = whitePawnRow[col]; 
            for(int fc = std::max(0, col-1); fc <= std::min(7, col+1); fc++){
                
                if(blackPawns[fc] > 0 && blackPawnRow[fc] <= advRow){
                    passed = false;
                    break;
                }
            }
            if(passed){
                int rank = 7 - advRow; 
                mgStructure += passedMG[rank];
                egStructure += passedEG[rank];
            }
        }
 
        if(blackPawns[col] > 0){
 
            // Doubled
            if(blackPawns[col] > 1){
                mgStructure -= (blackPawns[col] - 1) * doubledMG;
                egStructure -= (blackPawns[col] - 1) * doubledEG;
            }
 
            // Isolated
            bool leftEmpty  = (col == 0 || blackPawns[col-1] == 0);
            bool rightEmpty = (col == 7 || blackPawns[col+1] == 0);
            if(leftEmpty && rightEmpty){
                mgStructure -= isolatedMG;
                egStructure -= isolatedEG;
            }
 
            // Passed
            bool passed = true;
            int advRow = blackPawnRow[col]; 
            for(int fc = std::max(0, col-1); fc <= std::min(7, col+1); fc++){
                if(whitePawns[fc] > 0 && whitePawnRow[fc] >= advRow){
                    passed = false;
                    break;
                }
            }
            if(passed){
                int rank = advRow; 
                mgStructure -= passedMG[rank];
                egStructure -= passedEG[rank];
            }
        }
    }
 
    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){
            int piece = board[row][col];
            if(abs(piece) != 4) continue;
 
            bool isWhite = piece > 0;
            bool noFriendly = isWhite ? (whitePawns[col] == 0) : (blackPawns[col] == 0);
            bool noEnemy    = isWhite ? (blackPawns[col] == 0) : (whitePawns[col] == 0);
 
            int mg = 0, eg = 0;
            if(noFriendly && noEnemy){ mg = rookOpenMG; eg = rookOpenEG; }
            else if(noFriendly)      { mg = rookSemiMG; eg = rookSemiEG; }
 
            if(isWhite){ mgStructure += mg; egStructure += eg; }
            else       { mgStructure -= mg; egStructure -= eg; }
        }
    }
 
    if(whiteBishops >= 2){ mgStructure += bishopPairMG; egStructure += bishopPairEG; }
    if(blackBishops >= 2){ mgStructure -= bishopPairMG; egStructure -= bishopPairEG; }
 

    if(phase > 8){
 
        auto pawnShield = [&](int kingRow, int kingCol, bool kingIsWhite) -> int {
            int mg = 0;
            int dir  = kingIsWhite ? -1 : 1;  // direction toward own pawns
            int pawn = kingIsWhite ?  1 : -1;
 
            for(int dc = -1; dc <= 1; dc++){
                int c = kingCol + dc;
                if(c < 0 || c > 7) continue;
 
                int r1 = kingRow + dir;
                if(r1 >= 0 && r1 < 8 && board[r1][c] == pawn) mg += shieldClose;
 
                int r2 = kingRow + 2 * dir;
                if(r2 >= 0 && r2 < 8 && board[r2][c] == pawn) mg += shieldFar;
 
                bool noFriendly = kingIsWhite ? (whitePawns[c] == 0)
                                              : (blackPawns[c] == 0);
                bool noEnemy    = kingIsWhite ? (blackPawns[c] == 0)
                                              : (whitePawns[c] == 0);
                if(noFriendly && noEnemy) mg -= openFilePenalty;
                else if(noFriendly)       mg -= openFilePenalty / 2;
            }
            return mg;
        };
 
        int wShield = pawnShield(whiteKingRow, whiteKingCol, true);
        int bShield = pawnShield(blackKingRow, blackKingCol, false);
 
  
        int kingSafetyPhase = phase - 8;  
        mgStructure += (wShield - bShield) * kingSafetyPhase / 16;
    }
 
    mgScore += mgStructure;
    egScore += egStructure;
 
    int score = (mgScore * phase + egScore * (24 - phase)) / 24;
 
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

    whiteKingRow = u.oldWhiteKingRow;
    whiteKingCol = u.oldWhiteKingCol;
    blackKingRow = u.oldBlackKingRow;
    blackKingCol = u.oldBlackKingCol;

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

int Board::get_smallest_attacker(int row, int col, bool byWhite, int& fromRow, int& fromCol){
    
    int pawnDir = byWhite ? 1 : -1; // direction pawns come FROM
    int pawn    = byWhite ? 1 : -1;
    
    for(int dc = -1; dc <= 1; dc += 2){
        int r = row + pawnDir;
        int c = col + dc;
        if(in_bounds(r, c) && get_piece(r, c) == pawn){
            fromRow = r; fromCol = c;
            return pieceValue[1];
        }
    }

    // knights
    int knightOffsets[8][2] = {
        {-2,-1},{-2,1},{-1,-2},{-1,2},
        {1,-2},{1,2},{2,-1},{2,1}
    };
    int knight = byWhite ? 3 : -3;
    for(auto& o : knightOffsets){
        int r = row + o[0], c = col + o[1];
        if(in_bounds(r,c) && get_piece(r,c) == knight){
            fromRow = r; fromCol = c;
            return pieceValue[3];
        }
    }

    // bishops
    int bishopDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    int bishop = byWhite ? 2 : -2;
    for(auto& d : bishopDirs){
        int r = row + d[0], c = col + d[1];
        while(in_bounds(r,c)){
            int p = get_piece(r,c);
            if(p != 0){
                if(p == bishop){ fromRow = r; fromCol = c; return pieceValue[2]; }
                break;
            }
            r += d[0]; c += d[1];
        }
    }

    // rooks
    int rookDirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int rook = byWhite ? 4 : -4;
    for(auto& d : rookDirs){
        int r = row + d[0], c = col + d[1];
        while(in_bounds(r,c)){
            int p = get_piece(r,c);
            if(p != 0){
                if(p == rook){ fromRow = r; fromCol = c; return pieceValue[4]; }
                break;
            }
            r += d[0]; c += d[1];
        }
    }

    // queen (bishops + rooks combined)
    int queen = byWhite ? 5 : -5;
    for(auto& d : bishopDirs){
        int r = row + d[0], c = col + d[1];
        while(in_bounds(r,c)){
            int p = get_piece(r,c);
            if(p != 0){
                if(p == queen){ fromRow = r; fromCol = c; return pieceValue[5]; }
                break;
            }
            r += d[0]; c += d[1];
        }
    }
    for(auto& d : rookDirs){
        int r = row + d[0], c = col + d[1];
        while(in_bounds(r,c)){
            int p = get_piece(r,c);
            if(p != 0){
                if(p == queen){ fromRow = r; fromCol = c; return pieceValue[5]; }
                break;
            }
            r += d[0]; c += d[1];
        }
    }

    // king
    int king = byWhite ? 6 : -6;
    for(int dr = -1; dr <= 1; dr++){
        for(int dc = -1; dc <= 1; dc++){
            if(dr == 0 && dc == 0) continue;
            int r = row + dr, c = col + dc;
            if(in_bounds(r,c) && get_piece(r,c) == king){
                fromRow = r; fromCol = c;
                return pieceValue[6];
            }
        }
    }

    return -1; // no attacker found
}

int Board::see(int toRow, int toCol, int target, int fromRow, int fromCol){
    
    int value = 0;
    int attacker = abs(get_piece(fromRow, fromCol));
    
    // simulate the capture
    int captured = board[toRow][toCol];
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = 0;

    // find the least valuable opponent attacker
    int nextFromRow, nextFromCol;
    bool byWhite = (captured > 0); // opponent of who just captured
    
    int nextAttackerVal = get_smallest_attacker(
        toRow, toCol, byWhite, nextFromRow, nextFromCol);

    if(nextAttackerVal >= 0){
        // opponent can recapture — recurse
        value = std::max(0, pieceValue[abs(captured)] - 
                see(toRow, toCol, abs(captured), nextFromRow, nextFromCol));
    } else {
        // no recapture available
        value = pieceValue[abs(captured)];
    }

    // restore the board
    board[fromRow][fromCol] = board[toRow][toCol];
    board[toRow][toCol] = captured;

    return value;
}