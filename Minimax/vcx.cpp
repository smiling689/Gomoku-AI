#include "vcx.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>

// 外部变量
extern int board[15][15];
extern int ai_side;

// 常量定义
const int SIZE = 15;
const int VCX_DEP = 2000;

enum Cell { EMPTY = -1, BLACK = 0, WHITE = 1 };
enum Threat { NONE, WIN, OPEN_FOUR, DOUBLE_THREE, FOUR_THREE, FOUR, OPEN_THREE, THREE };



std::pair<int, int> find_win_in_one_move(int player);
std::map<Threat, int> analyze_threats(int r, int c, int player);
std::pair<int, int> find_move_by_threat(int player, Threat threat_level);
std::vector<std::pair<int, int>> generate_threats(int player, bool is_vct);
bool solve_min_node(int opponent, int dep, bool is_vct);
std::pair<int, int> solve_max_node(int player, int dep, bool is_vct);

// “一步制胜”检测函数
std::pair<int, int> find_win_in_one_move(int player) {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
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


    // --- 阶段二：处理次级威胁 ---
    
    // 检查对方是否存在“活三”或“冲四”的威胁
    auto opp_four_move = find_move_by_threat(opponent_side, FOUR);
    auto opp_open_three_move = find_move_by_threat(opponent_side, OPEN_THREE);

    bool has_opp_threat = (opp_four_move.first != -1) || (opp_open_three_move.first != -1);

    if (has_opp_threat) {
        // 检查我方是否有“冲四”的手段
        auto my_four_move = find_move_by_threat(ai_side, FOUR);

        // 如果对方有威胁，但我方没有冲四或更强的反击，则交由Minimax处理
        if (my_four_move.first == -1) {
            #ifdef DEBUG_MODE
            std::cerr << "VCX abstained: Opponent has threats, but AI has no decisive counter-attack. Passing to Minimax." << std::endl;
            #endif
            return {-1, -1}; // 退出算杀
        }
    }

    // --- 阶段三：如果局面简单，没有上述复杂情况，则执行次级威胁处理 ---
    
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
    bool flag = false;
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
    if(flag && threats[FOUR_THREE] > 0){
        std::cerr << "YES!!!" << std::endl;
    }
    return threats;
}

std::pair<int, int> find_move_by_threat(int player, Threat threat_level) {
    for (int r = 0; r < SIZE; ++r) 
    for (int c = 0; c < SIZE; ++c) {
        if (board[r][c] == EMPTY) {
            auto threats = analyze_threats(r, c, player);
            if (threats.count(threat_level) && threats[threat_level] > 0) return {r, c};
        }
    }
    return {-1, -1};
}



// --- 多步算杀模块主入口 (已修改) ---
std::pair<int, int> find_victory_sequence(int max_depth) {
    int opponent_side = 1 - ai_side;

    // 优先级 1: 我方一步胜利 (活五)
    auto my_move = find_win_in_one_move(ai_side);
    if (my_move.first != -1) return my_move;

    // 优先级 2: 防守对方的一步胜利
    auto opp_move = find_win_in_one_move(opponent_side);
    if (opp_move.first != -1) return opp_move;

    // 优先级 3: 搜索我方是否存在VCT/VCF必胜序列
    // --- 修改点：调用参数化的函数 ---
    auto vct_move = solve_max_node(ai_side, max_depth, true);
    if (vct_move.first != -1) {

        std::cerr << "VCT Found a winning sequence for AI starting at (" << vct_move.first << ", " << vct_move.second << ")" << std::endl;

        return vct_move;
    }
    auto vcf_move = solve_max_node(ai_side, max_depth, false);
    if (vcf_move.first != -1) {

        std::cerr << "VCF Found a winning sequence for AI starting at (" << vcf_move.first << ", " << vcf_move.second << ")" << std::endl;

        return vcf_move;
    }

    // --- 新增逻辑：优先级 4: 搜索对方是否存在VCT/VCF必胜序列，并加以防守 ---
    // 如果我们自己没有杀棋，就必须检查对方有没有。
    auto opp_vct_move = solve_max_node(opponent_side, max_depth, true);
    if (opp_vct_move.first != -1) {

        std::cerr << "VCT Found a winning sequence for OPPONENT. Blocking at (" << opp_vct_move.first << ", " << opp_vct_move.second << ")" << std::endl;

        return opp_vct_move; // 返回对方杀棋的起点，即为我方的防守点
    }
    auto opp_vcf_move = solve_max_node(opponent_side, max_depth, false);
    if (opp_vcf_move.first != -1) {

        std::cerr << "VCF Found a winning sequence for OPPONENT. Blocking at (" << opp_vcf_move.first << ", " << opp_vcf_move.second << ")" << std::endl;

        return opp_vcf_move; // 返回对方杀棋的起点，即为我方的防守点
    }

    return {-1, -1}; // 算杀未找到任何必胜或必须防守的点
}


// MAX层：当前玩家(player)回合，寻找一个能赢的威胁步 (已修改)
// --- 修改点：函数签名增加 player 参数，内部逻辑通用化 ---
std::pair<int, int> solve_max_node(int player, int dep, bool is_vct) {
    if (dep <= 0) return {-1, -1};
    
    // 检查是否有活四、双三等直接获胜的机会
    auto open_four = find_move_by_threat(player, OPEN_FOUR);
    if(open_four.first != -1) return open_four;
    auto four_three = find_move_by_threat(player, FOUR_THREE);
    if(four_three.first != -1) return four_three;
    auto double_three = find_move_by_threat(player, DOUBLE_THREE);
    if(double_three.first != -1) return double_three;

    // 生成当前玩家的所有威胁（活三或冲四）
    auto threats = generate_threats(player, is_vct);

    for (const auto& move : threats) {
        board[move.first][move.second] = player;
        // --- 修改点：调用min层时传入对方player ---
        bool can_win = solve_min_node(1 - player, dep - 1, is_vct);
        board[move.first][move.second] = EMPTY; // 回溯

        if (can_win) {
            return move;
        }
    }

    return {-1, -1};
}

// MIN层：对方(opponent)回合，检查我方(1-opponent)是否能应对所有防守 (已修改)
// --- 修改点：函数签名增加 opponent 参数，内部逻辑通用化 ---
bool solve_min_node(int opponent, int dep, bool is_vct) {
    if (dep <= 0) return false;
    
    int player = 1 - opponent; // 当前玩家，即MAX层的玩家

    // 检查对方是否能一步胜利，如果能，我方杀棋失败
    auto opp_win_move = find_win_in_one_move(opponent);
    if (opp_win_move.first != -1) return false;

    // 检查对方是否有活四/四三等必胜棋，如果有，我方杀棋失败
    auto opp_open_four = find_move_by_threat(opponent, OPEN_FOUR);
    if(opp_open_four.first != -1) return false;
    auto opp_four_three = find_move_by_threat(opponent, FOUR_THREE);
    if(opp_four_three.first != -1) return false;
    

    // 生成对方所有可能的应对策略
    std::set<std::pair<int,int>> reply_set;

    // 策略1：防守我方(player)上一步制造的威胁点。
    // 注意：这里的generate_threats是为player生成的，因为对方要防守player的棋
    auto defense_points = generate_threats(player, is_vct); 
    for(const auto& p : defense_points) reply_set.insert(p);

    // 策略2：下出他们自己的威胁进行反击
    auto counter_threats = generate_threats(opponent, true); // 对方总是可以尝试活三反击
    auto counter_fours = generate_threats(opponent, false);
    for(const auto& p : counter_threats) reply_set.insert(p);
    for(const auto& p : counter_fours) reply_set.insert(p);
    
    // 如果对方无棋可走（例如我方已形成活四，对方找不到防守点），则我方胜利
    // 这种情况通常在 max_node 的递归终点就被检测到了，但作为保险
    if (reply_set.empty()) {
        auto check_win = find_win_in_one_move(player);
        if (check_win.first != -1) return true;
    }

    // 遍历对方的每一种应对策略
    for (const auto& reply_move : reply_set) {
        board[reply_move.first][reply_move.second] = opponent;
        // --- 修改点：调用max层时传入我方player ---
        auto next_win_move = solve_max_node(player, dep - 1, is_vct);
        board[reply_move.first][reply_move.second] = EMPTY; // 回溯

        // 如果对于某一种防守，我方(player)无法继续构成杀棋，则本次杀棋失败
        if (next_win_move.first == -1) {
            return false;
        }
    }

    // 如果对方的所有防守策略，我方都有后续手段，则我方胜利
    return true;
}

// 辅助函数：生成威胁点 (你的原函数，无需修改)
std::vector<std::pair<int, int>> generate_threats(int player, bool is_vct) {
    std::vector<std::pair<int, int>> moves;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (board[r][c] == EMPTY) {
                Threat threat_to_find = is_vct ? OPEN_THREE : FOUR;
                auto threats = analyze_threats(r, c, player);
                if (threats.count(threat_to_find) && threats[threat_to_find] > 0) {
                    moves.push_back({r, c});
                }
                // VCF时，活四也算一种特殊的冲四，四三也算
                if (!is_vct) {
                    if (threats.count(OPEN_FOUR) && threats[OPEN_FOUR] > 0) moves.push_back({r, c});
                    if (threats.count(FOUR_THREE) && threats[FOUR_THREE] > 0) moves.push_back({r, c});
                }
                 // VCT时，四三和双三也算
                if(is_vct){
                    if(threats.count(FOUR_THREE) && threats[FOUR_THREE] > 0) moves.push_back({r, c});
                    if(threats.count(DOUBLE_THREE) && threats[DOUBLE_THREE] > 0) moves.push_back({r, c});
                }
            }
        }
    }
    return moves;
}

