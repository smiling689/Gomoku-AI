#include "vcx.h"
#include<iostream>
#include <utility>
#include<vector>

extern int board[15][15];
extern int ai_side;
const int VCX_DEP = 12;//算杀深度接口
extern int winner();
extern std::vector<std::pair<int, int>> generate_moves();
extern int score_move(int r, int c, int color); 

enum Cell {
    EMPTY = -1,
    BLACK = 0,
    WHITE = 1
};

std::pair<int , int> vcf(int dep){
    std::pair<int , int> ans = {-1 , -1};
    auto moves = generate_moves();
    int opponent_side = 1 - ai_side;
    // std::vector<int> 
    //不能这么直接！我在寻找自己的冲四的时候也需要考虑防守。如果这个时候必须防守（例如，我冲四进攻，对手防住并且同时造成冲四，那我就必须要先防守
    for(auto move : moves){
        int r = move.first , c = move.second;
        board[r][c] = ai_side;
        

    }
    
    
    
    return ans;
}

std::pair<int , int> vct(int dep){
    return {-1 , -1};
}

std::pair<int , int> find_victory(){
    auto vcf_path = vcf(VCX_DEP);
    if(vcf_path.first != -1){
        return vcf_path;
    }//如果vcf可以直接算杀，则使用vcf走

    auto vct_path = vct(VCX_DEP);
    if(vct_path.first != -1){
        return vct_path;
    }//如果vct可以算杀，则使用vct

    return {-1 , -1};
}