#pragma once
#include <cstdint>

namespace Zobrist {

    extern uint64_t pieces[12][64];

    extern uint64_t sideToMove;

    extern uint64_t castle[16];

    extern uint64_t enPassant[8];

    void init();
}