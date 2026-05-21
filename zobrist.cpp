#include "zobrist.h"
#include <random>

namespace Zobrist {

    uint64_t pieces[12][64];

    uint64_t sideToMove;

    uint64_t castle[16];

    uint64_t enPassant[8];

    static uint64_t random_u64() {

        static std::mt19937_64 rng(1234567);
        return rng();
    }

    void init(){

        for(int p=0;p<12;p++){
            for(int sq=0;sq<64;sq++){
                pieces[p][sq]=random_u64();
            }
        }

        sideToMove=random_u64();

        for(int i=0;i<16;i++){
            castle[i]=random_u64();
        }

        for(int i=0;i<8;i++){
            enPassant[i]=random_u64();
        }
    }

}