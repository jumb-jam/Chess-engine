#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "board.h"

struct BookMove {
    int fromRow, fromCol, toRow, toCol;
};

class OpeningBook {
public:
    std::unordered_map<uint64_t, std::vector<BookMove>> book;

    void init();

    void add(uint64_t key, int fr, int fc, int tr, int tc){
        book[key].push_back({fr, fc, tr, tc});
    }

    // returns true and sets m if a book move exists
    bool probe(uint64_t key, BookMove& out){
        auto it = book.find(key);
        if(it == book.end() || it->second.empty()) return false;

        // pick randomly among book moves for variety
        int idx = rand() % it->second.size();
        out = it->second[idx];
        return true;
    }
};