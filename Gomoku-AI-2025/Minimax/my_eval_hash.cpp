#include "my_eval_hash.h"
#include <vector>
#include <iostream>
#include <map>
#include <numeric>
#include <unordered_map>

const int SIZE = 15;
extern int board[15][15]; 
const uint64_t MOD = 1e9 + 7; 

// 新增一个全局变量，用于实时存储当前棋局的评估分数
long long current_total_score = 0;

enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1,
    WALL = 2
};

// --- 定义棋型内部使用的常量（相对表示）---
enum PatternToken { 
    P_ME = 1, 
    P_BLOCKER = 2, 
    P_EMPTY = 3
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

//棋型字符串，1表示己方，0表示空，-表示对方或者墙壁
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

const std::vector<std::pair<const std::vector<std::string>, int>> ALL_PATTERNS = {
    {{"11111"}, CHENG_5_SCORE},
    {{"011110"}, HUO_4_SCORE},
    {{"01111-", "-11110", "10111", "11101", "11011"}, CHONG_4_SCORE},
    {{"01110"}, DAN_HUO_3_SCORE},
    {{"010110", "011010"}, TIAO_HUO_3_SCORE},
    {{"00111-", "-11100", "01011-", "-11010", "01101-", "-10110", "10011", "11001", "10101"}, MIAN_3_SCORE},
    {{"001100", "01010", "010010"}, HUO_2_SCORE},
    {{"00011-", "-11000", "00101-", "-10100", "01001-", "-10010", "10001"}, MIAN_2_SCORE}
};

const uint64_t HASH_BASE = 31; // 使用一个小的素数作为基数 

uint64_t HASH_POWERS[20];

void precompute_hash_powers() {
    HASH_POWERS[0] = 1;
    for (int i = 1; i <= 19; ++i) {
        HASH_POWERS[i] = (HASH_POWERS[i-1] * HASH_BASE) % MOD;
    }
}


std::unordered_map<uint64_t, int> universal_pattern_scores; // 通用哈希数据库

std::unordered_map<uint64_t, int> black_line_cache;
std::unordered_map<uint64_t, int> white_line_cache;

void init_pattern_db() {
    precompute_hash_powers() ;
    universal_pattern_scores.clear();
    for (const auto& group : ALL_PATTERNS) {
        int score = group.second;
        for (const std::string& pattern_str : group.first) {
            std::vector<int> relative_pattern;
            for (char c : pattern_str) {
                if (c == '1') relative_pattern.push_back(P_ME);
                else if (c == '0') relative_pattern.push_back(P_EMPTY);
                else if (c == '-') relative_pattern.push_back(P_BLOCKER);
            }

            uint64_t hash_val = 0;
            for (int token : relative_pattern) {
                hash_val = hash_val * HASH_BASE + token;
                hash_val %= MOD;
            }
            universal_pattern_scores[hash_val] = score;
        }
    }
}

int score_line_hashed(const std::vector<int>& line_with_padding, int color) {
    int total_score = 0;
    const int len = line_with_padding.size();
    
    // 1. 将绝对棋盘表示转换为相对表示
    std::vector<int> relative_line(len);
    const int opponent_color = 1 - color;
    for (int i = 0; i < len; ++i) {
    const int cell = line_with_padding[i];
    if (cell == color) relative_line[i] = P_ME;
    else if (cell == opponent_color || cell == WALL) { 
        relative_line[i] = P_BLOCKER;
    } else if (cell == EMPTY) {
        relative_line[i] = P_EMPTY;
    }
}

    // 2. 计算滚动哈希
    const int max_pattern_len = 7;
    uint64_t power[max_pattern_len + 1];
    power[0] = 1;
    for (int i = 1; i <= max_pattern_len; ++i) {
        power[i] = power[i-1] * HASH_BASE;
        power[i] %= MOD;
    }
    
    for (int win_len = 4; win_len <= max_pattern_len; ++win_len) {
        if (len < win_len) continue;

        uint64_t current_hash = 0;
        for (int i = 0; i < win_len; ++i) {
            current_hash = current_hash * HASH_BASE + relative_line[i];
            current_hash %= MOD;
        }
        uint64_t power_win_len_minus_1 = HASH_POWERS[win_len - 1];

        for (int i = 0; i <= len - win_len; ++i) {
            if (i > 0) {
                uint64_t old_val = relative_line[i-1];
                uint64_t new_val = relative_line[i + win_len - 1];
                // 1. 减去旧值的影响
                uint64_t term_to_subtract = (old_val * power_win_len_minus_1) % MOD;
                current_hash = (current_hash - term_to_subtract + MOD) % MOD; // +MOD 防止负数
                // 2. "左移"一位并加上新值
                current_hash = (current_hash * HASH_BASE + new_val) % MOD;
            }

            // 3. 查表计分
            if (universal_pattern_scores.count(current_hash)) {
                total_score += universal_pattern_scores.at(current_hash);
            }
        }
    }
    return total_score;
}

// 新增的包装函数，实现了行分数的记忆化
int get_line_score(const std::vector<int>& line_with_padding, int color) {
    // 1. 计算整条带墙壁的行的哈希值，作为缓存的key
    uint64_t line_hash = 0;
    for (int cell : line_with_padding) {
        line_hash = line_hash * HASH_BASE + (cell + 2); 
    }

    // 2. 选择正确的缓存库并查找
    auto& cache = (color == BLACK) ? black_line_cache : white_line_cache;
    if (cache.count(line_hash)) {
        // 缓存命中，直接返回结果
        return cache.at(line_hash);
    }

    // 3. 缓存未命中，调用之前的函数进行实际计算
    int score = score_line_hashed(line_with_padding, color);

    // 4. 将新计算出的分数存入缓存，以便下次使用
    cache[line_hash] = score;

    return score;
}

int calculate_score(int color) {
    int total_score = 0;
    std::vector<int> line;

    // 1. 检查所有行
    for (int i = 0; i < SIZE; ++i) {
        line.clear();
        line.push_back(WALL);//墙壁
        for (int j = 0; j < SIZE; ++j) {
            line.push_back(board[i][j]);
        }
        line.push_back(WALL);
        total_score += get_line_score(line, color);
    }

    // 2. 检查所有列
    for (int j = 0; j < SIZE; ++j) {
        line.clear();
        line.push_back(WALL);
        for (int i = 0; i < SIZE; ++i) {
            line.push_back(board[i][j]);
        }
        line.push_back(WALL);
        total_score += get_line_score(line, color);
    }

    // 3. 检查所有对角线 (左上到右下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        line.clear();
        line.push_back(WALL);
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::max(0, (SIZE - 1) - k);
        for (int r = r_start, c = c_start; r < SIZE && c < SIZE; ++r, ++c) {
            line.push_back(board[r][c]);
        }
        line.push_back(WALL);
        if (line.size() >= 5) {
            total_score += get_line_score(line, color);
        }
    }

    // 4. 检查所有反对角线 (右上到左下)
    for (int k = 0; k < 2 * SIZE - 1; ++k) {
        line.clear();
        line.push_back(WALL);
        int r_start = std::max(0, k - (SIZE - 1));
        int c_start = std::min(k, SIZE - 1);
        for (int r = r_start, c = c_start; r < SIZE && c >= 0; ++r, --c) {
            line.push_back(board[r][c]);
        }
        line.push_back(WALL);
        if (line.size() >= 5) {
            total_score += get_line_score(line, color);
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
    line.push_back(WALL);
    for (int j = 0; j < SIZE; ++j) {
            line.push_back(board[r][j]);
        }
    line.push_back(WALL);
    total_score += get_line_score(line, color);

    // 2. 竖线
    line.clear();
    line.push_back(WALL);
    for (int i = 0; i < SIZE; ++i) {
        line.push_back(board[i][c]);
    }
    line.push_back(WALL);
    total_score += get_line_score(line, color);

    // 3. 主对角线 '\'
    line.clear();
    line.push_back(WALL);
    for (int i = -std::min(r, c), end = std::min(SIZE - 1 - r, SIZE - 1 - c); i <= end; ++i) {
        line.push_back(board[r + i][c + i]);
    }
    line.push_back(WALL);
    if (line.size() >= 5) {
        total_score += get_line_score(line, color);
    }

    // 4. 副对角线 '/'
    line.clear();
    line.push_back(WALL);
    for (int i = -std::min(r, SIZE - 1 - c), end = std::min(SIZE - 1 - r, c); i <= end; ++i) {
        line.push_back(board[r + i][c - i]);
    }
    line.push_back(WALL);
     if (line.size() >= 5) {
        total_score += get_line_score(line, color);
    }
    return total_score;
}

int update_score_for_position(int r, int c){
    int black_score = update_score_pos_color(r , c , BLACK);
    int white_score = update_score_pos_color(r , c,  WHITE);
    return black_score - white_score ;
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