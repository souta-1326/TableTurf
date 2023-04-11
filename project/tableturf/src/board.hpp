#pragma once
#include "stage.hpp"
#include "stage_database.hpp"
#include <bitset>
#include <iostream>
template<class stage> class Board{
public:
  static constexpr int max_turn = 12;
  //盤面の縦、横、マス数
  static constexpr int N_square = stage::n_square;
  //現在のターン数
  int current_turn;
  //自分のマスと相手のマス(SP含む)
  std::bitset<N_square> my_square,opponent_square;
  //自分のSPマスと相手のSPマス
  std::bitset<N_square> my_square_SP,opponent_square_SP;
  //壁
  std::bitset<N_square> wall_square;
  //壁を含むすべてのブロック
  std::bitset<N_square> all_square;
  //SPアタックでも置けないブロック(自分のSPマスと相手のSPマスと壁)
  std::bitset<N_square> hard_square;
  Board();
  bool is_valid_placement(int card_id,int status_id){
    assert(status_id < stage::card_status_size[card_id]);
    //TODO
  }
  void put_both_card_without_validation(int my_card_id,int my_card_direction,int my_card_H,int my_card_W,bool my_SP_attack,int opponent_card_id,int opponent_card_direction,int opponent_card_H,int opponent_card_W,bool opponent_SP_attack);
  void put_both_card_without_validation(int my_card_id,int my_status_id,bool my_SP_attack,int opponent_card_id,int opponent_status_id,bool opponent_SP_attack);
};
template<class stage> Board<stage>::Board():current_turn(1){
  //初期盤面の生成(最初のSPマスを設置)
  my_square[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  opponent_square[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  my_square_SP[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  opponent_square_SP[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
}
template<class stage> void Board<stage>::put_both_card_without_validation(int my_card_id,int my_card_direction,int my_card_H,int my_card_W,bool my_SP_attack,int opponent_card_id,int opponent_card_direction,int opponent_card_H,int opponent_card_W,bool opponent_SP_attack){
  put_both_card_without_validation(my_card_id,stage::card_direction_and_place_to_id[my_card_id][my_card_direction][my_card_H][my_card_W],my_SP_attack,
  opponent_card_id,stage::card_direction_and_place_to_id[my_card_id][opponent_card_direction][opponent_card_H][opponent_card_W],opponent_SP_attack);
}
template<class stage> void Board<stage>::put_both_card_without_validation(int my_card_id,int my_status_id,bool my_SP_attack,int opponent_card_id,int opponent_status_id,bool opponent_SP_attack){
  std::bitset<N_square> &my_card_covered_square = stage::card_covered_square[my_card_id][my_status_id];
  std::bitset<N_square> &my_card_covered_square_normal = stage::card_covered_square_normal[my_card_id][my_status_id];
  std::bitset<N_square> &my_card_covered_square_SP = stage::card_covered_square_SP[my_card_id][my_status_id];
  std::bitset<N_square> &opponent_card_covered_square = stage::card_covered_square[opponent_card_id][opponent_status_id];
  std::bitset<N_square> &opponent_card_covered_square_normal = stage::card_covered_square_normal[opponent_card_id][opponent_status_id];
  std::bitset<N_square> &opponent_card_covered_square_SP = stage::card_covered_square_SP[opponent_card_id][opponent_status_id];
  //パスの時の処理がこの下に入る予定
  assert(my_status_id != -1 && opponent_status_id != -1);
  //SPアタックの時の処理がこの下に入る予定
  //カードの面積が同じ場合
  if(cards[my_card_id].N_square == cards[opponent_card_id].N_square){
    //SPとSPまたは通常と通常が衝突したとき、壁となる
    wall_square |= (my_card_covered_square_SP&opponent_card_covered_square_SP)|(my_card_covered_square_normal&opponent_card_covered_square_normal);
    static std::bitset<N_square> wall_square_reversed;
    wall_square_reversed = ~wall_square;
    //壁の部分を取り除く
    my_square |= my_card_covered_square&wall_square_reversed;
    opponent_square |= opponent_card_covered_square&wall_square_reversed;
    my_square_SP |= my_card_covered_square_SP&wall_square_reversed;
    opponent_square_SP |= opponent_card_covered_square_SP&wall_square_reversed;
  }
  else if(cards[my_card_id].N_square < cards[opponent_card_id].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> removed_square;

  }
  hard_square = my_square_SP|opponent_square_SP|wall_square;
  all_square = my_square|opponent_square|wall_square;
}