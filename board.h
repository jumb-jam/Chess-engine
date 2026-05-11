#pragma once

#include <vector>

struct Move{
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;

    int capturedPiece = 0;

    bool isCastle = false;
    bool isEnPassant = false;
    bool isPromotion = false;

    int promotionPiece = 0;
};

class Board{
private:
    int board[8][8] = {0};

    bool whiteTurn = true;

    bool whiteKingMoved = false;
    bool blackKingMoved = false;

    bool whiteKingsideRookMoved = false;
    bool whiteQueensideRookMoved = false;

    bool blackKingsideRookMoved = false;
    bool blackQueensideRookMoved = false;

public:
    void init_board();

    bool is_white_turn();

    int get_piece(int row, int col);

    bool make_move(Move m);

    bool is_valid_move(const Move& m);

    bool is_square_attacked(int row, int col, bool byWhite);

    bool is_in_check(bool whiteKing);

    std::vector<Move> generate_moves();

    int evaluate_position();

    void undo_move(const Move& m);
};

