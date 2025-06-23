#include "my_eval_new.h"
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
* 评估一条线（由整数向量表示）的分数。
*/
int score_line_array(const std::vector<int>& line, int color) {
    int score = 0;
    const int opponent_color = 1 - color;
    const int line_len = line.size();

    // 辅助函数，用于检查一个位置是否为对手棋子或边界外（墙）
    auto is_opponent_or_wall = [&](int index) {
        return index < 0 || index >= line_len || line[index] == opponent_color;
    };
    
    // 连五: "11111"
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == color) {
            score += CHENG_5_SCORE;
            break;
        }
    }

    // 活四: "011110"
    for (int i = 0; i <= line_len - 6; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == color && line[i+5] == EMPTY) {
            score += HUO_4_SCORE;
            break;
        }
    }

    // 冲四: { "01111-", "-11110", "10111", "11101", "11011" }
    // 冲四模式1: "01111-"
    bool p_chong4_1 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_chong4_1 = true; break;
        }
    }
    if (p_chong4_1) score += CHONG_4_SCORE;

    // 冲四模式2: "-11110"
    bool p_chong4_2 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i - 1) && line[i] == color && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == EMPTY) {
            p_chong4_2 = true; break;
        }
    }
    if (p_chong4_2) score += CHONG_4_SCORE;

    // 冲四模式3: "10111"
    bool p_chong4_3 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == color && line[i+4] == color) {
            p_chong4_3 = true; break;
        }
    }
    if (p_chong4_3) score += CHONG_4_SCORE;
    
    // 冲四模式4: "11101"
    bool p_chong4_4 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == color && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == color) {
            p_chong4_4 = true; break;
        }
    }
    if (p_chong4_4) score += CHONG_4_SCORE;

    // 冲四模式5: "11011"
    bool p_chong4_5 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == color) {
            p_chong4_5 = true; break;
        }
    }
    if (p_chong4_5) score += CHONG_4_SCORE;

    // 单活三: "01110"
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == EMPTY) {
            score += DAN_HUO_3_SCORE;
            break;
        }
    }

    // 跳活三: { "010110", "011010" }
    // 跳活三模式1: "010110"
    bool p_tiao3_1 = false;
    for (int i = 0; i <= line_len - 6; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == color && line[i+5] == EMPTY) {
            p_tiao3_1 = true; break;
        }
    }
    if (p_tiao3_1) score += TIAO_HUO_3_SCORE;
    
    // 跳活三模式2: "011010"
    bool p_tiao3_2 = false;
    for (int i = 0; i <= line_len - 6; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == color && line[i+5] == EMPTY) {
            p_tiao3_2 = true; break;
        }
    }
    if (p_tiao3_2) score += TIAO_HUO_3_SCORE;

    // 眠三: { "00111-", "-11100", "01011-", "-11010", "01101-", "-10110", "10011", "11001", "10101", "-01110-" }
    // 眠三模式1: "00111-"
    bool p_mian3_1 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == color && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian3_1 = true; break;
        }
    }
    if(p_mian3_1) score += MIAN_3_SCORE;

    // 眠三模式2: "-11100"
    bool p_mian3_2 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i] == color && line[i+1] == color && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == EMPTY) {
            p_mian3_2 = true; break;
        }
    }
    if(p_mian3_2) score += MIAN_3_SCORE;

    // 眠三模式3: "01011-"
    bool p_mian3_3 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian3_3 = true; break;
        }
    }
    if(p_mian3_3) score += MIAN_3_SCORE;

    // 眠三模式4: , "-11010", 
    bool p_mian3_4 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i - 1) && line[i] == color && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == color && line[i + 4] == EMPTY) {
            p_mian3_4 = true; break;
        }
    }
    if(p_mian3_4) score += MIAN_3_SCORE;

    // 眠三模式5: "01101-",
    bool p_mian3_5 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian3_5 = true; break;
        }
    }
    if(p_mian3_5) score += MIAN_3_SCORE;

    // 眠三模式6:  "-10110",
    bool p_mian3_6 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i ] == color && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == color && line[i+4] == EMPTY) {
            p_mian3_6 = true; break;
        }
    }
    if(p_mian3_6) score += MIAN_3_SCORE;

    // 眠三模式7:  "10011", 
    bool p_mian3_7 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == EMPTY && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == color) {
            p_mian3_7 = true; break;
        }
    }
    if(p_mian3_7) score += MIAN_3_SCORE;

    // 眠三模式8: "11001", 
    bool p_mian3_8 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == color && line[i+2] == EMPTY && line[i+3] ==EMPTY && line[i+4] == color) {
            p_mian3_8 = true; break;
        }
    }
    if(p_mian3_8) score += MIAN_3_SCORE;

    // 眠三模式9: "10101",
    bool p_mian3_9 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == color) {
            p_mian3_9 = true; break;
        }
    }
    if(p_mian3_9) score += MIAN_3_SCORE;

    // 眠三模式10:  "-01110-"
    bool p_mian3_10 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i] == EMPTY && line[i+1] == color && line[i+2] == color && line[i+3] == color && line[i+4] == EMPTY && is_opponent_or_wall(i + 5)) {
            p_mian3_10 = true; break;
        }
    }
    if(p_mian3_10) score += MIAN_3_SCORE;
    
    // 活二: { "001100", "01010", "010010" }
    // 活二模式1: "001100"
    bool p_huo2_1 = false;
    for (int i = 0; i <= line_len - 6; ++i) {
        if (line[i] == EMPTY && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == color && line[i+4] == EMPTY && line[i+5] == EMPTY) {
            p_huo2_1 = true; break;
        }
    }
    if(p_huo2_1) score += HUO_2_SCORE;

    // 活二模式2: "01010"
    bool p_huo2_2 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == EMPTY) {
            p_huo2_2 = true; break;
        }
    }
    if(p_huo2_2) score += HUO_2_SCORE;

    // 活二模式3: "010010"
    bool p_huo2_3 = false;
    for (int i = 0; i <= line_len - 6; ++i) {
        if (line[i] == EMPTY && line[i+1] ==color && line[i+2] == EMPTY && line[i+3] == EMPTY && line[i+4] == color && line[i+5] == EMPTY) {
            p_huo2_3 = true; break;
        }
    }
    if(p_huo2_3) score += HUO_2_SCORE;

    // 眠二: { "00011-", "-11000", "00101-", "-10100", "01001-", "-10010", "10001" }
    // 眠二模式1: "00011-"
    bool p_mian2_1 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == EMPTY && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian2_1 = true; break;
        }
    }
    if(p_mian2_1) score += MIAN_2_SCORE;
    
    // 眠二模式2: "-11000"
    bool p_mian2_2 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i] == color && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == EMPTY && line[i+4] == EMPTY) {
            p_mian2_2 = true; break;
        }
    }
    if(p_mian2_2) score += MIAN_2_SCORE;
    
    // 眠二模式3: "00101-"
    bool p_mian2_3 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == EMPTY && line[i+2] == color && line[i+3] ==EMPTY && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian2_3 = true; break;
        }
    }
    if(p_mian2_3) score += MIAN_2_SCORE;
 
    // 眠二模式4: "-10100"
    bool p_mian2_4 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i] == color && line[i+1] == EMPTY && line[i+2] == color && line[i+3] == EMPTY && line[i+4] == EMPTY) {
            p_mian2_4 = true; break;
        }
    }
    if(p_mian2_4) score += MIAN_2_SCORE;
 
    // 眠二模式5: "01001-"
    bool p_mian2_5 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == EMPTY && line[i+1] == color && line[i+2] == EMPTY && line[i+3] == EMPTY && line[i+4] == color && is_opponent_or_wall(i + 5)) {
            p_mian2_5 = true; break;
        }
    }
    if(p_mian2_5) score += MIAN_2_SCORE;
 
    // 眠二模式6: "-10010"
    bool p_mian2_6 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (is_opponent_or_wall(i-1) && line[i] == color && line[i+1] == EMPTY && line[i+2] == EMPTY && line[i+3] == color && line[i+4] == EMPTY) {
            p_mian2_6 = true; break;
        }
    }
    if(p_mian2_6) score += MIAN_2_SCORE;

    // 眠二模式7: "10001"
    bool p_mian2_7 = false;
    for (int i = 0; i <= line_len - 5; ++i) {
        if (line[i] == color && line[i+1] == EMPTY && line[i+2] == EMPTY && line[i+3] == EMPTY && line[i+4] == color) {
            p_mian2_7 = true; break;
        }
    }
    if(p_mian2_7) score += MIAN_2_SCORE;
 
    return score;
}


int calculate_score(int color) {
    int total_score = 0;
    std::vector<int> line;

    // 1. 检查所有行
    for (int i = 0; i < SIZE; ++i) {
        line.assign(board[i], board[i] + SIZE);
        total_score += score_line_array(line, color);
    }

    // 2. 检查所有列
    for (int j = 0; j < SIZE; ++j) {
        line.clear();
        for (int i = 0; i < SIZE; ++i) {
            line.push_back(board[i][j]);
        }
        total_score += score_line_array(line, color);
    }

    // 3. 检查所有对角线 (左上到右下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        line.clear();
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::max(0, (SIZE - 1) - k);
        for (int r = r_start, c = c_start; r < SIZE && c < SIZE; ++r, ++c) {
            line.push_back(board[r][c]);
        }
        if (line.size() >= 5) {
            total_score += score_line_array(line, color);
        }
    }

    // 4. 检查所有反对角线 (右上到左下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        line.clear();
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::min(k, SIZE - 1);
        for (int r = r_start, c = c_start; r < SIZE && c >= 0; ++r, --c) {
            line.push_back(board[r][c]);
        }
        if (line.size() >= 5) {
            total_score += score_line_array(line, color);
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
int update_score_pos_color(int r, int c , int color) {
    int total_score = 0;
    std::vector<int> line;

    // 1. 横线
    line.assign(board[r], board[r] + SIZE);
    total_score += score_line_array(line, color);

    // 2. 竖线
    line.clear();
    for (int i = 0; i < SIZE; ++i) {
        line.push_back(board[i][c]);
    }
    total_score += score_line_array(line, color);

    // 3. 主对角线 '\'
    line.clear();
    for (int i = -std::min(r, c), end = std::min(SIZE - 1 - r, SIZE - 1 - c); i <= end; ++i) {
        line.push_back(board[r + i][c + i]);
    }
    if (line.size() >= 5) {
        total_score += score_line_array(line, color);
    }

    // 4. 副对角线 '/'
    line.clear();
    for (int i = -std::min(r, SIZE - 1 - c), end = std::min(SIZE - 1 - r, c); i <= end; ++i) {
        line.push_back(board[r + i][c - i]);
    }
     if (line.size() >= 5) {
        total_score += score_line_array(line, color);
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