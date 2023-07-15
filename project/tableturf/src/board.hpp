#pragma once
#include "stage.hpp"
#include "stage_database.hpp"
#include <bitset>
#include <iostream>
//P1:Player1(自分) P2:Player2(相手)
template<class stage> class Board{
public:
  static constexpr int max_turn = 12;
  //盤面の縦、横、マス数
  static constexpr int N_square = stage::n_square;
  //デッキに入るカードの枚数
  static constexpr int N_card_in_deck = 15;
  //現在のターン数
  int current_turn;
  //P1とP2が使ったカードのID
  int P1_used_cards[max_turn],P2_used_cards[max_turn];
  //P1の現在のSPポイントとP2の現在のSPポイント
  int P1_SP_point,P2_SP_point;
  //P1が既に使ったSPポイントとP2が既に使ったSPポイント
  int P1_SP_point_used,P2_SP_point_used;
  //P1がパスした回数とP2がパスした回数
  int P1_pass_time,P2_pass_time;
  //P1のマスとP2のマス(SP含む)
  std::bitset<N_square> P1_square,P2_square;
  //P1のSPマスとP2のSPマス
  std::bitset<N_square> P1_square_SP,P2_square_SP;
  //P1のSPポイントに寄与するSPマスとP2のSPポイントに寄与するSPマス
  std::bitset<N_square> P1_square_SP_burning,P2_square_SP_burning;
  //壁
  std::bitset<N_square> wall_square;
  //壁を含むすべてのブロック
  std::bitset<N_square> all_square;
  //SPアタックでも置けないブロック(P1のSPマスとP2のSPマスと壁)
  std::bitset<N_square> hard_square;
  //(左上、上、右上、左、右、左下、下、右下)[i]がブロックまたは盤面外であるか
  std::bitset<N_square> is_there_a_block_nearby[8];
  Board();
  //カードの置き方が合法かどうか
  bool is_valid_placement(const bool is_P1_placement,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const;
  bool is_valid_placement(const bool is_P1_placement,const int card_id,const int status_id,const bool is_SP_attack) const;
  //両方のプレイヤーのカードを置く
  void put_both_cards_without_validation(const int P1_card_id,const int P1_card_direction,const int P1_card_pos_H,const int P1_card_pos_W,const bool is_P1_pass,const bool is_P1_SP_attack,const int P2_card_id,const int P2_card_direction,const int P2_card_pos_H,const int P2_card_pos_W,const bool is_P2_pass,const bool is_P2_SP_attack);
  void put_both_cards_without_validation(const int P1_card_id,const int P1_status_id,const bool is_P1_SP_attack,const int P2_card_id,const int P2_status_id,const bool is_P2_SP_attack);
  //片方のプレイヤーのカードを置く(ビジュアライザ専用)
  void put_P1_card_without_validation(const int P1_card_id,const int P1_card_direction,const int P1_card_pos_H,const int P1_card_pos_W,const bool is_P1_pass,const bool is_P1_SP_attack);
  void put_P1_card_without_validation(const int P1_card_id,const int P1_status_id,const bool is_P1_SP_attack);
  void put_P2_card_without_validation(const int P2_card_id,const int P2_card_direction,const int P2_card_pos_H,const int P2_card_pos_W,const bool is_P2_pass,const bool is_P2_SP_attack);
  void put_P2_card_without_validation(const int P2_card_id,const int P2_status_id,const bool is_P2_SP_attack);
};
template<class stage> Board<stage>::Board():current_turn(1),P1_SP_point(0),P2_SP_point(0),P1_SP_point_used(0),P2_SP_point_used(0),P1_pass_time(0),P2_pass_time(0){
  //初期盤面の生成(最初のSPマスを設置)
  P1_square[stage::place_to_order[stage::P1_starting_pos_H][stage::P1_starting_pos_W]] = true;
  P2_square[stage::place_to_order[stage::P2_starting_pos_H][stage::P2_starting_pos_W]] = true;
  P1_square_SP[stage::place_to_order[stage::P1_starting_pos_H][stage::P1_starting_pos_W]] = true;
  P2_square_SP[stage::place_to_order[stage::P2_starting_pos_H][stage::P2_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::P1_starting_pos_H][stage::P1_starting_pos_W]] = true;
  all_square[stage::place_to_order[stage::P2_starting_pos_H][stage::P2_starting_pos_W]] = true;
  hard_square[stage::place_to_order[stage::P1_starting_pos_H][stage::P1_starting_pos_W]] = true;
  hard_square[stage::place_to_order[stage::P2_starting_pos_H][stage::P2_starting_pos_W]] = true;
  for(int i=0;i<8;i++) is_there_a_block_nearby[i] = stage::is_there_a_block_nearby_default[i];
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_P1_placement,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const{
  int status_id = (is_pass ? -1:stage::card_direction_and_place_to_id[card_id][card_direction][card_pos_H][card_pos_W]);
  //もしパスでないのにも拘わらずstatus_id==-1(カードが盤面外)ならfalse
  if(!is_pass && status_id == -1) return false;
  return is_valid_placement(is_P1_placement,card_id,status_id,is_SP_attack);
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_P1_placement,const int card_id,const int status_id,const bool is_SP_attack) const {
  //パスのときは問答無用でtrue
  if(status_id == -1) return true;
  //SPアタックのときはSPポイントが十分かどうか確認 不足していたらfalse
  if(is_SP_attack && (is_P1_placement ? P1_SP_point:P2_SP_point) < cards[card_id].SP_cost) return false;
  //SPアタックのときは壁とSPマス、通常の時はあるマスに被っていたらfalse
  if((stage::card_covered_square[card_id][status_id]&(is_SP_attack ? hard_square:all_square)).any()) return false;
  //カードがSPアタックなら自身のSPマス,通常なら自身のあるマスに接していたらtrue,接していなかったらfalse
  return (stage::card_around_square[card_id][status_id]&
  (is_P1_placement ?
  (is_SP_attack ? P1_square_SP:P1_square):
  (is_SP_attack ? P2_square_SP:P2_square))).any();
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int P1_card_id,const int P1_card_direction,const int P1_card_pos_H,const int P1_card_pos_W,const bool is_P1_pass,const bool is_P1_SP_attack,const int P2_card_id,const int P2_card_direction,const int P2_card_pos_H,const int P2_card_pos_W,const bool is_P2_pass,const bool is_P2_SP_attack){
  int P1_status_id = (is_P1_pass ? -1:stage::card_direction_and_place_to_id[P1_card_id][P1_card_direction][P1_card_pos_H][P1_card_pos_W]);
  int P2_status_id = (is_P2_pass ? -1:stage::card_direction_and_place_to_id[P2_card_id][P2_card_direction][P2_card_pos_H][P2_card_pos_W]);
  put_both_cards_without_validation(
  P1_card_id,P1_status_id,is_P1_SP_attack,
  P2_card_id,P2_status_id,is_P2_SP_attack);
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int P1_card_id,const int P1_status_id,const bool is_P1_SP_attack,const int P2_card_id,const int P2_status_id,const bool is_P2_SP_attack){
  //P1_status_id,P2_status_idが-1の時はパス
  const std::bitset<N_square> &P1_card_covered_square = stage::card_covered_square[P1_card_id][P1_status_id];
  const std::bitset<N_square> &P1_card_covered_square_normal = stage::card_covered_square_normal[P1_card_id][P1_status_id];
  const std::bitset<N_square> &P1_card_covered_square_SP = stage::card_covered_square_SP[P1_card_id][P1_status_id];
  const std::bitset<N_square> &P2_card_covered_square = stage::card_covered_square[P2_card_id][P2_status_id];
  const std::bitset<N_square> &P2_card_covered_square_normal = stage::card_covered_square_normal[P2_card_id][P2_status_id];
  const std::bitset<N_square> &P2_card_covered_square_SP = stage::card_covered_square_SP[P2_card_id][P2_status_id];
  //SPアタックの時の前処理
  if(is_P1_SP_attack){
    P1_SP_point_used += cards[P1_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P2_squareだけ塗り返す
    P2_square &= ~P1_card_covered_square;
  }
  if(is_P2_SP_attack){
    P2_SP_point_used += cards[P2_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P1_squareだけ塗り替えす
    P1_square &= ~P2_card_covered_square;
  }

  //パスの時の処理
  if(P1_status_id == -1 && P2_status_id == -1){
    P1_pass_time++;P2_pass_time++;
  }
  else if(P1_status_id != -1 && P2_status_id == -1){
    P2_pass_time++;
    P1_square_SP |= P1_card_covered_square_SP;
    P1_square |= P1_card_covered_square;
  }
  else if(P1_status_id == -1 && P2_status_id != -1){
    P1_pass_time++;
    P2_square_SP |= P2_card_covered_square_SP;
    P2_square |= P2_card_covered_square;
  }
  //ここから先は両者パスしていない場合
  //両者のカードの置く位置が一切被っていない場合、軽い処理で済ませる
  else if((P1_card_covered_square&P2_card_covered_square).none()){
    P1_square_SP |= P1_card_covered_square_SP;
    P2_square_SP |= P2_card_covered_square_SP;
    P1_square |= P1_card_covered_square;
    P2_square |= P2_card_covered_square;
  }
  //カードの面積が同じ場合
  else if(cards[P1_card_id].N_square == cards[P2_card_id].N_square){
    //SPとSPまたは通常と通常が衝突したとき、壁となる
    wall_square |= (P1_card_covered_square_SP&P2_card_covered_square_SP)|(P1_card_covered_square_normal&P2_card_covered_square_normal);
    static std::bitset<N_square> wall_square_reversed;
    wall_square_reversed = ~wall_square;
    //壁の部分を取り除いて加える
    P1_square_SP |= P1_card_covered_square_SP&wall_square_reversed;
    P2_square_SP |= P2_card_covered_square_SP&wall_square_reversed;
    P1_square |= P1_card_covered_square&wall_square_reversed;
    P2_square |= P2_card_covered_square&wall_square_reversed;
  }
  //P1のカードの方が面積が小さい場合
  else if(cards[P1_card_id].N_square < cards[P2_card_id].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = P1_card_covered_square_SP;
    P1_square_SP |= added_square;
    P1_square |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = P2_card_covered_square_SP&(~removed_square);
    P2_square_SP |= added_square;
    P2_square |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = P1_card_covered_square_normal&(~removed_square);
    P1_square |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = P2_card_covered_square_normal&(~removed_square);
    P2_square |= added_square;
  }
  //P2のカードの方が面積が小さい場合
  else if(cards[P1_card_id].N_square > cards[P2_card_id].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = P2_card_covered_square_SP;
    P2_square_SP |= added_square;
    P2_square |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = P1_card_covered_square_SP&(~removed_square);
    P1_square_SP |= added_square;
    P1_square |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = P2_card_covered_square_normal&(~removed_square);
    P2_square |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = P1_card_covered_square_normal&(~removed_square);
    P1_square |= added_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(P1_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[P1_card_id][P1_status_id][i];
  }
  if(P2_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[P2_card_id][P2_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //P1_square_SP,P2_square_SPと照合
  P1_square_SP_burning = P1_square_SP&are_there_8_blocks_nearby;
  P2_square_SP_burning = P2_square_SP&are_there_8_blocks_nearby;

  //hard_squareとall_squareを更新
  hard_square = P1_square_SP|P2_square_SP|wall_square;
  all_square = P1_square|P2_square|wall_square;

  //SPポイントを更新
  P1_SP_point = P1_square_SP_burning.count()+P1_pass_time-P1_SP_point_used;
  P2_SP_point = P2_square_SP_burning.count()+P2_pass_time-P2_SP_point_used;
  //P1_used_cardsとP2_used_cardsを更新
  P1_used_cards[current_turn-1] = P1_card_id;
  P2_used_cards[current_turn-1] = P2_card_id;
  //ターン数を更新
  current_turn++;
}
template<class stage> void Board<stage>::put_P1_card_without_validation(const int P1_card_id,const int P1_card_direction,const int P1_card_pos_H,const int P1_card_pos_W,const bool is_P1_pass,const bool is_P1_SP_attack){
  int P1_status_id = (is_P1_pass ? -1:stage::card_direction_and_place_to_id[P1_card_id][P1_card_direction][P1_card_pos_H][P1_card_pos_W]);
  put_P1_card_without_validation(P1_card_id,P1_status_id,is_P1_SP_attack);
}
template<class stage> void Board<stage>::put_P1_card_without_validation(const int P1_card_id,const int P1_status_id,const bool is_P1_SP_attack){
  const std::bitset<N_square> &P1_card_covered_square = stage::card_covered_square[P1_card_id][P1_status_id];
  const std::bitset<N_square> &P1_card_covered_square_SP = stage::card_covered_square_SP[P1_card_id][P1_status_id];
  //SPアタックの時の前処理
  if(is_P1_SP_attack){
    P1_SP_point_used += cards[P1_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P2_squareだけ塗り返す
    P2_square &= ~P1_card_covered_square;
  }
  //パスの時の処理
  if(P1_status_id == -1){
    P1_pass_time++;
  }
  //パスしていない時の処理
  else{
    P1_square_SP |= P1_card_covered_square_SP;
    P1_square |= P1_card_covered_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(P1_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[P1_card_id][P1_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //P1_square_SP,P2_square_SPと照合
  P1_square_SP_burning = P1_square_SP&are_there_8_blocks_nearby;
  P2_square_SP_burning = P2_square_SP&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = P1_square|P2_square|wall_square;

  //SPポイントを更新
  P1_SP_point = P1_square_SP_burning.count()+P1_pass_time-P1_SP_point_used;
  P2_SP_point = P2_square_SP_burning.count()+P2_pass_time-P2_SP_point_used;
}
template<class stage> void Board<stage>::put_P2_card_without_validation(const int P2_card_id,const int P2_card_direction,const int P2_card_pos_H,const int P2_card_pos_W,const bool is_P2_pass,const bool is_P2_SP_attack){
  int P2_status_id = (is_P2_pass ? -1:stage::card_direction_and_place_to_id[P2_card_id][P2_card_direction][P2_card_pos_H][P2_card_pos_W]);
  put_P2_card_without_validation(P2_card_id,P2_status_id,is_P2_SP_attack);
}
template<class stage> void Board<stage>::put_P2_card_without_validation(const int P2_card_id,const int P2_status_id,const bool is_P2_SP_attack){
  const std::bitset<N_square> &P2_card_covered_square = stage::card_covered_square[P2_card_id][P2_status_id];
  const std::bitset<N_square> &P2_card_covered_square_SP = stage::card_covered_square_SP[P2_card_id][P2_status_id];
  //SPアタックの時の前処理
  if(is_P2_SP_attack){
    P2_SP_point_used += cards[P2_card_id].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P1_squareだけ塗り替えす
    P1_square &= ~P2_card_covered_square;
  }
  //パスの時の処理
  if(P2_status_id == -1){
    P2_pass_time++;
  }
  //パスしていない時の処理
  else{
    P2_square_SP |= P2_card_covered_square_SP;
    P2_square |= P2_card_covered_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(P2_status_id != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[P2_card_id][P2_status_id][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //P1_square_SP,P2_square_SPと照合
  P1_square_SP_burning = P1_square_SP&are_there_8_blocks_nearby;
  P2_square_SP_burning = P2_square_SP&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = P1_square|P2_square|wall_square;

  //SPポイントを更新
  P1_SP_point = P1_square_SP_burning.count()+P1_pass_time-P1_SP_point_used;
  P2_SP_point = P2_square_SP_burning.count()+P2_pass_time-P2_SP_point_used;
}