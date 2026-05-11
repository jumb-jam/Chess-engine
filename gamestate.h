#pragma once

struct GameState{

    bool whiteTurn;

    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteKingsideRookMoved;
    bool whiteQueensideRookMoved;

    bool blackKingsideRookMoved;
    bool blackQueensideRookMoved;

    int enPassantRow;
    int enPassantCol;
};