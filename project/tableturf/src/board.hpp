#pragma once
#include <bitset>
#include <iostream>
#include "stage.hpp"
#include "stage_database.hpp"
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
  int used_cards_P1[max_turn],used_cards_P2[max_turn];
  //P1の現在のSPポイントとP2の現在のSPポイント
  int SP_point_P1,SP_point_P2;
  //P1が既に使ったSPポイントとP2が既に使ったSPポイント
  int SP_point_used_P1,SP_point_used_P2;
  //P1がパスした回数とP2がパスした回数
  int pass_time_P1,pass_time_P2;
  //P1のマスとP2のマス(SP含む)
  std::bitset<N_square> square_P1,square_P2;
  //P1のSPマスとP2のSPマス
  std::bitset<N_square> square_SP_P1,square_SP_P2;
  //P1のSPポイントに寄与するSPマスとP2のSPポイントに寄与するSPマス
  std::bitset<N_square> square_SP_burning_P1,square_SP_burning_P2;
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
  bool is_valid_placement(const bool is_placement_P1,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const;
  bool is_valid_placement(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack) const;
  //両方のプレイヤーのカードを置く
  void put_both_cards_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1,const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2);
  void put_both_cards_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1,const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2);
  //片方のプレイヤーのカードを置く(ビジュアライザ専用)
  void put_P1_card_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1);
  void put_P1_card_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1);
  void put_P2_card_without_validation(const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2);
  void put_P2_card_without_validation(const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2);
};
template<class stage> Board<stage>::Board():current_turn(1),SP_point_P1(0),SP_point_P2(0),SP_point_used_P1(0),SP_point_used_P2(0),pass_time_P1(0),pass_time_P2(0){
  //初期盤面の生成(最初のSPマスを設置)
  square_P1[stage::place_to_order[stage::starting_pos_H_P1][stage::starting_pos_W_P1]] = true;
  square_P2[stage::place_to_order[stage::starting_pos_H_P2][stage::starting_pos_W_P2]] = true;
  square_SP_P1[stage::place_to_order[stage::starting_pos_H_P1][stage::starting_pos_W_P1]] = true;
  square_SP_P2[stage::place_to_order[stage::starting_pos_H_P2][stage::starting_pos_W_P2]] = true;
  all_square[stage::place_to_order[stage::starting_pos_H_P1][stage::starting_pos_W_P1]] = true;
  all_square[stage::place_to_order[stage::starting_pos_H_P2][stage::starting_pos_W_P2]] = true;
  hard_square[stage::place_to_order[stage::starting_pos_H_P1][stage::starting_pos_W_P1]] = true;
  hard_square[stage::place_to_order[stage::starting_pos_H_P2][stage::starting_pos_W_P2]] = true;
  for(int i=0;i<8;i++) is_there_a_block_nearby[i] = stage::is_there_a_block_nearby_default[i];
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_placement_P1,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack) const{
  int status_id = (is_pass ? -1:stage::card_direction_and_place_to_id[card_id][card_direction][card_pos_H][card_pos_W]);
  //もしパスでないのにも拘わらずstatus_id==-1(カードが盤面外)ならfalse
  if(!is_pass && status_id == -1) return false;
  return is_valid_placement(is_placement_P1,card_id,status_id,is_SP_attack);
}
template<class stage> bool Board<stage>::is_valid_placement(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack) const {
  //パスのときは問答無用でtrue
  if(status_id == -1) return true;
  //SPアタックのときはSPポイントが十分かどうか確認 不足していたらfalse
  if(is_SP_attack && (is_placement_P1 ? SP_point_P1:SP_point_P2) < cards[card_id].SP_cost) return false;
  //SPアタックのときは壁とSPマス、通常の時はあるマスに被っていたらfalse
  if((stage::card_covered_square[card_id][status_id]&(is_SP_attack ? hard_square:all_square)).any()) return false;
  //カードがSPアタックなら自身のSPマス,通常なら自身のあるマスに接していたらtrue,接していなかったらfalse
  return (stage::card_around_square[card_id][status_id]&
  (is_placement_P1 ?
  (is_SP_attack ? square_SP_P1:square_P1):
  (is_SP_attack ? square_SP_P2:square_P2))).any();
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1,const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2){
  int status_id_P1 = (is_pass_P1 ? -1:stage::card_direction_and_place_to_id[card_id_P1][card_direction_P1][card_pos_H_P1][card_pos_W_P1]);
  int status_id_P2 = (is_pass_P2 ? -1:stage::card_direction_and_place_to_id[card_id_P2][card_direction_P2][card_pos_H_P2][card_pos_W_P2]);
  put_both_cards_without_validation(
  card_id_P1,status_id_P1,is_SP_attack_P1,
  card_id_P2,status_id_P2,is_SP_attack_P2);
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1,const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2){
  //status_id_P1,P2_status_idが-1の時はパス
  const std::bitset<N_square> &card_covered_square_P1 = stage::card_covered_square[card_id_P1][status_id_P1];
  const std::bitset<N_square> &card_covered_square_normal_P1 = stage::card_covered_square_normal[card_id_P1][status_id_P1];
  const std::bitset<N_square> &card_covered_square_SP_P1 = stage::card_covered_square_SP[card_id_P1][status_id_P1];
  const std::bitset<N_square> &card_covered_square_P2 = stage::card_covered_square[card_id_P2][status_id_P2];
  const std::bitset<N_square> &card_covered_square_normal_P2 = stage::card_covered_square_normal[card_id_P2][status_id_P2];
  const std::bitset<N_square> &card_covered_square_SP_P2 = stage::card_covered_square_SP[card_id_P2][status_id_P2];
  //SPアタックの時の前処理
  if(is_SP_attack_P1){
    SP_point_used_P1 += cards[card_id_P1].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P2_squareだけ塗り返す
    square_P2 &= ~card_covered_square_P1;
  }
  if(is_SP_attack_P2){
    SP_point_used_P2 += cards[card_id_P2].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P1_squareだけ塗り替えす
    square_P1 &= ~card_covered_square_P2;
  }

  //パスの時の処理
  if(status_id_P1 == -1 && status_id_P2 == -1){
    pass_time_P1++;pass_time_P2++;
  }
  else if(status_id_P1 != -1 && status_id_P2 == -1){
    pass_time_P2++;
    square_SP_P1 |= card_covered_square_SP_P1;
    square_P1 |= card_covered_square_P1;
  }
  else if(status_id_P1 == -1 && status_id_P2 != -1){
    pass_time_P1++;
    square_SP_P2 |= card_covered_square_SP_P2;
    square_P2 |= card_covered_square_P2;
  }
  //ここから先は両者パスしていない場合
  //両者のカードの置く位置が一切被っていない場合、軽い処理で済ませる
  else if((card_covered_square_P1&card_covered_square_P2).none()){
    square_SP_P1 |= card_covered_square_SP_P1;
    square_SP_P2 |= card_covered_square_SP_P2;
    square_P1 |= card_covered_square_P1;
    square_P2 |= card_covered_square_P2;
  }
  //カードの面積が同じ場合
  else if(cards[card_id_P1].N_square == cards[card_id_P2].N_square){
    //SPとSPまたは通常と通常が衝突したとき、壁となる
    wall_square |= (card_covered_square_SP_P1&card_covered_square_SP_P2)|(card_covered_square_normal_P1&card_covered_square_normal_P2);
    //壁以外を処理
    square_SP_P1 |= card_covered_square_SP_P1&(~card_covered_square_SP_P2);
    square_SP_P2 |= card_covered_square_SP_P2&(~card_covered_square_SP_P1);
    square_P1 |= square_SP_P1|(card_covered_square_normal_P1&(~card_covered_square_P2));
    square_P2 |= square_SP_P2|(card_covered_square_normal_P2&(~card_covered_square_P1));
  }
  //P1のカードの方が面積が小さい場合
  else if(cards[card_id_P1].N_square < cards[card_id_P2].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = card_covered_square_SP_P1;
    square_SP_P1 |= added_square;
    square_P1 |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = card_covered_square_SP_P2&(~removed_square);
    square_SP_P2 |= added_square;
    square_P2 |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = card_covered_square_normal_P1&(~removed_square);
    square_P1 |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = card_covered_square_normal_P2&(~removed_square);
    square_P2 |= added_square;
  }
  //P2のカードの方が面積が小さい場合
  else if(cards[card_id_P1].N_square > cards[card_id_P2].N_square){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    static std::bitset<N_square> added_square,removed_square;
    //面積小SP
    added_square = card_covered_square_SP_P2;
    square_SP_P2 |= added_square;
    square_P2 |= added_square;
    removed_square = added_square;
    //面積大SP
    added_square = card_covered_square_SP_P1&(~removed_square);
    square_SP_P1 |= added_square;
    square_P1 |= added_square;
    removed_square |= added_square;
    //面積小通常
    added_square = card_covered_square_normal_P2&(~removed_square);
    square_P2 |= added_square;
    removed_square |= added_square;
    //面積大通常
    added_square = card_covered_square_normal_P1&(~removed_square);
    square_P1 |= added_square;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(status_id_P1 != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[card_id_P1][status_id_P1][i];
  }
  if(status_id_P2 != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[card_id_P2][status_id_P2][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //square_SP_P1,P2_square_SPと照合
  square_SP_burning_P1 = square_SP_P1&are_there_8_blocks_nearby;
  square_SP_burning_P2 = square_SP_P2&are_there_8_blocks_nearby;

  //hard_squareとall_squareを更新
  hard_square = square_SP_P1|square_SP_P2|wall_square;
  all_square = square_P1|square_P2|wall_square;

  //SPポイントを更新
  SP_point_P1 = square_SP_burning_P1.count()+pass_time_P1-SP_point_used_P1;
  SP_point_P2 = square_SP_burning_P2.count()+pass_time_P2-SP_point_used_P2;
  //P1_used_cardsとP2_used_cardsを更新
  used_cards_P1[current_turn-1] = card_id_P1;
  used_cards_P2[current_turn-1] = card_id_P2;
  //ターン数を更新
  current_turn++;
}
template<class stage> void Board<stage>::put_P1_card_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1){
  int status_id_P1 = (is_pass_P1 ? -1:stage::card_direction_and_place_to_id[card_id_P1][card_direction_P1][card_pos_H_P1][card_pos_W_P1]);
  put_P1_card_without_validation(card_id_P1,status_id_P1,is_SP_attack_P1);
}
template<class stage> void Board<stage>::put_P1_card_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1){
  const std::bitset<N_square> &card_covered_square_P1 = stage::card_covered_square[card_id_P1][status_id_P1];
  const std::bitset<N_square> &card_covered_square_SP_P1 = stage::card_covered_square_SP[card_id_P1][status_id_P1];
  //SPアタックの時の前処理
  if(is_SP_attack_P1){
    SP_point_used_P1 += cards[card_id_P1].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P2_squareだけ塗り返す
    square_P2 &= ~card_covered_square_P1;
  }
  //パスの時の処理
  if(status_id_P1 == -1){
    pass_time_P1++;
  }
  //パスしていない時の処理
  else{
    square_SP_P1 |= card_covered_square_SP_P1;
    square_P1 |= card_covered_square_P1;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(status_id_P1 != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[card_id_P1][status_id_P1][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //square_SP_P1,P2_square_SPと照合
  square_SP_burning_P1 = square_SP_P1&are_there_8_blocks_nearby;
  square_SP_burning_P2 = square_SP_P2&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = square_P1|square_P2|wall_square;

  //SPポイントを更新
  SP_point_P1 = square_SP_burning_P1.count()+pass_time_P1-SP_point_used_P1;
  SP_point_P2 = square_SP_burning_P2.count()+pass_time_P2-SP_point_used_P2;
}
template<class stage> void Board<stage>::put_P2_card_without_validation(const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2){
  int status_id_P2 = (is_pass_P2 ? -1:stage::card_direction_and_place_to_id[card_id_P2][card_direction_P2][card_pos_H_P2][card_pos_W_P2]);
  put_P2_card_without_validation(card_id_P2,status_id_P2,is_SP_attack_P2);
}
template<class stage> void Board<stage>::put_P2_card_without_validation(const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2){
  const std::bitset<N_square> &card_covered_square_P2 = stage::card_covered_square[card_id_P2][status_id_P2];
  const std::bitset<N_square> &card_covered_square_SP_P2 = stage::card_covered_square_SP[card_id_P2][status_id_P2];
  //SPアタックの時の前処理
  if(is_SP_attack_P2){
    SP_point_used_P2 += cards[card_id_P2].SP_cost;
    //一度SPマスを取ったら取り返されることはないので、P1_squareだけ塗り替えす
    square_P1 &= ~card_covered_square_P2;
  }
  //パスの時の処理
  if(status_id_P2 == -1){
    pass_time_P2++;
  }
  //パスしていない時の処理
  else{
    square_SP_P2 |= card_covered_square_SP_P2;
    square_P2 |= card_covered_square_P2;
  }
  //8方向全てに囲まれているスペシャルマスを取得する
  //まずis_there_a_block_nearbyを更新
  if(status_id_P2 != -1){
    for(int i=0;i<8;i++) is_there_a_block_nearby[i] |= stage::card_shifted_square[card_id_P2][status_id_P2][i];
  }
  //8方向全て囲まれているかを表すbitsetを取得する
  static std::bitset<N_square> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //square_SP_P1,P2_square_SPと照合
  square_SP_burning_P1 = square_SP_P1&are_there_8_blocks_nearby;
  square_SP_burning_P2 = square_SP_P2&are_there_8_blocks_nearby;
  //all_squareを更新
  all_square = square_P1|square_P2|wall_square;

  //SPポイントを更新
  SP_point_P1 = square_SP_burning_P1.count()+pass_time_P1-SP_point_used_P1;
  SP_point_P2 = square_SP_burning_P2.count()+pass_time_P2-SP_point_used_P2;
}