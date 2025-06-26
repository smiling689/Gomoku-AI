#ifndef MY_EVAL_HASH_H
#define MY_EVAL_HASH_H

#include<vector>
#include<unordered_map>
// #define mizi

extern int board[15][15];
extern long long current_total_score;


void init_pattern_db();

int evaluate(int ter);

int winner();

int score_move(int r, int c, int color); 

bool victory(int r , int c , int color);

std::vector<int> chong_4(int r , int c , int color);

void recalculate_full_board_score();

int update_score_for_position(int r, int c);

int update_score_for_position_1(int r, int c);

#endif // MY_EVAL_HASH_H