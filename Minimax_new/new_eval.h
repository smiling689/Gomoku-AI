#ifndef OTHER_EVAL_H
#define OTHER_EVAL_H

#include<vector>

extern int board[15][15];

std::vector<std::pair<int , int> > generate_sorted_moves(int num) ;

// void init_eval();

int score_move(int r , int c);

int evaluate(int color);

int winner();

namespace Scores {
    constexpr int FIVE = 100000000;
    constexpr int LIVE_FOUR = 1000000;
    constexpr int DEAD_FOUR = 1000;
    constexpr int LIVE_THREE = 1000;
    constexpr int DEAD_THREE = 100;
    constexpr int LIVE_TWO = 100;
    constexpr int DEAD_TWO = 1;
    constexpr int LIVE_ONE = 1;
    constexpr int JUMP_LIVE_FOUR = LIVE_FOUR / 2;
    constexpr int JUMP_DEAD_FOUR = DEAD_FOUR / 2;
    constexpr int JUMP_LIVE_THREE = LIVE_THREE / 2;
}

namespace Point_Scores {
    constexpr int FIVE = 1000000;
    constexpr int LIVE_FOUR = 10000;
    constexpr int DEAD_FOUR = 100;
    constexpr int LIVE_THREE = 100;
    constexpr int DEAD_THREE = 10;
    constexpr int LIVE_TWO = 10;
    constexpr int DEAD_TWO = 1;
    constexpr int LIVE_ONE = 1;
    constexpr int ZERO = 1;
}


namespace GomokuLegacyEval {

    /**
     * @brief 评估整个棋盘对于指定颜色的分数。
     * * @param board const引用，指向一个15x15的棋盘。
     * @param eval_color 要为哪个颜色进行评估 (例如 0 代表黑棋, 1 代表白棋)。
     * @return 返回棋盘的总评估分数。
     */
    int evaluate(const int (&board)[15][15], int eval_color);

} // namespace GomokuLegacyEval


#endif // OTHER_EVAL_H