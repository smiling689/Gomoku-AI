#ifndef FLIP_H
#define FLIP_H

#include <utility>

extern int board[15][15];

struct MinimaxResult {
    std::pair<int, int> move;
    int score;
};

MinimaxResult Minimax(int dep);

#endif // FLIP_H