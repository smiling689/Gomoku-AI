#ifndef VCX_H
#define VCX_H

#include<utility>
#include <set>

extern int board[15][15];
extern const int VCX_DEP;

extern bool is_valid(int r , int c);

std::pair<int , int> find_victory(int dep);

std::pair<int, int> find_victory_sequence(int max_depth);

std::pair<int , int> vcf(int dep);

std::pair<int , int> vct(int dep);

struct VCXResult {
    int score;
    std::pair<int, int> step;
};

VCXResult vcx_search(int depth);

#endif // VCX_H


