#pragma once

#include "evaluation.h"
#include "gamestate.h"
#include <vector>
#include <string>
#include <cstdint>


const int pieceValue[7]={
        0,
        100,   // pawn
        330,   // bishop
        320,   // knight
        500,   // rook
        900,   // queen
        20000  // king
};

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
    int score=0;
};

struct Undo{

    int capturedPiece;

    int oldEnPassantRow;
    int oldEnPassantCol;

    bool wasNullMove;

    bool oldWhiteKingMoved;
    bool oldBlackKingMoved;

    bool oldWKRookMoved;
    bool oldWQRookMoved;

    bool oldBKRookMoved;
    bool oldBQRookMoved;

    int oldFiftyMove;

    uint64_t oldHash;
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

    uint64_t zobristKey=0;
    std::vector<uint64_t> repetitionHistory;

public:
    void init_board();

    void print_board();

    int pieceIndex(int piece);

    void initialize_hash();

    uint64_t get_hash() const;

    void load_fen(const std::string& fen);

    bool is_white_turn();

    bool is_checkmate();

    bool is_stalemate();

    bool is_fifty_move_draw();

    bool is_threefold_repetition();

    int get_piece(int row, int col);

    int castleRights();

    bool make_move(Move& m, Undo& u);

    bool is_valid_move(const Move& m);

    bool is_square_attacked(int row, int col, bool byWhite);

    bool is_in_check(bool whiteKing);

    std::vector<Move> generate_moves();

    std::vector<Move> generate_captures();

    int get_pst_score(int piece, int row, int col);

    int count_piece_mobility(int row, int col);

    int evaluate_position();

    void undo_move(const Move& m, Undo& u);

    void make_null_move(Undo& u);

    void undo_null_move(Undo& u);
};

