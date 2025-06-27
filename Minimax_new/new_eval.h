#ifndef OTHER_EVAL_H
#define OTHER_EVAL_H

#include<vector>

extern int board[15][15];

std::vector<std::pair<int , int> > generate_sorted_moves(int num) ;

void init_eval();

int score_move(int r , int c);

int evaluate(int color);

int winner();

#endif // OTHER_EVAL_H