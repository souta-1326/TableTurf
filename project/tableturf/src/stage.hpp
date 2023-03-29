#pragma once
#include "card_database.hpp"
#include <bitset>
template<int ID,int H,int W,int N_square> class Stage{
public:
  static constexpr void initialize(const bool exists_square[H][W],const int my_starting_pos_H,const int my_starting_pos_W,const int opponent_starting_pos_H,const int opponent_starting_pos_W);
private:
  //exists_square[i][j]:i行目j列目にマスがあるか(0-indexed)
  static bool exists_square[H][W];
  //自分、相手の最初のSPマスがそれぞれ何行目何列目にあるか
  static int my_starting_pos_H,my_starting_pos_W,opponent_starting_pos_H,opponent_starting_pos_W;
};
//static変数の初期化
template<int ID,int H,int W,int N_square> bool Stage<ID,H,W,N_square>::exists_square[H][W] = {};
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::my_starting_pos_H = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::my_starting_pos_W = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::opponent_starting_pos_H = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::opponent_starting_pos_W = 0;
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::initialize(const bool exists_square[H][W],const int my_starting_pos_H,const int my_starting_pos_W,const int opponent_starting_pos_H,const int opponent_starting_pos_W){
  for(int i=0;i<H;i++){
    for(int j=0;j<W;j++){
      Stage<ID,H,W,N_square>::exists_square[i][j] = exists_square[i][j];
    }
  }
  Stage<ID,H,W,N_square>::my_starting_pos_H = my_starting_pos_H;
  Stage<ID,H,W,N_square>::my_starting_pos_W = my_starting_pos_W;
  Stage<ID,H,W,N_square>::opponent_starting_pos_H = opponent_starting_pos_H;
  Stage<ID,H,W,N_square>::opponent_starting_pos_W = opponent_starting_pos_W;
}