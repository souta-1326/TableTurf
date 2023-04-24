#pragma once
#include "card_database.hpp"
#include <bitset>
#include <iostream>
#include <utility>
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
  //向きと左上の位置からidを復元(ステージからはみ出す場合は-1)
  static int card_direction_and_place_to_id[N_card+1][4][H][W];
  //各カードのマスが覆うマス目
  static std::bitset<N_square> card_covered_square[N_card+1][N_square*4];
  //各カードの通常マスが覆うマス目
  static std::bitset<N_square> card_covered_square_normal[N_card+1][N_square*4];
  //各カードのSPマスが覆うマス目
  //N=320程度であればランダムアクセスで1bitを変更するよりもbit演算の方が高速
  static std::bitset<N_square> card_covered_square_SP[N_card+1][N_square*4];
  //各カードを周囲8方向に移動した先のマス目(SPポイントが溜まったかの判定に用いる)
  static std::bitset<N_square> card_shifted_square[N_card+1][N_square*4][8];
  //各カードのいずれかのマスの周囲にあるマス目
  static std::bitset<N_square> card_around_square[N_card+1][N_square*4];
  //is_there_a_block_nearbyの初期値
  static std::bitset<N_square> is_there_a_block_nearby_default[8];
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
  static constexpr bool card_is_on_board(int card_id,int card_pos_H,int card_pos_W,int direction);
  static constexpr void card_to_bitset(int card_id,int card_pos_H,int card_pos_W,int direction);
  static constexpr void card_to_bitset_shifted(int card_id,int card_pos_H,int card_pos_W,int direction);
  static constexpr void card_to_bitset_around(int card_id,int card_pos_H,int card_pos_W,int direction);
  static constexpr void card_setting();
  static constexpr void board_default_setting();
};
//static変数の初期化
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::card_status_size[N_card+1] = {};
template<int ID,int H,int W,int N_square> std::tuple<int,int,int> Stage<ID,H,W,N_square>::card_status[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> int Stage<ID,H,W,N_square>::card_direction_and_place_to_id[N_card+1][4][H][W] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_covered_square[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_covered_square_normal[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_covered_square_SP[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_shifted_square[N_card+1][N_square*4][8] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::card_around_square[N_card+1][N_square*4] = {};
template<int ID,int H,int W,int N_square> std::bitset<N_square> Stage<ID,H,W,N_square>::is_there_a_block_nearby_default[8] = {};
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
template<int ID,int H,int W,int N_square> constexpr bool Stage<ID,H,W,N_square>::card_is_on_board(int card_id,int card_pos_H,int card_pos_W,int direction){
  //現在のカードの向きでのカードの縦と横の長さ
  const int current_card_H = ((direction==0 || direction==2) ? cards[card_id].H:cards[card_id].W);
  const int current_card_W = ((direction==0 || direction==2) ? cards[card_id].W:cards[card_id].H);
  const auto card_exists_square = (direction==0 ? cards[card_id].R0:direction==1 ? cards[card_id].R90:direction==2 ? cards[card_id].R180:cards[card_id].R270);
  for(int i=0;i<current_card_H;i++){
    for(int j=0;j<current_card_W;j++){
      if(card_exists_square[i][j] && !exists_square[card_pos_H+i][card_pos_W+j]){
        return false;
      }
    }
  }
  return true;
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset(int card_id,int card_pos_H,int card_pos_W,int direction){
  //現在のカードの向きでのカードの縦と横の長さ
  const int current_card_H = ((direction==0 || direction==2) ? cards[card_id].H:cards[card_id].W);
  const int current_card_W = ((direction==0 || direction==2) ? cards[card_id].W:cards[card_id].H);
  const auto card_exists_square = (direction==0 ? cards[card_id].R0:direction==1 ? cards[card_id].R90:direction==2 ? cards[card_id].R180:cards[card_id].R270);
  const int card_SP_H = (direction==0 ? cards[card_id].R0_SP_H:direction==1 ? cards[card_id].R90_SP_H:direction==2 ? cards[card_id].R180_SP_H:cards[card_id].R270_SP_H);
  const int card_SP_W = (direction==0 ? cards[card_id].R0_SP_W:direction==1 ? cards[card_id].R90_SP_W:direction==2 ? cards[card_id].R180_SP_W:cards[card_id].R270_SP_W);
  //変更したいbitset
  std::bitset<N_square> &target = card_covered_square[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_normal = card_covered_square_normal[card_id][card_status_size[card_id]-1];
  std::bitset<N_square> &target_SP = card_covered_square_SP[card_id][card_status_size[card_id]-1];
  for(int i=0;i<current_card_H;i++){
    for(int j=0;j<current_card_W;j++){
      if(card_exists_square[i][j]){
        assert(exists_square[card_pos_H+i][card_pos_W+j]);
        //targetはマスの種類関係なし
        target[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
        //(i,j)がSPマスならtarget_SP,通常マスならtarget_normal
        (i==card_SP_H && j==card_SP_W ? target_SP:target_normal)[place_to_order[card_pos_H+i][card_pos_W+j]] = true;
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_shifted(int card_id,int card_pos_H,int card_pos_W,int direction){
  //現在のカードの向きでのカードの縦と横の長さ
  const int current_card_H = ((direction==0 || direction==2) ? cards[card_id].H:cards[card_id].W);
  const int current_card_W = ((direction==0 || direction==2) ? cards[card_id].W:cards[card_id].H);
  const auto card_exists_square = (direction==0 ? cards[card_id].R0:direction==1 ? cards[card_id].R90:direction==2 ? cards[card_id].R180:cards[card_id].R270);
  //変更したいbitsetの配列
  auto target = card_shifted_square[card_id][card_status_size[card_id]-1];
  for(int i=0;i<current_card_H;i++){
    for(int j=0;j<current_card_W;j++){
      if(card_exists_square[i][j]){
        constexpr std::pair<int,int> cordinate_diff[8] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
        for(int k=0;k<8;k++){
          //座標(ni,nj)から見て(i,j)はcordinate_diff[k]平行移動した位置にある
          int ni = card_pos_H+i-cordinate_diff[k].first,nj = card_pos_W+j-cordinate_diff[k].second;
          if(0 <= ni && ni < H && 0 <= nj && nj < W && exists_square[ni][nj]){
            //(ni,nj)から見てcordinate_diff[k]方向にマスがあることを意味する
            target[k][place_to_order[ni][nj]] = true;
          }
        }
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_to_bitset_around(int card_id,int card_pos_H,int card_pos_W,int direction){
  //現在のカードの向きでのカードの縦と横の長さ
  const int current_card_H = ((direction==0 || direction==2) ? cards[card_id].H:cards[card_id].W);
  const int current_card_W = ((direction==0 || direction==2) ? cards[card_id].W:cards[card_id].H);
  const auto card_exists_square = (direction==0 ? cards[card_id].R0:direction==1 ? cards[card_id].R90:direction==2 ? cards[card_id].R180:cards[card_id].R270);
  //変更したいbitset
  std::bitset<N_square> &target = card_around_square[card_id][card_status_size[card_id]-1];
  for(int i=0;i<current_card_H;i++){
    for(int j=0;j<current_card_W;j++){
      if(card_exists_square[i][j]){
        constexpr std::pair<int,int> cordinate_diff[8] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
        for(auto [diff_i,diff_j]:cordinate_diff){
          //座標(i,j)から見て(ni,nj)はcordinate_diff[k]平行移動した位置にある
          int ni = card_pos_H+i+diff_i,nj = card_pos_W+j+diff_j;
          if(0 <= ni && ni < H && 0 <= nj && nj < W && exists_square[ni][nj]){
            target[place_to_order[ni][nj]] = true;
          }
        }
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::card_setting(){
  //card_direction_and_place_to_idを全部-1で埋める
  for(int i=1;i<=N_card;i++){
    for(int j=0;j<4;j++){
      for(int k=0;k<H;k++){
        for(int l=0;l<W;l++){
          card_direction_and_place_to_id[i][j][k][l] = -1;
        }
      }
    }
  }
  for(int i=1;i<=N_card;i++){
    //R0
    for(int j=0;j<=H-cards[i].H;j++){
      for(int k=0;k<=W-cards[i].W;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board(i,j,k,0)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {0,j,k};
          card_direction_and_place_to_id[i][0][j][k] = card_status_size[i]-1;
          card_to_bitset(i,j,k,0);
          card_to_bitset_shifted(i,j,k,0);
          card_to_bitset_around(i,j,k,0);
        }
      }
    }
    //R90
    for(int j=0;j<=H-cards[i].W;j++){
      for(int k=0;k<=W-cards[i].H;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board(i,j,k,1)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {1,j,k};
          card_direction_and_place_to_id[i][1][j][k] = card_status_size[i]-1;
          card_to_bitset(i,j,k,1);
          card_to_bitset_shifted(i,j,k,1);
          card_to_bitset_around(i,j,k,1);
        }
      }
    }
    //R180
    for(int j=0;j<=H-cards[i].H;j++){
      for(int k=0;k<=W-cards[i].W;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board(i,j,k,2)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {2,j,k};
          card_direction_and_place_to_id[i][2][j][k] = card_status_size[i]-1;
          card_to_bitset(i,j,k,2);
          card_to_bitset_shifted(i,j,k,2);
          card_to_bitset_around(i,j,k,2);
        }
      }
    }
    //R270
    for(int j=0;j<=H-cards[i].W;j++){
      for(int k=0;k<=W-cards[i].H;k++){
        //i番目のカードが盤面上にあるかどうか確認し、あったらbitsetを用意する
        if(card_is_on_board(i,j,k,3)){
          card_status_size[i]++;
          card_status[i][card_status_size[i]-1] = {3,j,k};
          card_direction_and_place_to_id[i][3][j][k] = card_status_size[i]-1;
          card_to_bitset(i,j,k,3);
          card_to_bitset_shifted(i,j,k,3);
          card_to_bitset_around(i,j,k,3);
        }
      }
    }
  }
}
template<int ID,int H,int W,int N_square> constexpr void Stage<ID,H,W,N_square>::board_default_setting(){
  constexpr std::pair<int,int> cordinate_diff[8] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
  for(int i=0;i<H;i++){
    for(int j=0;j<W;j++){
      if(!exists_square[i][j]) continue;
      for(int k=0;k<8;k++){
        //座標(i,j)から見て(ni,nj)はcordinate_diff[k]平行移動した位置にある
        //(ni,nj)が盤面外か初期のSPマスであれば、is_there_a_block_nearby_default[k]を更新
        int ni = i+cordinate_diff[k].first,nj = j+cordinate_diff[k].second;
        if(ni < 0 || H <= ni || nj < 0 || W <= nj || !exists_square[ni][nj] || (ni == my_starting_pos_H && nj == my_starting_pos_W) || (ni == opponent_starting_pos_H && nj == opponent_starting_pos_W)){
          is_there_a_block_nearby_default[k][place_to_order[i][j]] = true;
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
  board_default_setting();
}