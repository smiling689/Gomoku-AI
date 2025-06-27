#include "new_eval.h"
#include <algorithm>
#include <cstring>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

extern int ai_side;

bool is_valid_pair(std::pair<int, int> x) {
    return x.first <= 14 && x.second <= 14 && x.first >= 0 && x.second >= 0;
}

struct Move {
    std::pair<int, int> pos;
    int score;
};

enum Cell { EMPTY = -1, BLACK = 0, WHITE = 1 };

std::vector<std::pair<int, int>> dir = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

namespace GomokuLegacyEval {

// 内部辅助数据和函数
namespace {
const int DIRS[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
bool is_valid(int r, int c) { return r >= 0 && r < 15 && c >= 0 && c < 15; }
} // namespace

// 前向声明
int score_slice(const int (&board)[15][15], int eval_color, int segment_color,
                int len, int start_r, int start_c, int dir_idx);
int evaluate_direction(const int (&board)[15][15], int eval_color, int dir_idx);

// ==================================================================================
// 主评估函数
// ==================================================================================
int evaluate(const int (&board)[15][15], int eval_color) {
    int total_score = 0;
    total_score += evaluate_direction(board, eval_color, 0); // 水平
    total_score += evaluate_direction(board, eval_color, 1); // 垂直
    total_score += evaluate_direction(board, eval_color, 2); // 左上到右下
    total_score += evaluate_direction(board, eval_color, 3); // 右上到左下
    return total_score;
}

int evaluate_direction(const int (&board)[15][15], int eval_color,
                       int dir_idx) {
    int total_score = 0;
    const int dr = DIRS[dir_idx][0];
    const int dc = DIRS[dir_idx][1];

    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (is_valid(i - dr, j - dc))
                continue;

            int r = i, c = j;
            while (is_valid(r, c)) {
                if (board[r][c] == -1) {
                    r += dr;
                    c += dc;
                    continue;
                }

                int segment_color = board[r][c];
                int segment_len = 0;
                int start_r = r, start_c = c;

                while (is_valid(r, c) && board[r][c] == segment_color) {
                    segment_len++;
                    r += dr;
                    c += dc;
                }

                total_score +=
                    score_slice(board, eval_color, segment_color, segment_len,
                                start_r, start_c, dir_idx);
            }
        }
    }
    return total_score;
}

// 切片评分函数
int score_slice(const int (&board)[15][15], int eval_color, int segment_color,
                int len, int start_r, int start_c, int dir_idx) {
    int total_score = 0;
    const int flag = (segment_color == eval_color) ? 1 : -1;
    const int dr = DIRS[dir_idx][0];
    const int dc = DIRS[dir_idx][1];

    // 从切片信息推导出两端点的坐标
    const int r_before = start_r - dr;
    const int c_before = start_c - dc;
    const int r_after = start_r + len * dr;
    const int c_after = start_c + len * dc;

    if (len >= 5) {
        total_score += Scores::FIVE * flag;
    } else {
        const bool live_start =
            is_valid(r_before, c_before) && board[r_before][c_before] == -1;
        const bool live_end =
            is_valid(r_after, c_after) && board[r_after][c_after] == -1;
        const int opponent_color = 1 - segment_color;

        if (live_start && live_end) {
            if (len == 4)
                total_score += Scores::LIVE_FOUR * flag;
            else if (len == 3)
                total_score += Scores::LIVE_THREE * flag;
            else if (len == 2)
                total_score += Scores::LIVE_TWO * flag;
            else if (len == 1)
                total_score += Scores::LIVE_ONE * flag;
        } else if (live_start || live_end) {
            // 只有当另一端不是对方棋子时才算死棋，否则是彻底堵死的无价值棋
            bool dead_by_opponent_at_start =
                is_valid(r_before, c_before) &&
                board[r_before][c_before] == opponent_color;
            bool dead_by_opponent_at_end =
                is_valid(r_after, c_after) &&
                board[r_after][c_after] == opponent_color;
            if (live_start && !dead_by_opponent_at_end)
                total_score +=
                    (len == 4 ? Scores::DEAD_FOUR
                              : len == 3 ? Scores::DEAD_THREE
                                         : len == 2 ? Scores::DEAD_TWO : 0) *
                    flag;
            if (live_end && !dead_by_opponent_at_start)
                total_score +=
                    (len == 4 ? Scores::DEAD_FOUR
                              : len == 3 ? Scores::DEAD_THREE
                                         : len == 2 ? Scores::DEAD_TWO : 0) *
                    flag;
        }
    }

    // 特判的中心是切片后的第一个空格 (r_after, c_after)
    if (is_valid(r_after, c_after) && board[r_after][c_after] == -1) {

        // 特判1:  O O _ ??
        if (len == 2) {
            // p 对应切片后的空格 (r_after, c_after)
            // r1, r2, r3 是空格后的点
            int r1_r = r_after + dr, r1_c = c_after + dc;
            int r2_r = r_after + 2 * dr, r2_c = c_after + 2 * dc;
            int r3_r = r_after + 3 * dr, r3_c = c_after + 3 * dc;
            // l3 是 O O _ 前面的点
            int l3_r = start_r - 2 * dr, l3_c = start_c - 2 * dc;

            if (is_valid(r1_r, r1_c) && board[r1_r][r1_c] == segment_color) {
                if (is_valid(r2_r, r2_c) &&
                    board[r2_r][r2_c] == segment_color) { // 找到了 O O _ O O
                    if (is_valid(r3_r, r3_c) &&
                        board[r3_r][r3_c] != 1 - segment_color &&
                        is_valid(l3_r, l3_c) &&
                        board[l3_r][l3_c] != 1 - segment_color) {
                        total_score += Scores::JUMP_LIVE_FOUR * flag;
                    } else if (!((!is_valid(r3_r, r3_c) ||
                                  board[r3_r][r3_c] == 1 - segment_color) &&
                                 (!is_valid(l3_r, l3_c) ||
                                  board[l3_r][l3_c] == 1 - segment_color))) {
                        total_score += Scores::JUMP_DEAD_FOUR * flag;
                    }
                }
            }
        }

        // 特判2:  O _ ??
        if (len == 1) {
            int r1_r = r_after + dr, r1_c = c_after + dc;
            int r2_r = r_after + 2 * dr, r2_c = c_after + 2 * dc;
            int r3_r = r_after + 3 * dr, r3_c = c_after + 3 * dc;
            int r4_r = r_after + 4 * dr, r4_c = c_after + 4 * dc;
            int l2_r = start_r - 2 * dr, l2_c = start_c - 2 * dc;

            if (is_valid(r1_r, r1_c) && board[r1_r][r1_c] == segment_color &&
                is_valid(r2_r, r2_c) && board[r2_r][r2_c] == segment_color) {
                if (is_valid(r3_r, r3_c) &&
                    board[r3_r][r3_c] == segment_color) { // 找到了 O _ O O O
                    if (is_valid(r4_r, r4_c) &&
                        board[r4_r][r4_c] != 1 - segment_color &&
                        is_valid(l2_r, l2_c) && board[l2_r][l2_c] == -1) {
                        total_score += Scores::JUMP_LIVE_FOUR * flag;
                    } else if (!((!is_valid(r4_r, r4_c) ||
                                  board[r4_r][r4_c] == 1 - segment_color) &&
                                 (!is_valid(l2_r, l2_c) ||
                                  board[l2_r][l2_c] == 1 - segment_color))) {
                        total_score += Scores::JUMP_DEAD_FOUR * flag;
                    }
                }
            }
        }
    }

    return total_score;
}

} // namespace GomokuLegacyEval

int score_move_color(int r, int c, int color);

int score_move(int r, int c) { //启发式，空格评分函数
    int total_score = 0;
    total_score += score_move_color(r, c, BLACK);
    total_score += score_move_color(r, c, WHITE);
    return total_score;
}

int score_move_color(int r, int c, int color) {
    int score = 0;
    for (int i = 0; i <= 3; i++) {
        int length = 1;
        int right_r = r + dir[i].first;
        int right_c = c + dir[i].second;
        int left_r = r - dir[i].first;
        int left_c = c - dir[i].second;
        int huo_r = 1, huo_l = 1;

        while (is_valid_pair({right_r, right_c}) &&
               board[right_r][right_c] == color) {
            length++;
            right_r += dir[i].first;
            right_c += dir[i].second;
        }

        while (is_valid_pair({left_r, left_c}) &&
               board[left_r][left_c] == color) {
            length++;
            left_r -= dir[i].first;
            left_c -= dir[i].second;
        }
        if (is_valid_pair({left_r, left_c}) && board[left_r][left_c] == EMPTY)
            huo_r = 0;
        if (is_valid_pair({right_r, right_c}) &&
            board[right_r][right_c] == EMPTY)
            huo_l = 0;

        if (length >= 5) {
            score += Point_Scores::FIVE;
        }

        if (huo_l == 0 && huo_r == 0) { // 两端都活
            switch (length) {
            case 4:
                score += Point_Scores::LIVE_FOUR;
                break; // 活四
            case 3:
                score += Point_Scores::LIVE_THREE;
                break; // 活三
            case 2:
                score += Point_Scores::LIVE_TWO;
                break; // 活二
            case 1:
                score += Point_Scores::LIVE_ONE;
                break; // 活一
            case 0:
                score += Point_Scores::ZERO;
                break; // 零
            default:
                if (length >= 5) {
                    score += Point_Scores::FIVE;
                }
                break;
            }
        } else if (huo_l == 0 || huo_r == 0) { // 只有一端活
            switch (length) {
            case 4:
                score += Point_Scores::DEAD_FOUR;
                break; // 死四
            case 3:
                score += Point_Scores::DEAD_THREE;
                break; // 死三
            case 2:
                score += Point_Scores::DEAD_TWO;
                break; // 死二
            default:
                if (length >= 5) {
                    score += Point_Scores::FIVE;
                }
                break;
            }
        }
    }
    return score;
}

bool terminate(int r, int c) {
    if (board[r][c] != ai_side)
        return false;
    int length = 1;
    for (int i = 0; i <= 3; i++) {
        int right_r = r + dir[i].first;
        int right_c = c + dir[i].second;
        int left_r = r - dir[i].first;
        int left_c = c - dir[i].second;

        while (is_valid_pair({right_r, right_c}) &&
               board[right_r][right_c] == ai_side) {
            length++;
            right_r += dir[i].first;
            right_c += dir[i].second;
        }

        while (is_valid_pair({left_r, left_c}) &&
               board[left_r][left_c] == ai_side) {
            length++;
            left_r -= dir[i].first;
            left_c -= dir[i].second;
        }
    }
    return length == 5;
}

int winner() {
    for (int i = 0; i <= 10; i++) {
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] == BLACK && board[i + 1][j] == BLACK &&
                board[i + 2][j] == BLACK && board[i + 3][j] == BLACK &&
                board[i + 4][j] == BLACK) {
                return BLACK;
            }
            if (board[i][j] == WHITE && board[i + 1][j] == WHITE &&
                board[i + 2][j] == WHITE && board[i + 3][j] == WHITE &&
                board[i + 4][j] == WHITE) {
                return WHITE;
            }
        }
    }
    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j <= 10; j++) {
            if (board[i][j] == BLACK && board[i][j + 1] == BLACK &&
                board[i][j + 2] == BLACK && board[i][j + 3] == BLACK &&
                board[i][j + 4] == BLACK) {
                return BLACK;
            }
            if (board[i][j] == WHITE && board[i][j + 1] == WHITE &&
                board[i][j + 2] == WHITE && board[i][j + 3] == WHITE &&
                board[i][j + 4] == WHITE) {
                return WHITE;
            }
        }
    }
    for (int i = 0; i <= 10; i++) {
        for (int j = 0; j <= 10; j++) {
            if (board[i][j] == BLACK && board[i + 1][j + 1] == BLACK &&
                board[i + 2][j + 2] == BLACK && board[i + 3][j + 3] == BLACK &&
                board[i + 4][j + 4] == BLACK) {
                return BLACK;
            }
            if (board[i][j] == WHITE && board[i + 1][j + 1] == WHITE &&
                board[i + 2][j + 2] == WHITE && board[i + 3][j + 3] == WHITE &&
                board[i + 4][j + 4] == WHITE) {
                return WHITE;
            }
        }
    }
    for (int i = 0; i <= 10; i++) {
        for (int j = 4; j <= 14; j++) {
            if (board[i][j] == BLACK && board[i + 1][j - 1] == BLACK &&
                board[i + 2][j - 2] == BLACK && board[i + 3][j - 3] == BLACK &&
                board[i + 4][j - 4] == BLACK) {
                return BLACK;
            }
            if (board[i][j] == WHITE && board[i + 1][j - 1] == WHITE &&
                board[i + 2][j - 2] == WHITE && board[i + 3][j - 3] == WHITE &&
                board[i + 4][j - 4] == WHITE) {
                return WHITE;
            }
        }
    }
    return EMPTY;
}

std::vector<std::pair<int, int>> generate_sorted_moves(int tot) {
    std::vector<Move> fives;
    std::vector<Move> others;

    // 遍历棋盘，评估和分类
    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] != -1)
                continue;

            if (terminate(i, j)) {
                return std::vector<std::pair<int, int>>{{i, j}};
            }

            int current_score = score_move(i, j);

            if (current_score >= Point_Scores::FIVE) {
                fives.push_back({{i, j}, current_score});
            } else {
                others.push_back({{i, j}, current_score});
            }
        }
    }

    // 处理最高优先级的“成五”返回情况
    if (!fives.empty()) {
        std::vector<std::pair<int, int>> result;
        result.reserve(fives.size());
        for (const auto &move : fives) {
            result.push_back(move.pos);
        }
        return result;
    }

    // 对所有非“成五”点排序
    // 可以在分数相同时，保持原有的棋盘扫描顺序，是更稳健的选择。
    std::stable_sort(
        others.begin(), others.end(),
        [](const Move &a, const Move &b) { return a.score > b.score; });

    // 步骤 A: 计算出“成四”点的数量
    size_t num_fours =
        std::count_if(others.begin(), others.end(), [&](const Move &move) {
            return move.score >= Point_Scores::LIVE_FOUR;
        });

    // 确定最终限制，既要满足 tot 的要求，又要保证所有“四”都被包括
    size_t limit = std::max(static_cast<size_t>(tot), num_fours);
    // 同时确保限制不超过总数
    limit = std::min(limit, others.size());

    std::vector<std::pair<int, int>> result;
    result.reserve(limit);
    for (size_t i = 0; i < limit; ++i) {
        result.push_back(others[i].pos);
    }

    return result;
}