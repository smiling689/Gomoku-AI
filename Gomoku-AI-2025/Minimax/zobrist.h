#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>
#include <unordered_map>
#include <utility>

extern int board[15][15];

extern std::pair<int , int> Minimax();

// 分数类型，用于置换表
// EXACT: 精确的评估分数 (alpha < score < beta)
// LOWER_BOUND: 分数是一个下界 (搜索时发生了beta剪枝，实际分数 >= score)
// UPPER_BOUND: 分数是一个上界 (搜索时未能提高alpha，实际分数 <= score)
enum TTFlag { EXACT, LOWER_BOUND, UPPER_BOUND };

// 置换表结构
struct TTEntry {
    int depth;      // 当前结果的搜索深度
    int score;      // 评估分数
    TTFlag flag;    // 分数类型
    // std::pair<int, int> best_move; // 可以存储最佳移动以用于移动排序优化
};

// Zobrist键 [行][列][颜色] (0: 黑, 1: 白)
extern uint64_t zobrist_keys[15][15][2];
// 当前棋局的哈希值
extern uint64_t current_hash;

// 置换表
extern std::unordered_map<uint64_t, TTEntry> transposition_table;

// 初始化
void init_zobrist();

// 全量计算哈希值
uint64_t calculate_hash();

// 增量更新哈希值
void update_hash(int r, int c, int color);

#endif // ZOBRIST_H