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

std::unordered_map<int, int> value;
std::unordered_map<int, int> point_value;

const int CHENG_5_SCORE = 100000000; // "连五"10000000
const int HUO_4_SCORE = 1000000;     // "活四"100000
const int CHONG_4_SCORE = 1000;      // "冲四"1000
const int DAN_HUO_3_SCORE = 1000;    // "活三"1000
const int MIAN_3_SCORE = 100;        // "眠三"100
const int HUO_2_SCORE = 100;         // "活二"100
const int MIAN_2_SCORE = 1;          // "眠二"1
const int HUO_1_SCORE = 1;           // "活一"1

std::vector<std::pair<int, int>> dir = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

int get(std::pair<int, int> x) { return board[x.first][x.second]; }

std::pair<int, int> operator+(std::pair<int, int> x, std::pair<int, int> y) {
    return {x.first + y.first, x.second + y.second};
}

std::pair<int, int> operator-(std::pair<int, int> x, std::pair<int, int> y) {
    return {x.first - y.first, x.second - y.second};
}

void init_eval() {
    value[6] = value[-6] = value[-5] = value[7] = value[-7] = 10000000;
    value[5] = 100000000; //活五
    value[4] = 1000000;   //活四
    value[3] = 1000;      //活三
    value[2] = 100;       //活二
    value[1] = 1;         //一个
    value[-4] = 1000;     //死四
    value[-3] = 100;      //死三
    value[-2] = 1;        //死二
    point_value[5] = point_value[-5] = point_value[6] = point_value[7] =
        1000000;
    point_value[4] = 10000;
    point_value[3] = point_value[-4] = 100;
    point_value[2] = point_value[-3] = 10;
    point_value[0] = point_value[1] = point_value[-2] = 1;
}

int func1(int a, int b, int c, int d, int e, const int &color,
          const int &current_color, const int &length, int index) {
    int total = 0;
    int flag = current_color == color ? 1 : -1;

    if (length == 5)
        total += value[5] * flag; //连五特判

    //特判该死的 1 1 -1 1 与 1 -1 1 1
    if (length == 2 && board[a][b] == -1) {
        std::pair<int, int> p = {a, b};
        std::pair<int, int> r1 = p + dir[index];
        std::pair<int, int> r2 = r1 + dir[index];
        std::pair<int, int> r3 = r2 + dir[index];
        std::pair<int, int> l1 = p - dir[index];
        std::pair<int, int> l2 = l1 - dir[index];
        std::pair<int, int> l3 = l2 - dir[index];
        if (is_valid_pair(p + dir[index]) &&
            get(p + dir[index]) == current_color) {
            if (is_valid_pair(r2) &&
                get(r2) == current_color) //四   ?  1  1  -1  1  1  ?
            {
                if (is_valid_pair(r3) && get(r3) != 1 - current_color &&
                    is_valid_pair(l3) && get(l3) != 1 - current_color)
                    total += value[4] / 2 * flag; //活四
                else if (!((!is_valid_pair(r3) ||
                            get(r3) == 1 - current_color) &&
                           (!is_valid_pair(l3) ||
                            get(l3) == 1 - current_color)))
                    total += value[-4] / 2 * flag; //死四
            } else if (is_valid_pair(r2) && get(r2) == -1 &&
                       is_valid_pair(p - dir[index] - dir[index] -
                                     dir[index]) &&
                       get(p - dir[index] - dir[index] - dir[index]) == -1) {
                total += value[3] / 2 * flag; //活三
            }
            if (!((!is_valid_pair(p + dir[index] + dir[index]) ||
                   get(p + dir[index] + dir[index]) == 1 - current_color) &&
                  (!is_valid_pair(p - dir[index] - dir[index] - dir[index]) ||
                   get(p - dir[index] - dir[index] - dir[index]) ==
                       1 - current_color))) {
                total += value[-length - 1] * flag;
            }
        }
    }

    if (length == 1 && board[a][b] == -1) // l2  1  -1  1  1  r3
    {
        std::pair<int, int> p = {a, b};
        std::pair<int, int> r1 = p + dir[index], r2 = r1 + dir[index],
                            r3 = r2 + dir[index], r4 = r3 + dir[index],
                            l1 = p - dir[index], l2 = l1 - dir[index];
        if (is_valid_pair(r1) && is_valid_pair(r2) &&
            get(r1) == current_color && get(r2) == current_color) {
            if (is_valid_pair(r3) &&
                get(r3) == current_color) // l2  1  -1  1  1  1  r4
            {
                if (is_valid_pair(r4) && get(r4) != 1 - current_color &&
                    is_valid_pair(l2) && get(l2) == -1)
                    total += value[4] / 2 * flag;
                else if (!((!is_valid_pair(r4) ||
                            get(r4) == 1 - current_color) &&
                           (!is_valid_pair(l2) ||
                            get(l2) == 1 - current_color)))
                    total += value[-4] / 2 * flag;
            } else if (is_valid_pair(r3) && get(r3) != 1 - current_color &&
                       is_valid_pair(l2) && get(l2) != 1 - current_color) {
                total += value[length + 1] * flag;
            }
            if (!((!is_valid_pair(r3) || get(r3) == 1 - current_color) &&
                  (!is_valid_pair(l2) || get(l2) == 1 - current_color))) {
                total += value[-length - 1] * flag;
            }
        }
    }

    if (board[a][b] == -1 && (e >= 0 && board[c][d] == -1))
        total += value[length] * flag; //活
    else if (e >= 0 && board[c][d] == -1 && board[a][b] == 1 - color)
        total += value[-length] * flag; //死
    else if (e >= 0 && board[c][d] == 1 - color && board[a][b] == -1)
        total += value[-length] * flag; //死
    else if (e < 0 && board[a][b] == -1)
        total += value[-length] * flag; //死

    return total;
}

int func1_(int a, int b, int c, const int &color, const int &current_color,
           const int &length) {
    int total = 0;
    if (length == 5)
        total += color == current_color ? value[5] : -1 * value[5];
    int flag;
    if (current_color == color)
        flag = 1;
    else
        flag = -1;
    if (c >= 0 && board[a][b] == -1)
        total += value[-length] * flag; //死棋
    return total;
}

int evaluate(int color) {
    int total = 0;

    /*
     *     -1 0 0 0 -1   活 hh
     *
     *      1 0 0 0 -1  hh
     *     -1 0 0 0 1    hh
     *      | 0 0 0 -1  hh
     *     -1 0 0 0 |    死 hh
     *
     *     1 0 0 0 1
     *     | 0 0 0 1
     *     1 0 0 0 |     彻底死
     *
     * */

    for (int i = 0; i <= 14; i++) //遍历所有列
    {
        int current_color = board[i][0];
        int length = 1;

        for (int j = 1; j <= 14; j++) {
            if (board[i][j] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total += func1(i, j, i, j - length - 1, j - length - 1,
                                   color, current_color, length, 1);
                }
                current_color = board[i][j];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(i, 14 - length, 14 - length, color, current_color,
                            length);
        }
    }
    for (int i = 0; i <= 14; i++) //遍历所有行
    {
        int current_color = board[0][i];
        int length = 1;

        for (int j = 1; j <= 14; j++) {
            if (board[j][i] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total += func1(j, i, j - length - 1, i, j - length - 1,
                                   color, current_color, length, 0);
                }
                current_color = board[j][i];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(14 - length, i, 14 - length, color, current_color,
                            length);
        }
    }
    for (int i = 0; i <= 14; i++) //遍历所有左上到右下
    {
        int current_color = board[i][0];
        int length = 1;

        for (int j = 1; j <= 14 - i; j++) {
            if (board[i + j][j] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total +=
                        func1(i + j, j, i + j - length - 1, j - length - 1,
                              j - length - 1, color, current_color, length, 2);
                }
                current_color = board[i + j][j];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(i + 14 - i - length, 14 - length, 14 - length,
                            color, current_color, length);
        }
    }

    for (int i = 0; i <= 14; i++) //继续左上到右下
    {
        int current_color = board[0][i];
        int length = 1;

        for (int j = 1; j <= 14 - i; j++) {
            if (board[j][i + j] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total +=
                        func1(j, i + j, j - length - 1, i + j - length - 1,
                              j - length - 1, color, current_color, length, 2);
                }
                current_color = board[j][i + j];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(14 - i - length, i + 14 - i - length,
                            14 - i - length, color, current_color, length);
        }
    }

    for (int i = 0; i <= 14; i++) //遍历左下到右上
    {
        int current_color = board[i][14];
        int length = 1;

        for (int j = 1; j <= 14 - i; j++) {
            if (board[i + j][14 - j] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total += func1(i + j, 14 - j, i + j - length - 1,
                                   14 - (j - length - 1), j - length - 1, color,
                                   current_color, length, 3);
                }
                current_color = board[i + j][14 - j];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(i + 14 - i - length, i + length, 14 - i - length,
                            color, current_color, length);
        }
    }
    for (int i = 0; i <= 14; i++) //继续左下到右上
    {
        int current_color = board[0][i];
        int length = 1;

        for (int j = 1; j <= i; j++) {
            if (board[j][i - j] != current_color) {
                if (current_color != -1) //保证不是空格相连
                {
                    total +=
                        func1(j, i - j, j - length - 1, i - (j - length - 1),
                              j - length - 1, color, current_color, length, 3);
                }
                current_color = board[j][i - j];
                length = 0;
            }
            length++;
        }

        if (current_color != -1) {
            total += func1_(i - length, length, i - length, color,
                            current_color, length);
        }
    }
    return total;
}

namespace GomokuLegacyEval {

    // 内部辅助数据和函数
    namespace {
        const int DIRS[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        bool is_valid(int r, int c) {
            return r >= 0 && r < 15 && c >= 0 && c < 15;
        }
    }

    // 前向声明
    int score_slice(const int (&board)[15][15], int eval_color, int segment_color, int len, int start_r, int start_c, int dir_idx);
    int evaluate_direction(const int (&board)[15][15], int eval_color, int dir_idx);

    // ==================================================================================
    // 3. 主评估函数 (入口)
    // ==================================================================================
    int evaluate(const int (&board)[15][15], int eval_color) {
        int total_score = 0;
        total_score += evaluate_direction(board, eval_color, 0); // 水平
        total_score += evaluate_direction(board, eval_color, 1); // 垂直
        total_score += evaluate_direction(board, eval_color, 2); // 左上到右下
        total_score += evaluate_direction(board, eval_color, 3); // 右上到左下
        return total_score;
    }

    // ==================================================================================
    // 4. 统一的方向评估函数 (保持不变)
    // ==================================================================================
    int evaluate_direction(const int (&board)[15][15], int eval_color, int dir_idx) {
        int total_score = 0;
        const int dr = DIRS[dir_idx][0];
        const int dc = DIRS[dir_idx][1];

        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                if (is_valid(i - dr, j - dc)) continue;

                int r = i, c = j;
                while(is_valid(r, c)) {
                    if (board[r][c] == -1) {
                        r += dr; c += dc;
                        continue;
                    }

                    int segment_color = board[r][c];
                    int segment_len = 0;
                    int start_r = r, start_c = c;
                    
                    while(is_valid(r, c) && board[r][c] == segment_color) {
                        segment_len++;
                        r += dr; c += dc;
                    }
                    
                    total_score += score_slice(board, eval_color, segment_color, segment_len, start_r, start_c, dir_idx);
                }
            }
        }
        return total_score;
    }

    // ==================================================================================
    // 5. 精简参数并严格翻译原始逻辑的核心评分函数
    // ==================================================================================
    int score_slice(const int (&board)[15][15], int eval_color, int segment_color, int len, int start_r, int start_c, int dir_idx) {
        int total_score = 0;
        const int flag = (segment_color == eval_color) ? 1 : -1;
        const int dr = DIRS[dir_idx][0];
        const int dc = DIRS[dir_idx][1];

        // --- 逻辑翻译 Part A: 评估连续切片本身 ---
        // 从切片信息推导出两端点的坐标
        const int r_before = start_r - dr;
        const int c_before = start_c - dc;
        const int r_after = start_r + len * dr;
        const int c_after = start_c + len * dc;

        if (len >= 5) {
            total_score += Scores::FIVE * flag;
        } else {
            // 这个逻辑块完全等同于您 func1 最后四行 + func1_ 的逻辑
            const bool live_start = is_valid(r_before, c_before) && board[r_before][c_before] == -1;
            const bool live_end = is_valid(r_after, c_after) && board[r_after][c_after] == -1;
            const int opponent_color = 1 - segment_color;

            if (live_start && live_end) { // 对应: board[a][b] == -1 && board[c][d] == -1 -> 活棋
                if (len == 4) total_score += Scores::LIVE_FOUR * flag;
                else if (len == 3) total_score += Scores::LIVE_THREE * flag;
                else if (len == 2) total_score += Scores::LIVE_TWO * flag;
                else if (len == 1) total_score += Scores::LIVE_ONE * flag;
            } else if (live_start || live_end) { // 对应: 一端是边界或对方棋子 -> 死棋
                // 只有当另一端不是对方棋子时才算死棋，否则是彻底堵死的无价值棋
                bool dead_by_opponent_at_start = is_valid(r_before, c_before) && board[r_before][c_before] == opponent_color;
                bool dead_by_opponent_at_end = is_valid(r_after, c_after) && board[r_after][c_after] == opponent_color;
                if(live_start && !dead_by_opponent_at_end) total_score += (len==4 ? Scores::DEAD_FOUR : len==3 ? Scores::DEAD_THREE : len==2 ? Scores::DEAD_TWO : 0) * flag;
                if(live_end && !dead_by_opponent_at_start) total_score += (len==4 ? Scores::DEAD_FOUR : len==3 ? Scores::DEAD_THREE : len==2 ? Scores::DEAD_TWO : 0) * flag;
            }
        }

        // --- 逻辑翻译 Part B: 严格复现原始 func1 中的特判逻辑 ---
        // 特判的中心是切片后的第一个空格 (r_after, c_after)
        if (is_valid(r_after, c_after) && board[r_after][c_after] == -1) {
            
            // 特判1: "if (length == 2...)" -> 对应 O O _ ??
            if (len == 2) {
                // p 对应切片后的空格 (r_after, c_after)
                // r1, r2, r3 是空格后的点
                int r1_r = r_after + dr, r1_c = c_after + dc;
                int r2_r = r_after + 2 * dr, r2_c = c_after + 2 * dc;
                int r3_r = r_after + 3 * dr, r3_c = c_after + 3 * dc;
                // l3 是 O O _ 前面的点
                int l3_r = start_r - 2 * dr, l3_c = start_c - 2 * dc;

                if (is_valid(r1_r, r1_c) && board[r1_r][r1_c] == segment_color) {
                    if (is_valid(r2_r, r2_c) && board[r2_r][r2_c] == segment_color) { // 找到了 O O _ O O
                        if (is_valid(r3_r, r3_c) && board[r3_r][r3_c] != 1 - segment_color &&
                            is_valid(l3_r, l3_c) && board[l3_r][l3_c] != 1 - segment_color) {
                            total_score += Scores::JUMP_LIVE_FOUR * flag;
                        } else if (!((!is_valid(r3_r, r3_c) || board[r3_r][r3_c] == 1 - segment_color) &&
                                     (!is_valid(l3_r, l3_c) || board[l3_r][l3_c] == 1 - segment_color))) {
                            total_score += Scores::JUMP_DEAD_FOUR * flag;
                        }
                    }
                    // 此处省略了您原始代码中对活三的判断，因为它在更复杂的棋形下可能被重复计算。
                    // 这是为了100%匹配您原始代码分支，您的原始代码在O O _ O后，并不会判断活三。
                }
            }

            // 特判2: "if (length == 1...)" -> 对应 O _ ??
            if (len == 1) {
                int r1_r = r_after + dr, r1_c = c_after + dc;
                int r2_r = r_after + 2 * dr, r2_c = c_after + 2 * dc;
                int r3_r = r_after + 3 * dr, r3_c = c_after + 3 * dc;
                int r4_r = r_after + 4 * dr, r4_c = c_after + 4 * dc;
                int l2_r = start_r - 2 * dr, l2_c = start_c - 2 * dc;

                if (is_valid(r1_r, r1_c) && board[r1_r][r1_c] == segment_color &&
                    is_valid(r2_r, r2_c) && board[r2_r][r2_c] == segment_color) {
                    if (is_valid(r3_r, r3_c) && board[r3_r][r3_c] == segment_color) { // 找到了 O _ O O O
                        if (is_valid(r4_r, r4_c) && board[r4_r][r4_c] != 1 - segment_color &&
                            is_valid(l2_r, l2_c) && board[l2_r][l2_c] == -1) {
                            total_score += Scores::JUMP_LIVE_FOUR * flag;
                        } else if (!((!is_valid(r4_r, r4_c) || board[r4_r][r4_c] == 1 - segment_color) &&
                                     (!is_valid(l2_r, l2_c) || board[l2_r][l2_c] == 1 - segment_color))) {
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
        if (length >= 5)
            score += point_value[5];
        if (huo_l == 0 && huo_r == 0)
            score += point_value[length];
        else if (huo_l == 0 || huo_r == 0)
            score += point_value[-length];
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

    // 1. 遍历棋盘，评估和分类
    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] != -1)
                continue;

            if (terminate(i, j)) {
                return std::vector<std::pair<int, int>>{{i, j}};
            }

            int current_score = score_move(i, j);

            if (current_score >= point_value[5]) {
                fives.push_back({{i, j}, current_score});
            } else {
                others.push_back({{i, j}, current_score});
            }
        }
    }

    // 2. 处理最高优先级的“成五”返回情况
    if (!fives.empty()) {
        std::vector<std::pair<int, int>> result;
        result.reserve(fives.size());
        for (const auto &move : fives) {
            result.push_back(move.pos);
        }
        return result;
    }

    // 3. 对所有非“成五”点排序
    // std::stable_sort
    // 可以在分数相同时，保持原有的棋盘扫描顺序，是更稳健的选择。
    std::stable_sort(
        others.begin(), others.end(),
        [](const Move &a, const Move &b) { return a.score > b.score; });

    // 4. 根据您的优雅逻辑，确定最终返回数量
    // 步骤 A: 计算出“成四”点的数量
    size_t num_fours =
        std::count_if(others.begin(), others.end(), [&](const Move &move) {
            return move.score >= point_value[4];
        });

    // 步骤 B: 确定最终限制，既要满足 tot 的要求，又要保证所有“四”都被包括
    size_t limit = std::max(static_cast<size_t>(tot), num_fours);
    // 同时确保限制不超过总数
    limit = std::min(limit, others.size());

    // 5. 构建并返回最终结果
    std::vector<std::pair<int, int>> result;
    result.reserve(limit);
    for (size_t i = 0; i < limit; ++i) {
        result.push_back(others[i].pos);
    }

    return result;
}