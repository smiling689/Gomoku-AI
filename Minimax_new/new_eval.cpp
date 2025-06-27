#include "new_eval.h"
#include <cstring>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

extern int ai_side;

bool is_valid_pair(std::pair<int, int> x) {
    return x.first <= 14 && x.second <= 14 && x.first >= 0 && x.second >= 0;
}

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

int score_move(std::pair<int, int> p, int ai_color) //用于评估一个空格有多好
{
    // 我们如下给分：（下一步若可形成） 连五 1000000分  活四 10000分  死四、活三
    // 100分  死三 活二10分
    int total = 0;
    for (int color = 0; color <= 1; color++) {
        int flag = 1;

        for (int i = 0; i <= 3; i++) {
            int length = 1;
            std::pair<int, int> r_end = p + dir[i], l_end = p - dir[i];
            int flag_r = 1, flag_l = 1; // 1为死端，0为活端

            while (is_valid_pair(r_end) && get(r_end) == color) {
                length++;
                r_end = r_end + dir[i];
            }
            if (is_valid_pair(r_end) && get(r_end) == -1)
                flag_r = 0;

            while (is_valid_pair(l_end) && get(l_end) == color) {
                length++;
                l_end = l_end - dir[i];
            }
            if (is_valid_pair(l_end) && get(l_end) == -1)
                flag_l = 0;

            if (length >= 5)
                total += point_value[5] * flag;
            if (flag_l == 0 && flag_r == 0)
                total += point_value[length] * flag; //两个活端
            else if (!flag_l * flag_r == 1)
                total += point_value[-length] * flag; //有一端封死了
        }
    }
    return total;
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
    std::vector<std::pair<int, int>> res;

    std::vector<std::pair<int, int>> five;
    std::vector<std::pair<int, int>> four;
    std::vector<std::pair<int, int>> three;
    std::vector<std::pair<int, int>> rest;
    std::priority_queue<std::pair<int, std::pair<int, int>>> rest_list;

    for (int i = 0; i <= 14; i++)
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] != -1)
                continue;
            std::pair<int, int> p = {i, j};
            if (terminate(p.first, p.second))
                return std::vector<std::pair<int, int>>{p};
            if (score_move(p, ai_side) >= point_value[5])
                five.push_back(p);
            else if (score_move(p, ai_side) >= point_value[4])
                four.push_back(p);
            else if (score_move(p, ai_side) >= point_value[3])
                three.push_back(p);
            else
                rest_list.push({score_move(p, ai_side), p});
        }
    if (!five.empty())
        return five;

    while (!rest_list.empty()) {
        std::pair<int, int> tmp = rest_list.top().second;
        rest_list.pop();
        rest.push_back(tmp);
    }

    for (int i = 0; i < four.size(); i++) {
        res.push_back(four[i]);
        tot--;
    }
    for (int i = 0; i < three.size(); i++) {
        if (tot > 0) {
            res.push_back(three[i]);
            tot--;
        }
    }
    for (int i = 0; i < rest.size(); i++) {
        if (tot > 0) {
            res.push_back(rest[i]);
            tot--;
        }
    }

    return res;
}