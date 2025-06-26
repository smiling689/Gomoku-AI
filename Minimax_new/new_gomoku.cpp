// g++ Minimax_new/new_gomoku.cpp Minimax_new/new_eval.cpp Minimax_new/flip.cpp Minimax_new/zobrist.cpp Minimax_new/vcx.cpp -o gomoku           编译
// python Minimax_new/judge.py ./gomoku ./baseline                                                                      时，我的ai是ai0(先手，黑棋)
// python Minimax_new/judge.py ./baseline ./gomoku                                                                      时，我的ai是ai1(后手，白棋)
#include "AIController.h"
#include "zobrist.h"
#include "flip.h"
#include "vcx.h"
#include <utility>
#include <cstring>
#include <climits>
#include <vector>
#include <ctime>
#include <algorithm>
#include "new_eval.h"
#include<random>
#include<chrono>
// #define DEBUG_MODE //是否开启调试模式
// #define timing
// #define pos_val

#define DEBUG_MODE //是否开启调试模式
// #define timing
// #define pos_val
#define huanshou


extern int ai_side; //0: black, 1: white
//ai_side == 0：代表AI执黑棋。
//ai_side == 1：代表AI执白棋。
//对手是 1-ai_side
std::string ai_name = "Smiling_AI";

int turn = 0;
int current_turn = 0;

const int SIZE = 15;
int board[SIZE][SIZE];
const int INF = INT_MAX;
const int DEP = 6; //depth接口，表示搜索深度
// const int save_moves = 7;//从生成的可能moves中选前若干个进行计算
// int save_moves[8] = {7, 7, 7, 7, 7, 7, 7, 7};
const int VCX_WIN_SCORE = INF - 1;
const int VCX_LOSE_SCORE = 1 - INF;


enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

struct MoveScore {
    std::pair<int, int> move;
    int score;
};

const int POSITION_WEIGHT_FACTOR = 10;




MinimaxResult Minimax(int depth);

int min_value(int alpha, int beta, int depth);

int max_value(int alpha, int beta, int depth);


namespace rand_int {
    std::mt19937 rnd(std::chrono::system_clock::now().time_since_epoch().count());
    int rd_get(int l, int r) { return rnd() % (r - l + 1ll) + l; } // 获得 [l , r] 之间的随机数
}

using rand_int::rd_get;


void init() {
    srand(time(0));
    // transposition_table.clear();
    memset(board, EMPTY, sizeof(board));
    init_zobrist();
    init_eval();
}

std::pair<int, int> getRandom() {
    while (true) {
        int x = rd_get(0, 14);
        int y = rd_get(0, 14);
        if (board[x][y] == EMPTY) {
            board[x][y] = ai_side;
            return std::make_pair(x, y);
        }
    }
}

bool is_valid(int r, int c) {
    return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
}

bool adj(int r, int c) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            if (is_valid(r + i, c + j) && board[r + i][c + j] != EMPTY) {
                return true;
            }
        }
    }
    return false;
}


// 一个辅助函数，用来计算棋盘上的棋子数
int count_black() {
    int count = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == BLACK) {
                count++;
            }
        }
    }
    return count;
}

int count_white() {
    int count = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == WHITE) {
                count++;
            }
        }
    }
    return count;
}

void flip_board() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == BLACK) {
                board[i][j] = WHITE;
            } else if (board[i][j] == WHITE) {
                board[i][j] = BLACK;
            }
        }
    }
    calculate_hash();
}

std::pair<int, int> action(std::pair<int, int> loc) {
    turn++;
    if (loc.first != -1 && loc.second != -1)
        board[loc.first][loc.second] = 1 - ai_side; 

    if (loc.first == -1 && loc.second == -1 && turn != 1)
        flip_board(); 

    int black = count_black();
    int white = count_white();
    current_turn = black + white + 1;

    if (current_turn == SIZE * SIZE) {
        return getRandom();
    }

    if (current_turn == 1) {
        int x = rd_get(1 , 14);
        int y = rd_get(1 , 14);
        board[x][y] = ai_side;
        update_hash(x , y , ai_side);
        return {x, y};
    } 
    
    VCXResult vcx_res = vcx_search(10); 
    if (vcx_res.score >= VCX_WIN_SCORE) { 
        board[vcx_res.step.first][vcx_res.step.second] = ai_side;
        update_hash(vcx_res.step.first , vcx_res.step.second , ai_side);
        return vcx_res.step;
    }

    // 4. 对于所有常规回合，使用 Minimax 算法计算最佳落子
    MinimaxResult my_move = Minimax(turn <= 4 ? 2  : 6);

    if (my_move.move.first != -1 && my_move.move.second != -1) {
        // int i = my_move.move.first, j = my_move.move.second;
        board[my_move.move.first][my_move.move.second] = ai_side;
        update_hash(my_move.move.first, my_move.move.second, ai_side);
    } else {
        std::cerr << "MiniMax wrong! -1,-1" << std::endl;
        // std::throw runtime_error("Minimax_wrong");
    }

#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif

    return my_move.move;
}


//terminal用于判断当前棋局是否已经结束，-1表示平局，0表示黑子胜，1表示白子胜，2表示未结束
int terminal() {
    int win = winner();
    if (win != EMPTY) {
        return win;
    } //如果已经决出胜者了，返回胜者
    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] == EMPTY) {
                return 2;
            }
        }
    } //如果所有格子都满了，也返回empty（平局）
    return EMPTY;
}



MinimaxResult Minimax(int depth) {
    int alpha = -INF;
    int beta = INF;
    std::pair<int, int> res = {-1, -1};

    auto moves = generate_sorted_moves(turn <= 4 ? 225 : 30);

    //如果我的ai执黑子，我想让结果最大
    if (ai_side == 0) {
        int val = -INF;
        for (const auto &move: moves) {
            int i = move.first, j = move.second;
            board[i][j] = BLACK; //落子
            update_hash(i, j, BLACK);
            int move_value = min_value(alpha, beta, depth - 1); //调用min
            board[i][j] = EMPTY; //撤销
            update_hash(i, j, BLACK);
            if (move_value > val) {
                val = move_value;
                res = {i, j};
                alpha = std::max(alpha, val);
            }
            if (alpha >= beta) {
                break; // Beta剪枝
            }
            // if (alpha >= 10000000) {
            //     break; //必胜剪枝
            // }

        }
        return {res, val};
    } else {
        //如果我的ai执白子，我想让结果最小
        int val = INF;
        for (const auto &move: moves) {
            int i = move.first, j = move.second;
            board[i][j] = WHITE; //落子
            update_hash(i, j, WHITE);
            int move_value = max_value(alpha, beta, depth - 1); //调用min
            board[i][j] = EMPTY; //撤销

            update_hash(i, j, WHITE);
            if (move_value < val) {
                val = move_value;
                res = {i, j}; 
                beta = std::min(beta, val);
            }
            if (alpha >= beta) {
                break; // Alpha剪枝
            }
            // if (alpha <= -10000000) {
            //     break; //必胜剪枝
            // }

        }
        return {res, val};
    }
}

int min_value(int alpha, int beta, int depth) {
    int ter = terminal();
    if (depth == 0 || ter != 2) {
        return evaluate(BLACK); //评估当前棋局的分数，当前该白棋落子
    }

    auto it = transposition_table.find(current_hash);
    if (it != transposition_table.end() && it->second.depth >= depth) {
        TTEntry entry = it->second;
        if (entry.flag == EXACT) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
        if (entry.flag == LOWER_BOUND)
            alpha = std::max(alpha, entry.score);
        else if (entry.flag == UPPER_BOUND)
            beta = std::min(beta, entry.score);
        if (alpha >= beta) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
    }


    int val = INF;
    int original_beta = beta;
    auto moves = generate_sorted_moves(turn <= 4 ? 225 : 30);
    for (const auto &move: moves) {
        int i = move.first, j = move.second;
        board[i][j] = WHITE; //落子
        update_hash(i, j, WHITE);
        val = std::min(val, max_value(alpha, beta, depth - 1)); //调用max
        board[i][j] = EMPTY; //撤销
        update_hash(i, j, WHITE);
        if (val <= alpha) {
            return val;
        } //alpha-beta剪枝
        beta = std::min(beta, val);
    }

    // --- Zobrist 存储 ---
    TTEntry entry;
    entry.score = val;
    entry.depth = depth;
    if (val >= original_beta) {
        entry.flag = LOWER_BOUND;
    } else if (val <= alpha) {
        entry.flag = UPPER_BOUND;
    } else {
        entry.flag = EXACT;
    }
    transposition_table[current_hash] = entry;
    // --- Zobrist 存储结束 ---


    return val;
}

int max_value(int alpha, int beta, int depth) {
    int ter = terminal();
    if (depth == 0 || ter != 2) {
        return evaluate(BLACK); //评估当前棋局的分数，当前该黑棋落子
    }

    // --- Zobrist 查询 ---
    auto it = transposition_table.find(current_hash);
    if (it != transposition_table.end() && it->second.depth >= depth) {
        TTEntry entry = it->second;
        if (entry.flag == EXACT) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
        if (entry.flag == LOWER_BOUND)
            alpha = std::max(alpha, entry.score);
        else if (entry.flag == UPPER_BOUND)
            beta = std::min(beta, entry.score);
        if (alpha >= beta) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
    }
    // --- Zobrist 查询结束 ---


    int val = -INF;
    int original_alpha = alpha;
    auto moves = generate_sorted_moves(turn <= 4 ? 225 : 30);
    for (const auto &move: moves) {
        int i = move.first, j = move.second;
        board[i][j] = BLACK; //落子
        update_hash(i, j, BLACK);
        val = std::max(val, min_value(alpha, beta, depth - 1)); //调用max
        board[i][j] = EMPTY; //撤销
        update_hash(i, j, BLACK);
        if (val >= beta) {
            return val;
        } //alpha-beta剪枝
        alpha = std::max(alpha, val);
    }


    // --- Zobrist 存储 ---
    TTEntry entry;
    entry.score = val;
    entry.depth = depth;
    if (val <= original_alpha) {
        entry.flag = UPPER_BOUND;
    } else if (val >= beta) {
        entry.flag = LOWER_BOUND;
    } else {
        entry.flag = EXACT;
    }
    transposition_table[current_hash] = entry;
    // --- Zobrist 存储结束 ---

    return val;
}


