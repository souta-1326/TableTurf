#pragma once
#include "card_database.hpp"
#include <bitset>
#include <iostream>
template<int ID,int H,int W,int N_square> class Stage{
public:
  static constexpr void initialize(const bool exists_square[H][W],const int my_starting_pos_H,const int my_starting_pos_W,const int opponent_starting_pos_H,const int opponent_starting_pos_W);
  //BoardクラスでH,W,N_squareを利用できるようにするため
  static constexpr int h = H;
  static constexpr int w = W;
  static constexpr int n_square = N_square;
  
  //各カードがステージ外へはみ出さない位置での(カードの向き,左上の行,左上の列)
  //カードの向き R0:0,R90:1,R180:2,R270:3
  //N_square*4は理論上の置き方の方法の数の最大値
  static int card_status_size[N_card+1];
  static std::tuple<int,int,int> card_status[N_card+1][N_square*4];
  //各カードの通常マスが覆うマス目
  static std::bitset<N_square> card_covered_square_normal[N_card+1][N_square*4];
  //各カードのSPマスが覆うマス目
  //N=320程度であればランダムアクセスよりもbit演算の方が高速
  static std::bitset<N_square> card_covered_square_SP[N_card+1][N_square*4];
  //exists_square[i][j]:i行目j列目にマスがあるか(0-indexed)
  static bool exists_square[H][W];
  //自分、相手の最初のSPマスがそれぞれ何行目何列目にあるか
  static int my_starting_pos_H,my_starting_pos_W,opponent_starting_pos_H,opponent_starting_pos_W;
  //各マスが左上から何番目にあるか(0-indexed)(ない場合は-1)
  static int place_to_order[H][W];
  //左上からi番目にあるマスの位置(0-indexed)
  static std::pair<int,int> order_to_place[N_square];
private:
  static constexpr void order_setting();
  static constexpr bool card_is_on_board_R0(int card_id,int card_pos_H,int card_pos_W);
  static constexpr bool card_is_on_board_R90(int card_id,int card_pos_H,int card_pos_W);
  static constexpr bool card_is_on_board_R180(int card_id,int card_pos_H,int card_pos_W);
  static constexpr bool card_is_on_board_R270(int card_id,int card_pos_H,int card_pos_W);
  static constexpr void card_to_bitset_R0(int card_id,int card_pos_H,int card_pos_W);
  static constexpr void card_to_bitset_R90(int card_id,int card_pos_H,int card_pos_W);
  static constexpr void card_to_bitset_R180(int card_id,int card_pos_H,int card_pos_W);
  static constexpr void card_to_bitset_R270(int card_id,int card_pos_H,int card_pos_W);
  static constexpr void card_setting();
};
//static変数の初期化
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::card_status_size[N_card+1] = {};
template<int ID,int H,int W,int N_square> std::tuple<int,int,int> Stage<ID,H,W,N_square>::card_status[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_covered_square_normal[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_covered_square_SP[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> bool Stage<ID,H,W,N_square>::exists_square[H][W] = {};
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::my_starting_pos_H = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::my_starting_pos_W = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::opponent_starting_pos_H = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::opponent_starting_pos_W = 0;
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::place_to_order[H][W] = {};
template<int ID,int H,int W,int N_square> std::pair<int,int> Stage<ID,H,W,N_square>::order_to_place[N_square] = {};
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::order_setting(){
  int order_itr = 0;
  for(int i=0;i<H;i++){
    for(int j=0;j<W;j++){
      if(exists_square[i][j]){
        order_to_place[order_itr] = {i,j};
        place_to_order[i][j] = order_itr;
        order_itr++;
      }
      else{
        place_to_order[i][j] = -1;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr bool Stage<ID,H,W,N_square>::card_is_on_board_R0(int card_id,int card_pos_H,int card_pos_W){
  for(int i=0;i<cards[card_id].H;i++){
    for(int j=0;j<cards[card_id].W;j++){
      if(cards[card_id].R0[i][j] && !exists_square[card_pos_H+i][card_pos_W+j]){
        return false;
      }
    }
  }
  return true;
}
template<int ID,int H,int W,int N_square> constexpr bool Stage<ID,H,W,N_square>::card_is_on_board_R90(int card_id,int card_pos_H,int card_pos_W){
  for(int i=0;i<cards[card_id].W;i++){
    for(int j=0;j<cards[card_id].H;j++){
      if(cards[card_id].R90[i][j] && !exists_square[card_pos_H+i][card_pos_W+j]){
        return false;
      }
    }
  }
  return true;
}
template<int ID,int H,int W,int N_square> constexpr bool Stage<ID,H,W,N_square>::card_is_on_board_R180(int card_id,int card_pos_H,int card_pos_W){
  for(int i=0;i<cards[card_id].H;i++){
    for(int j=0;j<cards[card_id].W;j++){
      if(cards[card_id].R180[i][j] && !exists_square[card_pos_H+i][card_pos_W+j]){
        return false;
      }
    }
  }
  return true;
}
template<int ID,int H,int W,int N_square> constexpr bool Stage<ID,H,W,N_square>::card_is_on_board_R270(int card_id,int card_pos_H,int card_pos_W){
  for(int i=0;i<cards[card_id].W;i++){
    for(int j=0;j<cards[card_id].H;j++){
      if(cards[card_id].R270[i][j] && !exists_square[card_pos_H+i][card_pos_W+j]){
        return false;
      }
    }
  }
  return true;
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_R0(int card_id,int card_pos_H,int card_pos_W){
  std::bitset<N_square> &target_normal = card_covered_square_normal[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_SP = card_covered_square_SP[card_id][card_status_size[card_id]-1];
  for(int i=0;i<cards[card_id].H;i++){
    for(int j=0;j<cards[card_id].W;j++){
      if(cards[card_id].R0[i][j]){
        assert(exists_square[card_pos_H+i][card_pos_W+j]);
        //(i,j)がSPマスならtarget_SP,通常マスならtarget_normal
        (i==cards[card_id].SP_H && j==cards[card_id].SP_W ? target_SP:target_normal)[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_R90(int card_id,int card_pos_H,int card_pos_W){
  std::bitset<N_square> &target_normal = card_covered_square_normal[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_SP = card_covered_square_SP[card_id][card_status_size[card_id]-1];
  for(int i=0;i<cards[card_id].W;i++){
    for(int j=0;j<cards[card_id].H;j++){
      if(cards[card_id].R90[i][j]){
        assert(exists_square[card_pos_H+i][card_pos_W+j]);
        (i==cards[card_id].SP_H && j==cards[card_id].SP_W ? target_SP:target_normal)[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_R180(int card_id,int card_pos_H,int card_pos_W){
  std::bitset<N_square> &target_normal = card_covered_square_normal[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_SP = card_covered_square_SP[card_id][card_status_size[card_id]-1];
  for(int i=0;i<cards[card_id].H;i++){
    for(int j=0;j<cards[card_id].W;j++){
      if(cards[card_id].R180[i][j]){
        assert(exists_square[card_pos_H+i][card_pos_W+j]);
        (i==cards[card_id].SP_H && j==cards[card_id].SP_W ? target_SP:target_normal)[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_R270(int card_id,int card_pos_H,int card_pos_W){
  std::bitset<N_square> &target_normal = card_covered_square_normal[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_SP = card_covered_square_SP[card_id][card_status_size[card_id]-1];
  for(int i=0;i<cards[card_id].W;i++){
    for(int j=0;j<cards[card_id].H;j++){
      if(cards[card_id].R270[i][j]){
        assert(exists_square[card_pos_H+i][card_pos_W+j]);
        (i==cards[card_id].SP_H && j==cards[card_id].SP_W ? target_SP:target_normal)[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_setting(){
  for(int i=1;i<=N_card;i++){
    //R0
    for(int j=0;j<=H-cards[i].H;j++){
      for(int k=0;k<=W-cards[i].W;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board_R0(i,j,k)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {0,j,k};
          card_to_bitset_R0(i,j,k);
        }
      }
    }
    //R90
    for(int j=0;j<=H-cards[i].W;j++){
      for(int k=0;k<=W-cards[i].H;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board_R90(i,j,k)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {1,j,k};
          card_to_bitset_R90(i,j,k);
        }
      }
    }
    //R180
    for(int j=0;j<=H-cards[i].H;j++){
      for(int k=0;k<=W-cards[i].W;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board_R180(i,j,k)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {2,j,k};
          card_to_bitset_R180(i,j,k);
        }
      }
    }
    //R270
    for(int j=0;j<=H-cards[i].W;j++){
      for(int k=0;k<=W-cards[i].H;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board_R270(i,j,k)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {3,j,k};
          card_to_bitset_R270(i,j,k);
        }
      }
    }
  }
}
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
  order_setting();
  card_setting();
}