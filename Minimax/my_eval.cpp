#include "my_eval.h"
#include <vector>
#include <map>
#include <numeric>

const int SIZE = 15;
extern int board[15][15]; 

// 新增一个全局变量，用于实时存储当前棋局的评估分数
long long current_total_score = 0;

enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

//棋型分数定义
const int CHENG_5_SCORE = 5000000;
const int HUO_4_SCORE = 100000;
const int CHONG_4_SCORE = 10000;
const int DAN_HUO_3_SCORE = 8000;   // "单活三"
const int TIAO_HUO_3_SCORE = 7000;  // "跳活三"
const int MIAN_3_SCORE = 500;       // "眠三"
const int HUO_2_SCORE = 50;         // "活二"
const int MIAN_2_SCORE = 10;        // "眠二"

//棋型字符串
const std::string CHENG_5_STRING = "11111";
const std::string HUO_4_STRING = "011110";
const std::vector<std::string> CHONG_4_STRINGS = { "01111-", "-11110", "10111", "11101", "11011" };
const std::string DAN_HUO_3_STRING = "01110";
const std::vector<std::string> TIAO_HUO_3_STRINGS = { "010110", "011010" }; // 跳活三两边也需为空
const std::vector<std::string> MIAN_3_STRINGS = {
    "00111-", "-11100", "01011-", "-11010", "01101-", "-10110",
    "10011", "11001", "10101", "-01110-"
};
const std::vector<std::string> HUO_2_STRINGS = { "001100", "01010", "010010" };
const std::vector<std::string> MIAN_2_STRINGS = {
    "00011-", "-11000", "00101-", "-10100", "01001-", "-10010", "10001"
};


/**
* 评估一条线（字符串表示）的分数
*/
int score_line(const std::string& line_str) {
    int score = 0;
    // 必须使用 if, 而不是 else if, 因为一条线可能包含多种棋型组合
    if (line_str.find(CHENG_5_STRING) != std::string::npos) {
        score += CHENG_5_SCORE;
    }
    if (line_str.find(HUO_4_STRING) != std::string::npos) {
        score += HUO_4_SCORE;
    }
    for (const auto& pattern : CHONG_4_STRINGS) {
        if (line_str.find(pattern) != std::string::npos) {
            score += CHONG_4_SCORE;
        }
    }
    if (line_str.find(DAN_HUO_3_STRING) != std::string::npos) {
        score += DAN_HUO_3_SCORE;
    }
    for (const auto& pattern : TIAO_HUO_3_STRINGS) {
        if (line_str.find(pattern) != std::string::npos) {
            score += TIAO_HUO_3_SCORE;
        }
    }
    for (const auto& pattern : MIAN_3_STRINGS) {
        if (line_str.find(pattern) != std::string::npos) {
            score += MIAN_3_SCORE;
        }
    }
    for (const auto& pattern : HUO_2_STRINGS) {
        if (line_str.find(pattern) != std::string::npos) {
            score += HUO_2_SCORE;
        }
    }
    for (const auto& pattern : MIAN_2_STRINGS) {
        if (line_str.find(pattern) != std::string::npos) {
            score += MIAN_2_SCORE;
        }
    }
    return score;
}


int calculate_score(int color) {
    int total_score = 0;
    int opponent_color = (color == BLACK) ? WHITE : BLACK;

    // 1. 检查所有行
    for (int i = 0; i < SIZE; ++i) {
        std::string line_str;
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == color) line_str += '1';
            else if (board[i][j] == EMPTY) line_str += '0';
            else line_str += '-';
        }
        total_score += score_line(line_str);
    }

    // 2. 检查所有列
    for (int j = 0; j < SIZE; ++j) {
        std::string line_str;
        for (int i = 0; i < SIZE; ++i) {
            if (board[i][j] == color) line_str += '1';
            else if (board[i][j] == EMPTY) line_str += '0';
            else line_str += '-';
        }
        total_score += score_line(line_str);
    }

    // 3. 检查所有对角线 (左上到右下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        std::string line_str;
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::max(0, (SIZE - 1) - k);
        for (int r = r_start, c = c_start; r < SIZE && c < SIZE; ++r, ++c) {
            if (board[r][c] == color) line_str += '1';
            else if (board[r][c] == EMPTY) line_str += '0';
            else line_str += '-';
        }
        if (line_str.length() >= 5) {
            total_score += score_line(line_str);
        }
    }

    // 4. 检查所有反对角线 (右上到左下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        std::string line_str;
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::min(k, SIZE - 1);
        for (int r = r_start, c = c_start; r < SIZE && c >= 0; ++r, --c) {
            if (board[r][c] == color) line_str += '1';
            else if (board[r][c] == EMPTY) line_str += '0';
            else line_str += '-';
        }
        if (line_str.length() >= 5) {
            total_score += score_line(line_str);
        }
    }

    return total_score;
}

// 完全重新计算棋盘分数，用于初始化和交换后
void recalculate_full_board_score() {
    int black_score = calculate_score(BLACK);
    int white_score = calculate_score(WHITE);
    current_total_score = black_score - white_score;
}

// 局部更新评估分数
int update_score_pos_color(int i, int j , int color) {
    int total_score = 0;

    // 1. 横线
    std::string line_str_1;
    for (int c = 0; c < SIZE; ++c) {
        if (board[i][c] == color) line_str_1 += '1';
        else if (board[i][c] == EMPTY) line_str_1 += '0';
        else line_str_1 += '-';
    }
    total_score += score_line(line_str_1);

    // 2. 竖线
    std::string line_str_2;
    for (int r = 0; r < SIZE; ++r) {


        if (board[r][j] == color) line_str_2 += '1';
        else if (board[r][j] == EMPTY) line_str_2 += '0';
        else line_str_2 += '-';


    }
    total_score += score_line(line_str_2);

    // 3. 主对角线 '\'
    int r_start = i - std::min(i, j);
    int c_start = j - std::min(i, j);
    std::string line_str_3;
    for (int r = r_start, c = c_start; r < SIZE && c < SIZE; ++r, ++c) {
        if (board[r][c] == color) line_str_3 += '1';
        else if (board[r][c] == EMPTY) line_str_3 += '0';
        else line_str_3 += '-';
    }
    if (line_str_3.length() >= 5) {
        total_score += score_line(line_str_3);
    }


    // 4. 副对角线 '/'
    std::string line_str_4;
    r_start = i - std::min(i, SIZE - 1 - j);
    c_start = j + std::min(i, SIZE - 1 - j);
    for (int r = r_start, c = c_start; r < SIZE && c >= 0; ++r, --c) {
        if (board[r][c] == color) line_str_4 += '1';
        else if (board[r][c] == EMPTY) line_str_4 += '0';
        else line_str_4 += '-';
    }
    if (line_str_4.length() >= 5) {
        total_score += score_line(line_str_4);
    }
    return total_score;
}

int update_score_for_position(int r, int c){
    int black_score = update_score_pos_color(r , c , BLACK);
    int white_score = update_score_pos_color(r , c,  WHITE);
    return black_score - white_score;
}



int eval() {
    int black_score = calculate_score(BLACK);
    int white_score = calculate_score(WHITE);

    // 如果一方已经连五，直接返回极值表示胜负
    if (black_score >= CHENG_5_SCORE) return 10000000; // 黑棋胜利
    if (white_score >= CHENG_5_SCORE) return -10000000; // 白棋胜利

    return black_score - white_score;
}  



/**
* 评估函数的主接口
* ter:棋局的终止状态。-1: 平局, 0: 黑胜, 1: 白胜, 2: 未结束。
* 返回对当前棋局的评分。
*/
int evaluate(int ter) {
    if (ter == EMPTY) { // 平局
                       return 0;
                      }

    #ifdef mizi
    return current_total_score;
    #endif
    int score = eval();
    // 进行全盘局面评估
    return score;


}


int winner(){
    for(int i = 0 ; i <= 10 ; i++){
        for(int j = 0 ; j <= 14 ; j++){
            if(board[i][j] == BLACK && board[i + 1][j] == BLACK && board[i + 2][j] == BLACK && board[i + 3][j] == BLACK && board[i + 4][j] == BLACK){
                return BLACK;
            }
            if(board[i][j] == WHITE && board[i + 1][j] == WHITE && board[i + 2][j] == WHITE && board[i + 3][j] == WHITE && board[i + 4][j] == WHITE){
                return WHITE;
            }
        }
    }
    for(int i = 0 ; i <= 14 ; i++){
        for(int j = 0 ; j <= 10 ; j++){
            if(board[i][j] == BLACK && board[i][j + 1] == BLACK && board[i][j + 2] == BLACK && board[i][j + 3] == BLACK && board[i][j + 4] == BLACK){
                return BLACK;
            }
            if(board[i][j] == WHITE && board[i][j + 1] == WHITE && board[i][j + 2] == WHITE && board[i][j + 3] == WHITE && board[i][j + 4] == WHITE){
                return WHITE;
            }
        }
    }
    for(int i = 0 ; i <= 10 ; i++){
        for(int j = 0 ; j <= 10 ; j++){
            if(board[i][j] == BLACK && board[i + 1][j + 1] == BLACK && board[i + 2][j + 2] == BLACK && board[i + 3][j + 3] == BLACK && board[i + 4][j + 4] == BLACK){
                return BLACK;
            }
            if(board[i][j] == WHITE && board[i + 1][j + 1] == WHITE && board[i + 2][j + 2] == WHITE && board[i + 3][j + 3] == WHITE && board[i + 4][j + 4] == WHITE){
                return WHITE;
            }
        }
    }
    for(int i = 0 ; i <= 10 ; i++){
        for(int j = 4 ; j <= 14 ; j++){
            if(board[i][j] == BLACK && board[i + 1][j - 1] == BLACK && board[i + 2][j - 2] == BLACK && board[i + 3][j - 3] == BLACK && board[i + 4][j - 4] == BLACK){
                return BLACK;
            }
            if(board[i][j] == WHITE && board[i + 1][j - 1] == WHITE && board[i + 2][j - 2] == WHITE && board[i + 3][j - 3] == WHITE && board[i + 4][j - 4] == WHITE){
                return WHITE;
            }
        }
    }
    return EMPTY;
}

//用于实现启发式评估函数
int score_move(int r, int c, int color) {
    board[r][c] = color; // 临时落子
    int score = update_score_pos_color(r, c, color);
    board[r][c] = EMPTY; // 撤销落子
    return score;
}