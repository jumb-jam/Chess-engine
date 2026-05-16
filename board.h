#pragma once

#include "evaluation.h"
#include "gamestate.h"
#include <vector>
#include <string>

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

    int enPassantRow = -1;
    int enPassantCol = -1;

    int fiftymoveClock=0;

    std::vector<std::string> positionHistory;

public:
    void init_board();

    void print_board();

    std::string get_position_key() const;

    GameState save_state() const;

    void restore_state(const GameState& state);

    bool is_white_turn();

    bool is_checkmate();

    bool is_stalemate();

    bool is_fifty_move_draw();

    bool is_threefold_repetition();

    int get_piece(int row, int col);

    bool make_move(Move& m);

    bool is_valid_move(const Move& m);

    bool is_square_attacked(int row, int col, bool byWhite);

    bool is_in_check(bool whiteKing);

    std::vector<Move> generate_moves();

    int get_pst_score(int piece, int row, int col);

    int count_piece_mobility(int row, int col);

    int evaluate_position();

    void undo_move(const Move& m);
};

