#ifndef MY_EVAL_H
#define MY_EVAL_H

extern int board[15][15];

int evaluate(int ter);


int winner();

int score_move(int r, int c, int color); 

#endif // MY_EVAL_H