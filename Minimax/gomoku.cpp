// g++ Minimax/gomoku.cpp Minimax/my_eval_hash.cpp Minimax/flip.cpp Minimax/zobrist.cpp Minimax/vcx.cpp -o gomoku           编译
// python Minimax/judge.py ./gomoku ./baseline                                                                              时，我的ai是ai0(先手，黑棋)
// python Minimax/judge.py ./baseline ./gomoku                                                                              时，我的ai是ai1(后手，白棋)
#include "AIController.h"
#include "zobrist.h"
#include "flip.h"
#include "vcx.h"
#include <utility>
#include <cstring>
#include <climits>
#include <vector>
#include <ctime>
#include <algorithm>
#include "my_eval_hash.h"
#include <chrono>
#include <random>
// #define DEBUG_MODE //是否开启调试模式
// #define timing
// #define pos_val
#define huanshou

// #define string_eval //使用string.find进行查找评估
// #define vector_eval //使用vector对每种棋型找一遍来评估
#define hash_eval //构建哈希表来评估，滑动窗口
#define deeping //迭代加深接口
#define mizi

extern int ai_side; //0: black, 1: white
//ai_side == 0：代表AI执黑棋。
//ai_side == 1：代表AI执白棋。
//对手是 1-ai_side
std::string ai_name = "Smiling_AI";

int turn = 0;
int current_turn = 0;

const int SIZE = 15;
int board[SIZE][SIZE];
const int INF = INT_MAX;
const int DEP = 8; //depth接口，表示搜索深度
// const int save_moves = 7;//从生成的可能moves中选前若干个进行计算
int save_moves[8] = {7, 7, 7, 7, 7, 7, 7, 7};


enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

struct MoveScore {
    std::pair<int, int> move;
    int score;
};

const int POSITION_WEIGHT_FACTOR = 10;

// 15x15 位置权重矩阵
const int positional_weights[15][15] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 6, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 6, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

MinimaxResult Minimax(int depth);

MinimaxResult deepingMinimax();

MinimaxResult deepingMinimax_2();

int min_value(int alpha, int beta, int depth);

int max_value(int alpha, int beta, int depth);


namespace rand_int {
    std::mt19937 rnd(std::chrono::system_clock::now().time_since_epoch().count());
    int get(int l, int r) { return rnd() % (r - l + 1ll) + l; }
}

using rand_int::get;


void init() {
    srand(time(0));
    transposition_table.clear();
    memset(board, EMPTY, sizeof(board));
    init_zobrist();
    init_pattern_db();
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.

std::pair<int, int> getRandom() {
    while (true) {
        // int x = rand() % SIZE;
        // int y = rand() % SIZE;
        int x = get(0, 14);
        int y = get(0, 14);
        if (board[x][y] == EMPTY) {
            board[x][y] = ai_side;
            return std::make_pair(x, y);
        }
    }
}

bool is_valid(int r, int c) {
    return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
}

bool adj(int r, int c) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            if (is_valid(r + i, c + j) && board[r + i][c + j] != EMPTY) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::pair<int, int> > generate_moves() {
    std::vector<std::pair<int, int> > moves;
    bool visited[SIZE][SIZE] = {false};

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            // 只考虑已有棋子周围的空点
            if (board[r][c] == EMPTY && adj(r, c)) {
                if (r + c <= 1 && ai_side == WHITE) {
                    continue;
                }
                if (!visited[r][c]) {
                    moves.push_back({r, c});
                    visited[r][c] = true;
                }
            }
        }
    }
    return moves;
}

std::vector<std::pair<int, int> > generate_sorted_moves(int current_player, int depth) {
    auto moves = generate_moves();
    if (moves.empty()) {
        return {};
    }

    std::vector<MoveScore> scored_moves;
    int opponent_player = 1 - current_player;

    for (const auto &move: moves) {
        // 评估每个点
        // 分数 = 我方在此落子的进攻分 + 对方在此落子的防守分
        int offensive_score = score_move(move.first, move.second, current_player);
        int defensive_score = score_move(move.first, move.second, opponent_player);
        scored_moves.push_back({move, offensive_score + defensive_score});
    }

    // 按分数从高到低排序
    std::sort(scored_moves.begin(), scored_moves.end(), [](const MoveScore &a, const MoveScore &b) {
        return a.score > b.score;
    });

    // 提取排序后的坐标
    std::vector<std::pair<int, int> > sorted_moves;
    for (const auto &scored_move: scored_moves) {
        sorted_moves.push_back(scored_move.move);
    }
    size_t count = std::min(static_cast<size_t>(save_moves[DEP - depth]), sorted_moves.size());
    sorted_moves.assign(sorted_moves.begin(), sorted_moves.begin() + count);

    return sorted_moves;
}

// 一个辅助函数，用来计算棋盘上的棋子数
int count_black() {
    int count = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == BLACK) {
                count++;
            }
        }
    }
    return count;
}

int count_white() {
    int count = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == WHITE) {
                count++;
            }
        }
    }
    return count;
}

void flip_board() {
    // std::cerr << "before flip : " << current_total_score << std::endl;

    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == BLACK) {
                board[i][j] = WHITE;
            } else if (board[i][j] == WHITE) {
                board[i][j] = BLACK;
            }
        }
    }
#ifdef mizi
    recalculate_full_board_score();
#endif


    // std::cerr << "after flip : " << current_total_score << std::endl;
}

std::pair<int, int> action(std::pair<int, int> loc) {
#ifdef timing
        clock_t start = clock();
#endif
    int i = loc.first, j = loc.second;
    if (loc.first != -1 && loc.second != -1) {
        int score_before = update_score_for_position(i, j);
        // std::cerr << "score before : !!! " << score_before << std::endl;
        board[i][j] = 1 - ai_side;
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        // std::cerr << "score_before:" << score_before << " after : " << score_after << std::endl;
        current_total_score += (score_after - score_before);

        update_hash(loc.first, loc.second, 1 - ai_side);
    }

    // std::cerr << "current_score : total __" << current_total_score << std::endl;

    // 2. 根据棋盘状态，计算当前是第几手棋
    // 已经下了 stone_count 手，现在轮到我们下第 stone_count + 1 手
    int black = count_black();
    int white = count_white();
    current_turn = black + white + 1;
#ifdef DEBUG_MODE
    std::cerr << "current_turn is " << current_turn << std::endl;
#endif

    if (current_turn == SIZE * SIZE) {
        return getRandom();
    }

    // --- 回合 1: 我方是先手 (黑棋) ---
    if (current_turn == 1) {
        auto random = getRandom();
        // std::pair<int , int > random = {4, 4};
        i = random.first, j = random.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[random.first][random.second] = ai_side;
        update_hash(random.first, random.second, ai_side);
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif
        return random;
    }

    if (current_turn == 2) {
        std::cerr << "2 " << std::endl;

        // auto random = getRandom(); 
        // std::pair<int , int > random = {SIZE - loc.first - 1 , SIZE - loc.second - 1};
        std::pair<int, int> random = {0, 0};
        if (board[0][0] != EMPTY) {
            random = {1, 0};
        }
        i = random.first, j = random.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[random.first][random.second] = ai_side;
        update_hash(random.first, random.second, ai_side);
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif
        return random;
    }

    if (current_turn == 3) {
        // auto random = getRandom(); 
        std::pair<int, int> random = {14, 14};
        if (board[14][14] != EMPTY) {
            random = {1, 14};
        }
        i = random.first, j = random.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[random.first][random.second] = ai_side;
        update_hash(random.first, random.second, ai_side);
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif
        return random;
    }


    // --- 回合 2: 我方是后手 (白棋)，决定是否交换 ---
    if (ai_side == WHITE && black == 2 && white == 1) {
        // int no_flip = no_flip_score();
        // int flip = flip_score();
        bool flag = true;
        // if (flip < no_flip) {
        if (flag) {
            // 确认要换手，执行真正的翻转并返回
            flip_board();
            current_hash = calculate_hash();
#ifdef timing
            clock_t end = clock();
            double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
            std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif
            std::cerr << "flip " << std::endl;
            return {-1, -1};
        } else {
#ifdef DEBUG_MODE
            std::cerr << "Smiling_AI: Not swapping is better. Playing regular move." << std::endl;
#endif
        }
    }


    if (ai_side == BLACK && black == 2 && white == 1 && loc.first == -1) {
#ifdef DEBUG_MODE
        std::cerr << "Smiling_AI: Opponent swapped. " << std::endl;
#endif
        flip_board();
        current_hash = calculate_hash();
    }


    // ================== 算杀逻辑开始 ==================
    std::pair<int, int> vcx_move = find_victory(VCX_DEP);
    if (vcx_move.first != -1) {
#ifdef DEBUG_MODE
        std::cerr << "Smiling_AI: VCX found a winning move at (" << vcx_move.first << ", " << vcx_move.second << ")" <<
                std::endl;
#endif

        i = vcx_move.first;
        j = vcx_move.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[i][j] = ai_side;
        update_hash(i, j, ai_side);
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif

#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "VCX耗时: " << elapsed_secs << " 秒" << std::endl;
#endif

        return vcx_move;
    }
    // ================== 算杀逻辑结束 ==================


    // 4. 对于所有常规回合，使用 Minimax 算法计算最佳落子
    MinimaxResult my_move = deepingMinimax();

    if (current_turn == 2) {
        my_move = deepingMinimax_2();
    }

    const int WIN_SCORE = 5000000;
    bool already_found_win = (ai_side == BLACK && my_move.score >= WIN_SCORE) ||
                             (ai_side == WHITE && my_move.score <= -WIN_SCORE);

    // 如果主搜索没找到必胜局，则启动独立的深度算杀
    if (!already_found_win) {
#ifdef DEBUG_MODE
        std::cerr << "Minimax didn't find a win. Starting deep VCX search..." << std::endl;
#endif
        std::pair<int, int> vcx_move = find_victory_sequence(2000); // 新算杀入口
        if (vcx_move.first != -1) {
#ifdef DEBUG_MODE
            std::cerr << "VCX search found a winning sequence! Overriding Minimax move." << std::endl;
#endif
            my_move.move = vcx_move;
        }
    }


    if (my_move.move.first != -1 && my_move.move.second != -1) {
        i = my_move.move.first, j = my_move.move.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[my_move.move.first][my_move.move.second] = ai_side;

        update_hash(my_move.move.first, my_move.move.second, ai_side);
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
    } else {
        std::cerr << "MiniMax wrong! -1,-1" << std::endl;
        // std::throw runtime_error("Minimax_wrong");
    }

#ifdef timing
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        std::cerr << "耗时: " << elapsed_secs << " 秒" << std::endl;
#endif

    return my_move.move;
}


//terminal用于判断当前棋局是否已经结束，-1表示平局，0表示黑子胜，1表示白子胜，2表示未结束
int terminal() {
    int win = winner();
    if (win != EMPTY) {
        return win;
    } //如果已经决出胜者了，返回胜者
    for (int i = 0; i <= 14; i++) {
        for (int j = 0; j <= 14; j++) {
            if (board[i][j] == EMPTY) {
                return 2;
            }
        }
    } //如果所有格子都满了，也返回empty（平局）
    return EMPTY;
}


// 前几步只搜两步
MinimaxResult deepingMinimax_2() {
#ifdef deeping

    clock_t start_time = clock();

    MinimaxResult best_result = {{-1, -1}, 0};

    for (int depth = 2; depth <= 2; depth += 2) {
        auto current_result = Minimax(depth);

        // 无论如何，都使用更深层次搜索的结果来更新最佳走法
        best_result = current_result;

#ifdef DEBUG_MODE
        std::cerr << "Depth " << depth << " best move: (" << best_result.move.first << "," << best_result.move.second <<
                ") with score " << best_result.score << std::endl;
#endif

        // 提前发现胜利：如果找到必胜局(连五)，就不再搜索更深层
        const int WIN_SCORE = 5000000;
        if (ai_side == 0 && best_result.score >= WIN_SCORE) {
#ifdef DEBUG_MODE
            std::cerr << "Found winning move for BLACK at depth " << depth << ". Stopping search." << std::endl;
#endif
            break;
        }
        if (ai_side == 1 && best_result.score <= -WIN_SCORE) {
#ifdef DEBUG_MODE
            std::cerr << "Found winning move for WHITE at depth " << depth << ". Stopping search." << std::endl;
#endif
            break;
        }

        // 加入超时判断，如果时间快用完了，就跳出循环，返回当前找到的最佳结果（好像没用？考察9,7  5,4）
        clock_t end_time = clock();
        if (double(end_time - start_time) / CLOCKS_PER_SEC > 4.5) {
#ifdef DEBUG_MODE
            std::cerr << "时间不够，跳出迭代加深" << std::endl;
#endif
            break;
        }
    }

    // 返回从最深已完成的搜索中找到的最佳走法
    return best_result;

#else
    // 如果不使用迭代加深，则直接调用最大深度搜索
    return Minimax(2);
#endif
}


// 优化的迭代加深 Minimax
MinimaxResult deepingMinimax() {
#ifdef deeping

    clock_t start_time = clock();

    MinimaxResult best_result = {{-1, -1}, 0};

    for (int depth = 2; depth <= DEP; depth += 2) {
        auto current_result = Minimax(depth);

        // 无论如何，都使用更深层次搜索的结果来更新最佳走法
        best_result = current_result;

#ifdef DEBUG_MODE
        std::cerr << "Depth " << depth << " best move: (" << best_result.move.first << "," << best_result.move.second <<
                ") with score " << best_result.score << std::endl;
#endif

        // 提前发现胜利：如果找到必胜局(连五)，就不再搜索更深层
        const int WIN_SCORE = 5000000;
        if (ai_side == 0 && best_result.score >= WIN_SCORE) {
#ifdef DEBUG_MODE
            std::cerr << "Found winning move for BLACK at depth " << depth << ". Stopping search." << std::endl;
#endif
            break;
        }
        if (ai_side == 1 && best_result.score <= -WIN_SCORE) {
#ifdef DEBUG_MODE
            std::cerr << "Found winning move for WHITE at depth " << depth << ". Stopping search." << std::endl;
#endif
            break;
        }

        // 加入超时判断，如果时间快用完了，就跳出循环，返回当前找到的最佳结果（好像没用？考察9,7  5,4）
        clock_t end_time = clock();
        if (double(end_time - start_time) / CLOCKS_PER_SEC > 4.5) {
#ifdef DEBUG_MODE
            std::cerr << "时间不够，跳出迭代加深" << std::endl;
#endif
            break;
        }
    }

    // 返回从最深已完成的搜索中找到的最佳走法
    return best_result;

#else
    // 如果不使用迭代加深，则直接调用最大深度搜索
    return Minimax(DEP);
#endif
}

MinimaxResult Minimax(int depth) {
    int alpha = -INF;
    int beta = INF;
    std::pair<int, int> res = {-1, -1};


    auto moves = generate_sorted_moves(ai_side, DEP);

    //如果我的ai执黑子，我想让结果最大
    if (ai_side == 0) {
        int val = -INF;
        for (const auto &move: moves) {
            int i = move.first, j = move.second;
#ifdef mizi
            int score_before = update_score_for_position(i, j);
#endif
            board[i][j] = BLACK; //落子
#ifdef mizi
            int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
            current_total_score += (score_after - score_before);
#endif
            update_hash(i, j, BLACK);
            int move_value = min_value(alpha, beta, depth - 1); //调用min
            board[i][j] = EMPTY; //撤销
            update_hash(i, j, BLACK);
#ifdef mizi
            current_total_score -= (score_after - score_before);
#endif
            if (move_value > val) {
                val = move_value;
                res = {i, j};
                alpha = std::max(alpha, val);
            }
            if (alpha >= beta) {
                break; // Beta剪枝
            }
            if (alpha >= 10000000) {
                break; //必胜剪枝
            }
#ifdef DEBUG_MODE
            std::cerr << i << " " << j << " score: " << move_value << std::endl;
#endif
        }
        return {res, val};
    } else {
        //如果我的ai执白子，我想让结果最小
        int val = INF;
        for (const auto &move: moves) {
            int i = move.first, j = move.second;
#ifdef mizi
            int score_before = update_score_for_position(i, j);
#endif
            board[i][j] = WHITE; //落子
#ifdef mizi
            int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
            current_total_score += (score_after - score_before);
#endif
            update_hash(i, j, WHITE);
            int move_value = max_value(alpha, beta, depth - 1); //调用min
            board[i][j] = EMPTY; //撤销

            update_hash(i, j, WHITE);
#ifdef mizi
            current_total_score -= (score_after - score_before);
#endif
            if (move_value < val) {
                val = move_value;
                res = {i, j};
                beta = std::min(beta, val);
            }
            if (alpha >= beta) {
                break; // Alpha剪枝
            }
            if (alpha <= -10000000) {
                break; //必胜剪枝
            }
#ifdef DEBUG_MODE
            std::cerr << i << " " << j << " score: " << move_value << std::endl;
#endif
        }
        return {res, val};
    }
}

int min_value(int alpha, int beta, int depth) {
    int ter = terminal();
    if (depth == 0 || ter != 2) {
        return evaluate(ter == EMPTY ? 2 : ter); //评估当前棋局的分数，当前该白棋落子
    }

    auto it = transposition_table.find(current_hash);
    if (it != transposition_table.end() && it->second.depth >= depth) {
        TTEntry entry = it->second;
        if (entry.flag == EXACT) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
        if (entry.flag == LOWER_BOUND)
            alpha = std::max(alpha, entry.score);
        else if (entry.flag == UPPER_BOUND)
            beta = std::min(beta, entry.score);
        if (alpha >= beta) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
    }


    int val = INF;
    int original_beta = beta;
    auto moves = generate_sorted_moves(WHITE, depth);
    for (const auto &move: moves) {
        int i = move.first, j = move.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[i][j] = WHITE; //落子
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
        update_hash(i, j, WHITE);
        val = std::min(val, max_value(alpha, beta, depth - 1)); //调用max
        board[i][j] = EMPTY; //撤销
#ifdef mizi
        current_total_score -= (score_after - score_before);
#endif
        update_hash(i, j, WHITE);
        if (val <= alpha) {
            return val;
        } //alpha-beta剪枝
        if (val <= -10000000) {
            return val; //必胜剪枝
        }
        beta = std::min(beta, val);
    }

    // --- Zobrist 存储 ---
    TTEntry entry;
    entry.score = val;
    entry.depth = depth;
    if (val >= original_beta) {
        entry.flag = LOWER_BOUND;
    } else if (val <= alpha) {
        entry.flag = UPPER_BOUND;
    } else {
        entry.flag = EXACT;
    }
    transposition_table[current_hash] = entry;
    // --- Zobrist 存储结束 ---


    return val;
}

int max_value(int alpha, int beta, int depth) {
    int ter = terminal();
    if (depth == 0 || ter != 2) {
        return evaluate(ter == EMPTY ? 2 : ter); //评估当前棋局的分数，当前该黑棋落子
    }

    // --- Zobrist 查询 ---
    auto it = transposition_table.find(current_hash);
    if (it != transposition_table.end() && it->second.depth >= depth) {
        TTEntry entry = it->second;
        if (entry.flag == EXACT) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
        if (entry.flag == LOWER_BOUND)
            alpha = std::max(alpha, entry.score);
        else if (entry.flag == UPPER_BOUND)
            beta = std::min(beta, entry.score);
        if (alpha >= beta) {
            // std::cerr << "Zobrist is used !  ";
            return entry.score;
        }
    }
    // --- Zobrist 查询结束 ---


    int val = -INF;
    int original_alpha = alpha;
    auto moves = generate_sorted_moves(BLACK, depth);
    for (const auto &move: moves) {
        int i = move.first, j = move.second;
#ifdef mizi
        int score_before = update_score_for_position(i, j);
#endif
        board[i][j] = BLACK; //落子
#ifdef mizi
        int score_after = update_score_for_position(i, j);
#ifdef pos_val
            if(board[i][j] == BLACK){
                score_after += positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }else{
                score_after -= positional_weights[i][j] * POSITION_WEIGHT_FACTOR;
            }
#endif
        current_total_score += (score_after - score_before);
#endif
        update_hash(i, j, BLACK);
        val = std::max(val, min_value(alpha, beta, depth - 1)); //调用max
        board[i][j] = EMPTY; //撤销
#ifdef mizi
        current_total_score -= (score_after - score_before);
#endif
        update_hash(i, j, BLACK);
        if (val >= beta) {
            return val;
        } //alpha-beta剪枝
        if (val >= 10000000) {
            return val; //必胜剪枝
        }
        alpha = std::max(alpha, val);
    }


    // --- Zobrist 存储 ---
    TTEntry entry;
    entry.score = val;
    entry.depth = depth;
    if (val <= original_alpha) {
        entry.flag = UPPER_BOUND;
    } else if (val >= beta) {
        entry.flag = LOWER_BOUND;
    } else {
        entry.flag = EXACT;
    }
    transposition_table[current_hash] = entry;
    // --- Zobrist 存储结束 ---

    return val;
}

