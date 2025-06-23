#ifndef FLIP_H
#define FLIP_H

#include <utility>

extern int board[15][15];

extern std::pair<int , int> deepingMinimax();

extern int evaluate(int ter);

extern void flip_board();

int flip_score() ;

int no_flip_score();

#endif // FLIP_H