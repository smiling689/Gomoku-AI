#include "zobrist.h"
#include <random>

// 外部变量定义
extern int board[15][15];

// 全局变量实现
uint64_t zobrist_keys[15][15][2];
uint64_t current_hash = 0;
std::unordered_map<uint64_t, TTEntry> transposition_table;


void init_zobrist() {
    // 固定种子
    std::mt19937_64 gen(20240622); 
    std::uniform_int_distribution<uint64_t> dist;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            zobrist_keys[i][j][0] = dist(gen); // 黑棋的键
            zobrist_keys[i][j][1] = dist(gen); // 白棋的键
        }
    }
    // 清空置换表和当前哈希值
    transposition_table.clear();
    current_hash = 0;
}

uint64_t calculate_hash() {
    uint64_t h = 0;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (board[i][j] != -1) { // EMPTY = -1
                h ^= zobrist_keys[i][j][board[i][j]];
            }
        }
    }
    return h;
}

void update_hash(int r, int c, int color) {
    current_hash ^= zobrist_keys[r][c][color];
}