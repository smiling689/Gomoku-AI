#ifndef VCX_H
#define VCX_H

#include<utility>

extern int board[15][15];
extern const int VCX_DEP;

extern bool is_valid(int r , int c);


std::pair<int , int> find_victory(int dep);

std::pair<int , int> vcf(int dep);

std::pair<int , int> vct(int dep);

#endif // VCX_H