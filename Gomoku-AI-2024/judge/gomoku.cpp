// g++ judge/gomoku.cpp judge/my_eval.cpp judge/flip.cpp judge/zobrist.cpp -o gomoku           编译
// python judge/judge.py ./gomoku ./baseline                                                   时，我的ai是ai0(先手，黑棋)
// python judge/judge.py ./baseline ./gomoku                                                   时，我的ai是ai1(后手，白棋)
#include "AIController.h"
#include "my_eval.h"
#include "zobrist.h"
#include "flip.h"
#include <utility>
#include <cstring>
#include <climits>
#include <vector>
#include <algorithm>

extern int ai_side; //0: black, 1: white
//ai_side == 0：代表AI执黑棋。
//ai_side == 1：代表AI执白棋。
//对手是 1-ai_side
std::string ai_name = "Smiling_AI";

int turn = 0;
int board[15][15];
const int INF = INT_MAX;
const int DEP = 6;//depth接口，表示搜索深度

enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

struct MoveScore {
    std::pair<int, int> move;
    int score;
};

std::pair<int , int> Minimax();

int min_value(int alpha , int beta , int depth);

int max_value(int alpha , int beta , int depth);

int winner();

void init() {
    transposition_table.clear(); 
    memset(board, EMPTY, sizeof(board));
    init_zobrist();
    // srand(time(NULL));
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.

std::pair<int, int> getRandom() {
    while (true) {
        int x = rand() % 15;
        int y = rand() % 15;
        if (board[x][y] == EMPTY) {
            board[x][y] = ai_side;
            return std::make_pair(x, y);
        }
    }   
}

bool is_valid(int r, int c) {
    return r >= 0 && r < 15 && c >= 0 && c < 15;
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

std::vector<std::pair<int, int>> generate_moves() {
    std::vector<std::pair<int, int>> moves;
    bool visited[15][15] = {false}; 

    for (int r = 0; r < 15; ++r) {
        for (int c = 0; c < 15; ++c) {
            // 只考虑已有棋子周围的空点
            if (board[r][c] == EMPTY && adj(r, c)) {
                if (!visited[r][c]) {
                    moves.push_back({r, c});
                    visited[r][c] = true;
                }
            }
        }
    }
    return moves;
}

std::vector<std::pair<int, int>> generate_sorted_moves(int current_player) {
    auto moves = generate_moves(); 
    if (moves.empty()) {
        return {};
    }

    std::vector<MoveScore> scored_moves;
    int opponent_player = 1 - current_player;

    for (const auto& move : moves) {
        // 评估每个点
        // 分数 = 我方在此落子的进攻分 + 对方在此落子的防守分
        int offensive_score = score_move(move.first, move.second, current_player);
        int defensive_score = score_move(move.first, move.second, opponent_player);
        scored_moves.push_back({move, offensive_score + defensive_score});
    }

    // 按分数从高到低排序
    std::sort(scored_moves.begin(), scored_moves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.score > b.score;
    });

    // 提取排序后的坐标
    std::vector<std::pair<int, int>> sorted_moves;
    for (const auto& scored_move : scored_moves) {
        sorted_moves.push_back(scored_move.move);
    }

    return sorted_moves;
}

// 一个辅助函数，用来计算棋盘上的棋子数
int count_black() {
    int count = 0;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (board[i][j] == BLACK) {
                count++;
            }
        }
    }
    return count;
}

int count_white() {
    int count = 0;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (board[i][j] == WHITE) {
                count++;
            }
        }
    }
    return count;
}

void flip_board(){
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (board[i][j] == BLACK) {
                board[i][j] = WHITE;
            }
            else if (board[i][j] == WHITE) {
                board[i][j] = BLACK;
            }
        }
    }
}

std::pair<int, int> action(std::pair<int, int> loc) {
    if (loc.first != -1 && loc.second != -1) {
        board[loc.first][loc.second] = 1 - ai_side;
        update_hash(loc.first, loc.second, 1 - ai_side);
    }

    // 2. 根据棋盘状态，计算当前是第几手棋
    // 已经下了 stone_count 手，现在轮到我们下第 stone_count + 1 手
    int black = count_black();
    int white = count_white();
    int current_turn = black + white + 1;

    std::cerr << "current_turn is " << current_turn << std::endl ; 

    // --- 回合 1: 我方是先手 (黑棋) ---
    if (current_turn == 1) {
        auto random = getRandom(); 
        board[random.first][random.second] = ai_side;
        update_hash(random.first, random.second, ai_side);
        return random;
    }

    // --- 回合 2: 我方是后手 (白棋)，决定是否交换 ---
    if (ai_side == WHITE && black == 2 && white == 1) {
        int no_flip = no_flip_score();
        int flip = flip_score();
        if (flip < no_flip) {
            std::cerr << "Smiling_AI: Flip is better. Choosing to SWAP." << std::endl;
            // 确认要换手，执行真正的翻转并返回
            flip_board();
            current_hash = calculate_hash();
            return {-1, -1};
        } else {
            std::cerr << "Smiling_AI: Not swapping is better. Playing regular move." << std::endl;
        }
    }

    if (ai_side == BLACK && black == 2 && white == 1 && loc.first == -1) {
        std::cerr << "Smiling_AI: Opponent swapped. " << std::endl;
        flip_board();
        current_hash = calculate_hash();
    }

    // transposition_table.clear(); 

    // 4. 对于所有常规回合，使用 Minimax 算法计算最佳落子
    std::pair<int, int> my_move = Minimax();
    if (my_move.first != -1 && my_move.second != -1) {
        board[my_move.first][my_move.second] = ai_side;
        update_hash(my_move.first, my_move.second, ai_side);
    }else{
        std::cerr << "MiniMax wrong! -1,-1" << std::endl;
        // std::throw runtime_error("Minimax_wrong");
    }

    return my_move;
}


//terminal用于判断当前棋局是否已经结束，-1表示平局，0表示黑子胜，1表示白子胜，2表示未结束
int terminal(){
    int win = winner();
    if(win != EMPTY){
        return win;
    }//如果已经决出胜者了，返回胜者
    for(int i = 0 ; i <= 14 ; i++){
        for(int j = 0 ; j <= 14 ; j++){
            if(board[i][j] == EMPTY){
                return 2;
            }
        }
    }//如果所有格子都满了，也返回empty（平局）
    return EMPTY;
}

std::pair<int , int> Minimax(){
    int alpha = -INF;
    int beta = INF;
    std::pair<int , int> res = {-1 , -1};
    int depth = DEP;

    auto moves = generate_sorted_moves(ai_side);
    //如果我的ai执黑子，我想让结果最大
    if(ai_side == 0){
        int val = -INF;
        for (const auto& move : moves) {
            int i = move.first , j = move.second;
            board[i][j] = BLACK;//落子
            update_hash(i, j, BLACK);
            int move_value = min_value(alpha , beta , depth - 1);//调用min
            board[i][j] = EMPTY;//撤销
            update_hash(i, j, BLACK);
            if(move_value > val){
                val = move_value;
                res = {i , j};
                alpha = std::max(alpha , val);
            }
            if (alpha >= beta) {
                break; // Beta剪枝
            }
            std::cerr << i << " " << j << " score: " << move_value << std::endl;
        }
        return res;
    }else{
        //如果我的ai执白子，我想让结果最小
        int val = INF;
        for (const auto& move : moves) {
            int i = move.first , j = move.second;
            board[i][j] = WHITE;//落子
            update_hash(i, j, WHITE);
            int move_value = max_value(alpha , beta , depth - 1);//调用min
            board[i][j] = EMPTY;//撤销
            update_hash(i, j, WHITE);
            if(move_value < val){
                val = move_value;
                res = {i , j};
                beta = std::min(beta , val);
            }
            if (alpha >= beta) {
                break; // Alpha剪枝
            }
            std::cerr << i << " " << j << " score: " << move_value << std::endl;
        }
        return res;
    }
}

int min_value(int alpha , int beta , int depth){
    int ter = terminal();
    if(depth == 0 || ter != 2){
        return evaluate(ter == EMPTY ? 2 : ter);//评估当前棋局的分数，当前该白棋落子
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
    auto moves = generate_sorted_moves(ai_side);
    for (const auto& move : moves){
        int i = move.first , j = move.second;
        board[i][j] = WHITE;//落子
        update_hash(i, j, WHITE);
        val = std::min(val , max_value(alpha , beta , depth - 1));//调用max
        board[i][j] = EMPTY;//撤销
        update_hash(i, j, WHITE);
        if(val <= alpha){
            return val;
        }//alpha-beta剪枝
        beta = std::min(beta , val);
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

int max_value(int alpha , int beta , int depth){
    int ter = terminal();
    if(depth == 0 || ter != 2){
        return evaluate(ter == EMPTY ? 2 : ter);//评估当前棋局的分数，当前该黑棋落子
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
    auto moves = generate_sorted_moves(ai_side);
    for (const auto& move : moves) {
        int i = move.first , j = move.second;
        board[i][j] = BLACK;//落子
        update_hash(i, j, BLACK);
        val = std::max(val , min_value(alpha , beta , depth - 1));//调用max
        board[i][j] = EMPTY;//撤销
        update_hash(i, j, BLACK);
        if(val >= beta){
            return val;
        }//alpha-beta剪枝
        alpha = std::max(alpha , val);
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