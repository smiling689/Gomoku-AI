// smiling689/gomoku-ai/Gomoku-AI-084a86008736698bc8c92e1ecc4299b7d4a57e2e/Gomoku-AI-2024/judge/vcx.cpp
#include "vcx.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

// 外部变量
extern int board[15][15];
extern int ai_side;

// 常量定义
const int BOARD_SIZE = 15;
const int VCX_DEP = 10;

enum Cell { EMPTY = -1, BLACK = 0, WHITE = 1 };
enum Threat { NONE, WIN, OPEN_FOUR, DOUBLE_THREE, FOUR_THREE, FOUR, OPEN_THREE, THREE };

// 辅助函数：检查坐标是否合法

// “一步制胜”检测函数
std::pair<int, int> find_win_in_one_move(int player) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c] == EMPTY) {
                board[r][c] = player; // 假设落子
                int dr[] = {1, 1, 0, -1};
                int dc[] = {0, 1, 1, 1};
                for (int i = 0; i < 4; ++i) {
                    int count = 0;
                    for(int k = -4; k <= 4; ++k) {
                        if (is_valid(r + k * dr[i], c + k * dc[i]) && board[r + k * dr[i]][c + k * dc[i]] == player) {
                            if (++count >= 5) {
                                board[r][c] = EMPTY; // 撤销
                                return {r, c};
                            }
                        } else {
                            count = 0;
                        }
                    }
                }
                board[r][c] = EMPTY; // 撤销
            }
        }
    }
    return {-1, -1};
}

std::map<Threat, int> analyze_threats(int r, int c, int player);
std::pair<int, int> find_move_by_threat(int player, Threat threat_level);


// “知难而退”策略
std::pair<int, int> find_victory(int dep) {
    int opponent_side = 1 - ai_side;

    // --- 阶段一：处理确定性的、必须立即响应的走法 ---

    // 优先级 1: 我方一步胜利
    auto my_move = find_win_in_one_move(ai_side);
    if (my_move.first != -1) return my_move;

    // 优先级 2: 防守对方的一步胜利
    auto opp_move = find_win_in_one_move(opponent_side);
    if (opp_move.first != -1) return opp_move;

    // 优先级 3: 我方制造必胜棋型 (活四/双活三)
    my_move = find_move_by_threat(ai_side, OPEN_FOUR);
    if (my_move.first != -1) return my_move;
    my_move = find_move_by_threat(ai_side, DOUBLE_THREE);
    if (my_move.first != -1) return my_move;
     my_move = find_move_by_threat(ai_side, FOUR_THREE);
    if (my_move.first != -1) return my_move;

    // 优先级 4: 防守对方的必胜棋型
    opp_move = find_move_by_threat(opponent_side, OPEN_FOUR);
    if (opp_move.first != -1) return opp_move;
    opp_move = find_move_by_threat(opponent_side, DOUBLE_THREE);
    if (opp_move.first != -1) return opp_move;
    opp_move = find_move_by_threat(opponent_side, FOUR_THREE);
    if (opp_move.first != -1) return opp_move;


    // --- 阶段二：处理次级威胁，并根据情况决定是否“弃权” ---
    
    // 检查对方是否存在“活三”或“冲四”的威胁
    auto opp_four_move = find_move_by_threat(opponent_side, FOUR);
    auto opp_open_three_move = find_move_by_threat(opponent_side, OPEN_THREE);

    bool has_opp_threat = (opp_four_move.first != -1) || (opp_open_three_move.first != -1);

    if (has_opp_threat) {
        // 检查我方是否有“冲四”的强力反击手段
        auto my_four_move = find_move_by_threat(ai_side, FOUR);

        // 如果对方有威胁，但我方没有冲四或更强的反击，则情况复杂，交由Minimax处理
        if (my_four_move.first == -1) {
            #ifdef DEBUG_MODE
            std::cerr << "VCX abstained: Opponent has threats, but AI has no decisive counter-attack. Passing to Minimax." << std::endl;
            #endif
            return {-1, -1}; // “知难而退”，退出算杀
        }
    }

    // --- 阶段三：如果局面简单，没有上述复杂情况，则按原计划执行次级威胁处理 ---
    
    // 防守对方的普通冲四
    if (opp_four_move.first != -1) return opp_four_move;
    // (防守活三的逻辑已包含在上面的判断中，若执行到这里，说明我方有强力反击，应优先进攻)


    // 我方主动走普通冲四
    auto my_four_move = find_move_by_threat(ai_side, FOUR);
    if (my_four_move.first != -1) return my_four_move;


    // 默认情况：算杀模块没有找到任何有把握的棋步
    return {-1, -1};
}


std::map<Threat, int> analyze_threats(int r, int c, int player) {
    if (!is_valid(r, c) || board[r][c] == EMPTY) return {};
    std::map<Threat, int> threats;
    int dr[] = {1, 0, 1, 1}; int dc[] = {0, 1, 1, -1};
    board[r][c] = player;
    for (int i = 0; i < 4; ++i) {
        std::string line_str = "";
        for (int k = -5; k <= 5; ++k) {
            int cr = r + k * dr[i], cc = c + k * dc[i];
            if (!is_valid(cr, cc)) line_str += 'X';
            else if (board[cr][cc] == player) line_str += 'P';
            else if (board[cr][cc] == 1 - player) line_str += 'O';
            else line_str += '_';
        }
        if (line_str.find("PPPPP") != std::string::npos) threats[WIN]++;
        else if (line_str.find("_PPPP_") != std::string::npos) threats[OPEN_FOUR]++;
        else if (line_str.find("OPPPP_") != std::string::npos || line_str.find("_PPPPO") != std::string::npos) threats[FOUR]++;
        else if (line_str.find("P_PPP") != std::string::npos || line_str.find("PPP_P") != std::string::npos || line_str.find("PP_PP") != std::string::npos) threats[FOUR]++;
        else if (line_str.find("_PPP_") != std::string::npos) threats[OPEN_THREE]++;
        else if (line_str.find("_P_PP_") != std::string::npos || line_str.find("_PP_P_") != std::string::npos) threats[OPEN_THREE]++;
    }
    board[r][c] = EMPTY;
    if (threats[OPEN_THREE] >= 2) threats[DOUBLE_THREE]++;
    if (threats[FOUR] >= 1 && threats[OPEN_THREE] >= 1) threats[FOUR_THREE]++;
    return threats;
}
std::pair<int, int> find_move_by_threat(int player, Threat threat_level) {
    for (int r = 0; r < BOARD_SIZE; ++r) for (int c = 0; c < BOARD_SIZE; ++c) {
        if (board[r][c] == EMPTY) {
            auto threats = analyze_threats(r, c, player);
            if (threats.count(threat_level) && threats[threat_level] > 0) return {r, c};
        }
    }
    return {-1, -1};
}