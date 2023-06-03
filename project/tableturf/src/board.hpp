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
  //自分の現在のSPポイントと相手の現在のSPポイント
  int my_SP_point,opponent_SP_point;
  //自分が既に使ったSPポイントと相手が既に使ったSPポイント
  int my_SP_point_used,opponent_SP_point_used;
  //自分がパスした回数と相手がパスした回数
  int my_pass_time,opponent_pass_time;
  //自分のマスと相手のマス(SP含む)
  std::bitset<N_square> my_square,opponent_square;
  //自分のSPマスと相手のSPマス
  std::bitset<N_square> my_square_SP,opponent_square_SP;
  //自分のSPポイントに寄与するSPマスと相手のSPポイントに寄与するSPマス
  std::bitset<N_square> my_square_SP_burning,opponent_square_SP_burning;
  //壁
  std::bitset<N_square> wall_square;
  //壁を含むすべてのブロック
  std::bitset<N_square> all_square;
  //SPアタックでも置けないブロック(自分のSPマスと相手のSPマスと壁)
  std::bitset<N_square> hard_square;
  //(左上、上、右上、左、右、左下、下、右下)[i]がブロックまたは盤面外であるか
  std::bitset<N_square> is_there_a_block_nearby[8];
  Board();
  //カードの置き方が合法かどうか
  bool is_valid_placement(const bool is_my_placement,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const;
  bool is_valid_placement(const bool is_my_placement,const int card_id,const int status_id,const bool is_SP_attack) const;
  //両方のプレイヤーのカードを置く
  void put_both_cards_without_validation(const int my_card_id,const int my_card_direction,const int my_card_pos_H,const int my_card_pos_W,const bool is_my_pass,const bool is_my_SP_attack,const int opponent_card_id,const int opponent_card_direction,const int opponent_card_pos_H,const int opponent_card_pos_W,const bool is_opponent_pass,const bool is_opponent_SP_attack);
  void put_both_cards_without_validation(const int my_card_id,const int my_status_id,const bool is_my_SP_attack,const int opponent_card_id,const int opponent_status_id,const bool is_opponent_SP_attack);
  //片方のプレイヤーのカードを置く(ビジュアライザ専用)
  void put_my_card_without_validation(const int my_card_id,const int my_card_direction,const int my_card_pos_H,const int my_card_pos_W,const bool is_my_pass,const bool is_my_SP_attack);
  void put_my_card_without_validation(const int my_card_id,const int my_status_id,const bool is_my_SP_attack);
  void put_opponent_card_without_validation(const int opponent_card_id,const int opponent_card_direction,const int opponent_card_pos_H,const int opponent_card_pos_W,const bool is_opponent_pass,const bool is_opponent_SP_attack);
  void put_opponent_card_without_validation(const int opponent_card_id,const int opponent_status_id,const bool is_opponent_SP_attack);
};
template<class stage> Board<stage>::Board():current_turn(1),my_SP_point(0),opponent_SP_point(0),my_SP_point_used(0),opponent_SP_point_used(0),my_pass_time(0),opponent_pass_time(0){
  //初期盤面の生成(最初のSPマスを設置)
  my_square[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  opponent_square[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  my_square_SP[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  opponent_square_SP[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  hard_square[stage::place_to_order[stage::my_starting_pos_H][stage::my_starting_pos_W]] = true;
  hard_square[stage::place_to_order[stage::opponent_starting_pos_H][stage::opponent_starting_pos_W]] = true;
  for(int i=0;i<8;i++) is_there_a_block_nearby[i] = stage::is_there_a_block_nearby_default[i];
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_my_placement,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const{
  int status_id = (is_pass ? -1:stage::card_direction_and_place_to_id[card_id][card_direction][card_pos_H][card_pos_W]);
  //もしパスでないのにも拘わらずstatus_id==-1(カードが盤面外)ならfalse
  if(!is_pass && status_id == -1) return false;
  return is_valid_placement(is_my_placement,card_id,status_id,is_SP_attack);
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_my_placement,const int card_id,const int status_id,const bool is_SP_attack) const {
  //パスのときは問答無用でtrue
  if(status_id == -1) return true;
  //SPアタックのときはSPポイントが十分かどうか確認 不足していたらfalse
  if(is_SP_attack && (is_my_placement ? my_SP_point:opponent_SP_point) < cards[card_id].SP_cost) return false;
  //SPアタックのときは壁とSPマス、通常の時はあるマスに被っていたらfalse
  if((stage::card_covered_square[card_id][status_id]&(is_SP_attack ? hard_square:all_square)).any()) return false;
  //カードがSPアタックなら自身のSPマス,通常なら自身のあるマスに接していたらtrue,接していなかったらfalse
  return (stage::card_around_square[card_id][status_id]&
  (is_my_placement ?
  (is_SP_attack ? my_square_SP:my_square):
  (is_SP_attack ? opponent_square_SP:opponent_square))).any();
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int my_card_id,const int my_card_direction,const int my_card_pos_H,const int my_card_pos_W,const bool is_my_pass,const bool is_my_SP_attack,const int opponent_card_id,const int opponent_card_direction,const int opponent_card_pos_H,const int opponent_card_pos_W,const bool is_opponent_pass,const bool is_opponent_SP_attack){
  int my_status_id = (is_my_pass ? -1:stage::card_direction_and_place_to_id[my_card_id][my_card_direction][my_card_pos_H][my_card_pos_W]);
  int opponent_status_id = (is_opponent_pass ? -1:stage::card_direction_and_place_to_id[opponent_card_id][opponent_card_direction][opponent_card_pos_H][opponent_card_pos_W]);
  put_both_cards_without_validation(
  my_card_id,my_status_id,is_my_SP_attack,
  opponent_card_id,opponent_status_id,is_opponent_SP_attack);
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int my_card_id,const int my_status_id,const bool is_my_SP_attack,const int opponent_card_id,const int opponent_status_id,const bool is_opponent_SP_attack){
  //my_status_id,opponent_status_idが-1の時はパス
  const std::bitset<N_square> &my_card_covered_square = stage::card_covered_square[my_card_id][my_status_id];
  const std::bitset<N_square> &my_card_covered_square_normal = stage::card_covered_square_normal[my_card_id][my_status_id];
  const std::bitset<N_square> &my_card_covered_square_SP = stage::card_covered_square_SP[my_card_id][my_status_id];
  const std::bitset<N_square> &opponent_card_covered_square = stage::card_covered_square[opponent_card_id][opponent_status_id];
  const std::bitset<N_square> &opponent_card_covered_square_normal = stage::card_covered_square_normal[opponent_card_id][opponent_status_id];
  const std::bitset<N_square> &opponent_card_covered_square_SP = stage::card_covered_square_SP[opponent_card_id][opponent_status_id];
  //SPアタックの時の前処理
  if(is_my_SP_attack){
    my_SP_point_used += cards[my_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、opponent_squareだけ塗り返す
    opponent_square &= ~my_card_covered_square;
  }
  if(is_opponent_SP_attack){
    opponent_SP_point_used += cards[opponent_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、my_squareだけ塗り替えす
    my_square &= ~opponent_card_covered_square;
  }

  //パスの時の処理
  if(my_status_id == -1 && opponent_status_id == -1){
    my_pass_time++;opponent_pass_time++;
  }
  else if(my_status_id != -1 && opponent_status_id == -1){
    opponent_pass_time++;
    my_square_SP |= my_card_covered_square_SP;
    my_square |= my_card_covered_square;
  }
  else if(my_status_id == -1 && opponent_status_id != -1){
    my_pass_time++;
    opponent_square_SP |= opponent_card_covered_square_SP;
    opponent_square |= opponent_card_covered_square;
  }
  //ここから先は両者パスしていない場合
  //両者のカードの置く位置が一切被っていない場合、軽い処理で済ませる
  else if((my_card_covered_square&opponent_card_covered_square).none()){
    my_square_SP |= my_card_covered_square_SP;
    opponent_square_SP |= opponent_card_covered_square_SP;
    my_square |= my_card_covered_square;
    opponent_square |= opponent_card_covered_square;
  }
  //カードの面積が同じ場合
  else if(cards[my_card_id].N_square == cards[opponent_card_id].N_square){
    //SPとSPまたは通常と通常が衝突したとき、壁となる
    wall_square |= (my_card_covered_square_SP&opponent_card_covered_square_SP)|(my_card_covered_square_normal&opponent_card_covered_square_normal);
    static std::bitset<N_square> wall_square_reversed;
    wall_square_reversed = ~wall_square;
    //壁の部分を取り除いて加える
    my_square_SP |= my_card_covered_square_SP&wall_square_reversed;
    opponent_square_SP |= opponent_card_covered_square_SP&wall_square_reversed;
    my_square |= my_card_covered_square&wall_square_reversed;
    opponent_square |= opponent_card_covered_square&wall_square_reversed;
  }
  //自分のカードの方が面積が小さい場合
  else if(cards[my_card_id].N_square < cards[opponent_card_id].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = my_card_covered_square_SP;
    my_square_SP |= added_square;
    my_square |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = opponent_card_covered_square_SP&(~removed_square);
    opponent_square_SP |= added_square;
    opponent_square |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = my_card_covered_square_normal&(~removed_square);
    my_square |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = opponent_card_covered_square_normal&(~removed_square);
    opponent_square |= added_square;
  }
  //相手のカードの方が面積が小さい場合
  else if(cards[my_card_id].N_square > cards[opponent_card_id].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = opponent_card_covered_square_SP;
    opponent_square_SP |= added_square;
    opponent_square |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = my_card_covered_square_SP&(~removed_square);
    my_square_SP |= added_square;
    my_square |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = opponent_card_covered_square_normal&(~removed_square);
    opponent_square |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = my_card_covered_square_normal&(~removed_square);
    my_square |= added_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(my_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[my_card_id][my_status_id][i];
  }
  if(opponent_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[opponent_card_id][opponent_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //my_square_SP,opponent_square_SPと照合
  my_square_SP_burning = my_square_SP&are_there_8_blocks_nearby;
  opponent_square_SP_burning = opponent_square_SP&are_there_8_blocks_nearby;

  //hard_squareとall_squareを更新
  hard_square = my_square_SP|opponent_square_SP|wall_square;
  all_square = my_square|opponent_square|wall_square;

  //SPポイントを更新
  my_SP_point = my_square_SP_burning.count()+my_pass_time-my_SP_point_used;
  opponent_SP_point = opponent_square_SP_burning.count()+opponent_pass_time-opponent_SP_point_used;
  //ターン数を更新
  current_turn++;
}
template<class stage> void Board<stage>::put_my_card_without_validation(const int my_card_id,const int my_card_direction,const int my_card_pos_H,const int my_card_pos_W,const bool is_my_pass,const bool is_my_SP_attack){
  int my_status_id = (is_my_pass ? -1:stage::card_direction_and_place_to_id[my_card_id][my_card_direction][my_card_pos_H][my_card_pos_W]);
  put_my_card_without_validation(my_card_id,my_status_id,is_my_SP_attack);
}
template<class stage> void Board<stage>::put_my_card_without_validation(const int my_card_id,const int my_status_id,const bool is_my_SP_attack){
  const std::bitset<N_square> &my_card_covered_square = stage::card_covered_square[my_card_id][my_status_id];
  const std::bitset<N_square> &my_card_covered_square_SP = stage::card_covered_square_SP[my_card_id][my_status_id];
  //SPアタックの時の前処理
  if(is_my_SP_attack){
    my_SP_point_used += cards[my_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、opponent_squareだけ塗り返す
    opponent_square &= ~my_card_covered_square;
  }
  //パスの時の処理
  if(my_status_id == -1){
    my_pass_time++;
  }
  //パスしていない時の処理
  else{
    my_square_SP |= my_card_covered_square_SP;
    my_square |= my_card_covered_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(my_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[my_card_id][my_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //my_square_SP,opponent_square_SPと照合
  my_square_SP_burning = my_square_SP&are_there_8_blocks_nearby;
  opponent_square_SP_burning = opponent_square_SP&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = my_square|opponent_square|wall_square;

  //SPポイントを更新
  my_SP_point = my_square_SP_burning.count()+my_pass_time-my_SP_point_used;
  opponent_SP_point = opponent_square_SP_burning.count()+opponent_pass_time-opponent_SP_point_used;
}
template<class stage> void Board<stage>::put_opponent_card_without_validation(const int opponent_card_id,const int opponent_card_direction,const int opponent_card_pos_H,const int opponent_card_pos_W,const bool is_opponent_pass,const bool is_opponent_SP_attack){
  int opponent_status_id = (is_opponent_pass ? -1:stage::card_direction_and_place_to_id[opponent_card_id][opponent_card_direction][opponent_card_pos_H][opponent_card_pos_W]);
  put_opponent_card_without_validation(opponent_card_id,opponent_status_id,is_opponent_SP_attack);
}
template<class stage> void Board<stage>::put_opponent_card_without_validation(const int opponent_card_id,const int opponent_status_id,const bool is_opponent_SP_attack){
  const std::bitset<N_square> &opponent_card_covered_square = stage::card_covered_square[opponent_card_id][opponent_status_id];
  const std::bitset<N_square> &opponent_card_covered_square_SP = stage::card_covered_square_SP[opponent_card_id][opponent_status_id];
  //SPアタックの時の前処理
  if(is_opponent_SP_attack){
    opponent_SP_point_used += cards[opponent_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、my_squareだけ塗り替えす
    my_square &= ~opponent_card_covered_square;
  }
  //パスの時の処理
  if(opponent_status_id == -1){
    opponent_pass_time++;
  }
  //パスしていない時の処理
  else{
    opponent_square_SP |= opponent_card_covered_square_SP;
    opponent_square |= opponent_card_covered_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(opponent_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[opponent_card_id][opponent_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //my_square_SP,opponent_square_SPと照合
  my_square_SP_burning = my_square_SP&are_there_8_blocks_nearby;
  opponent_square_SP_burning = opponent_square_SP&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = my_square|opponent_square|wall_square;

  //SPポイントを更新
  my_SP_point = my_square_SP_burning.count()+my_pass_time-my_SP_point_used;
  opponent_SP_point = opponent_square_SP_burning.count()+opponent_pass_time-opponent_SP_point_used;
}