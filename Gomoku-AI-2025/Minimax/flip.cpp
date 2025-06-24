#include "flip.h"
#include<iostream>
#include <utility>

extern int board[15][15]; 

enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

extern int evaluate(int ter);

int flip_score() {
    // 1. 临时翻转棋盘
    flip_board(); 

    // 2. 调用评估函数  
    int score = evaluate(2); 

    // 3. 恢复棋盘状态，撤销翻转
    flip_board(); 

    return score;
}

int no_flip_score() {
    int score_no_flip;
    std::pair<int, int> best_move = deepingMinimax().move; // 找到白棋的最佳下法

    board[best_move.first][best_move.second] = WHITE; // ai_side is 1 (WHITE)
    score_no_flip = evaluate(2); // 获取局面分
    board[best_move.first][best_move.second] = EMPTY; // 恢复棋盘

    return score_no_flip;
}

