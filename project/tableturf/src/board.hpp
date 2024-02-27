#pragma once
#include <bitset>
#include <vector>
#include <iostream>
#include <cassert>
#include "stage.hpp"
#include "stage_database.hpp"
#include "deck.hpp"
#include "choice.hpp"
//P1:Player1(自分) P2:Player2(相手)
template<class stage> class Board{
public:
  //最大ターン数
  static constexpr int TURN_MAX = 12;
  //盤面の縦、横、マス数
  static constexpr int N_SQUARE = stage::N_SQUARE;
  //現在のターン数(1からスタート)
  int current_turn;
  //P1とP2が使ったカードのID(1-indexed)
  std::vector<int> used_cards_P1,used_cards_P2;

  //P1の現在のSPポイントとP2の現在のSPポイント
  int SP_point_P1,SP_point_P2;
  //P1が既に使ったSPポイントとP2が既に使ったSPポイント
  int SP_point_used_P1,SP_point_used_P2;
  //P1がパスした回数とP2がパスした回数
  int pass_time_P1,pass_time_P2;
  //P1のマスとP2のマス(SP含む)
  std::bitset<N_SQUARE> square_P1,square_P2;
  //P1のSPマスとP2のSPマス
  std::bitset<N_SQUARE> square_SP_P1,square_SP_P2;
  //P1のSPポイントに寄与するSPマスとP2のSPポイントに寄与するSPマス
  std::bitset<N_SQUARE> square_SP_burning_P1,square_SP_burning_P2;
  //壁
  std::bitset<N_SQUARE> wall_square;
  //壁を含むすべてのブロック
  std::bitset<N_SQUARE> all_square;
  //SPアタックでも置けないブロック(P1のSPマスとP2のSPマスと壁)
  std::bitset<N_SQUARE> hard_square;
  //(左上、上、右上、左、右、左下、下、右下)[i]がブロックまたは盤面外であるか
  std::bitset<N_SQUARE> is_there_a_block_nearby[8];
public:
  Board();
  constexpr int get_current_turn() const {return current_turn;}
  //SPポイントが足りているかどうか
  bool is_enough_SP_point(const bool is_placement_P1,const int card_id) const;
  //カードの置き方が合法かどうか
  //detect_unnessesary_SP_attack=0:ゲーム内で出来るなら全てOK
  //detect_unnessesary_SP_attack=1:完全に無意味なSPアタック(どのマスとも被らない)を除外
  //detect_unnessesary_SP_attack=2:ほぼ無意味なSPアタック(敵のマスと被らない)を除外
  constexpr bool is_valid_placement(const bool is_placement_P1,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack,const short detect_unnessesary_SP_attack=2) const noexcept;
  constexpr bool is_valid_placement(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack,const short detect_unnessesary_SP_attack=2) const noexcept;
  constexpr bool is_valid_placement(const bool is_placement_P1,const Choice<stage> choice,const short detect_unnessesary_SP_attack=2) const noexcept;
  //SPポイントが足りているかの判断を行わない(高速化のため)
  constexpr bool is_valid_placement_without_SP_point_validation(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack,const short detect_unnessesary_SP_attack=2) const noexcept;
  constexpr bool is_valid_placement_without_SP_point_validation(const bool is_placement_P1,const Choice<stage> choice,const short detect_unnessesary_SP_attack=2) const noexcept;
  //デッキの手札に対する合法手リスト [手札のi枚目][SPアタックか]
  std::vector<std::vector<std::vector<Choice<stage>>>> get_valid_choices(const bool is_placement_P1,const Deck &deck,const bool include_pass = true) const;
  //両方のプレイヤーのカードを置く(合法チェックなし)
  void put_both_cards_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1,const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2);
  void put_both_cards_without_validation(const Choice<stage> choice_P1,const Choice<stage> choice_P2);
  void put_both_cards_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1,const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2);
  //片方のプレイヤーのカードを置く
  void put_P1_card_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1);
  void put_P1_card_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1);
  void put_P1_card_without_validation(const Choice<stage> choice_P1);
  void put_P2_card_without_validation(const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2);
  void put_P2_card_without_validation(const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2);
  void put_P2_card_without_validation(const Choice<stage> choice_P2);
  //プレイヤーのマス数を求める
  int square_count_P1() const;
  int square_count_P2() const;
  //SPポイント
  int get_SP_point_P1() const {return SP_point_P1;}
  int get_SP_point_P2() const {return SP_point_P2;}
  //結果判定
  float get_final_value(float diff_bonus) const;
  //コンソールに盤面を出力
  void show() const;
};
template<class stage> Board<stage>::Board():
current_turn(1),
used_cards_P1(TURN_MAX+1),used_cards_P2(TURN_MAX+1),
SP_point_P1(0),SP_point_P2(0),SP_point_used_P1(0),SP_point_used_P2(0),pass_time_P1(0),pass_time_P2(0)
{
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
template<class stage> bool Board<stage>::is_enough_SP_point(const bool is_placement_P1,const int card_id) const {
  return (is_placement_P1 ? SP_point_P1:SP_point_P2) >= cards[card_id].SP_COST;
}

template<class stage> constexpr bool Board<stage>::is_valid_placement(const bool is_placement_P1,const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack,const short detect_unnessesary_SP_attack) const noexcept{
  int status_id = (is_pass ? -1:stage::card_direction_and_place_to_id[card_id][card_direction][card_pos_H][card_pos_W]);
  //もしパスでないのにも拘わらずstatus_id==-1(カードが盤面外)ならfalse
  if(!is_pass && status_id == -1) return false;
  return is_valid_placement(is_placement_P1,card_id,status_id,is_SP_attack,detect_unnessesary_SP_attack);
}
template<class stage> constexpr bool Board<stage>::is_valid_placement(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack,const short detect_unnessesary_SP_attack) const noexcept{
  //パスのときは、SPアタックをちゃんとOFFにしていたらtrue
  if(status_id == -1) return !is_SP_attack;
  //SPアタックのときはSPポイントが十分かどうか確認 不足していたらfalse
  //if(is_SP_attack && (is_placement_P1 ? SP_point_P1:SP_point_P2) < cards[card_id].SP_COST) return false;
  if(is_SP_attack && !is_enough_SP_point(is_placement_P1,card_id)) return false;
  //SPアタックのときは壁とSPマス、通常の時はあるマスに被っていたらfalse
  if((stage::card_covered_square[card_id][status_id]&(is_SP_attack ? hard_square:all_square)).any()) return false;
  //無駄なSPアタックを、detect_unnessesary_SP_attackの値に基づいて除外
  if(is_SP_attack && detect_unnessesary_SP_attack>=1 &&
  (stage::card_covered_square[card_id][status_id]&(detect_unnessesary_SP_attack == 2 ? (is_placement_P1 ? square_P2:square_P1):(all_square))).none()) return false;
  //カードがSPアタックなら自身のSPマス,通常なら自身のあるマスに接していたらtrue,接していなかったらfalse
  return (stage::card_around_square[card_id][status_id]&
  (is_placement_P1 ?
  (is_SP_attack ? square_SP_P1:square_P1):
  (is_SP_attack ? square_SP_P2:square_P2))).any();
}
template<class stage> constexpr bool Board<stage>::is_valid_placement(const bool is_placement_P1,const Choice<stage> choice,const short detect_unnessesary_SP_attack) const noexcept{
  return is_valid_placement(is_placement_P1,choice.card_id,choice.status_id,choice.is_SP_attack,detect_unnessesary_SP_attack);
}

template<class stage> constexpr bool Board<stage>::is_valid_placement_without_SP_point_validation(const bool is_placement_P1,const int card_id,const int status_id,const bool is_SP_attack,const short detect_unnessesary_SP_attack) const noexcept{
  //パスのときは、SPアタックをちゃんとOFFにしていたらtrue
  if(status_id == -1) return !is_SP_attack;
  //SPアタックのときは壁とSPマス、通常の時はあるマスに被っていたらfalse
  if((stage::card_covered_square[card_id][status_id]&(is_SP_attack ? hard_square:all_square)).any()) return false;
  //無駄なSPアタックを、detect_unnessesary_SP_attackの値に基づいて除外
  if(is_SP_attack && detect_unnessesary_SP_attack>=1 &&
  (stage::card_covered_square[card_id][status_id]&(detect_unnessesary_SP_attack == 2 ? (is_placement_P1 ? square_P2:square_P1):(all_square))).none()) return false;
  //カードがSPアタックなら自身のSPマス,通常なら自身のあるマスに接していたらtrue,接していなかったらfalse
  return (stage::card_around_square[card_id][status_id]&
  (is_placement_P1 ?
  (is_SP_attack ? square_SP_P1:square_P1):
  (is_SP_attack ? square_SP_P2:square_P2))).any();
}
template<class stage> constexpr bool Board<stage>::is_valid_placement_without_SP_point_validation(const bool is_placement_P1,const Choice<stage> choice,const short detect_unnessesary_SP_attack) const noexcept{
  return is_valid_placement_without_SP_point_validation(is_placement_P1,choice.card_id,choice.status_id,choice.is_SP_attack,detect_unnessesary_SP_attack);
}

template<class stage> std::vector<std::vector<std::vector<Choice<stage>>>> Board<stage>::get_valid_choices(const bool is_placement_P1,const Deck &deck,const bool include_pass) const {
  std::vector<std::vector<std::vector<Choice<stage>>>> valid_choices(4,std::vector<std::vector<Choice<stage>>>(2));
  std::vector<int> hand = deck.get_hand();
  for(int i=0;i<Deck::N_CARD_IN_HAND;i++){
    int card_id = hand[i];
    for(bool is_SP_attack:{false,true}){
      //SPポイントが不足していたら走査しない
      //if(is_SP_attack && (is_placement_P1 ? SP_point_P1:SP_point_P2) < cards[card_id].SP_COST) continue;
      if(is_SP_attack && !is_enough_SP_point(is_placement_P1,card_id)) continue;
      for(int status_id=(include_pass ? -1:0);status_id<stage::card_status_size[card_id];status_id++){
        if(is_valid_placement(is_placement_P1,card_id,status_id,is_SP_attack)){
          valid_choices[i][is_SP_attack].emplace_back(card_id,status_id,is_SP_attack);
        }
      }
    }
  }
  return valid_choices;
}

template<class stage> void Board<stage>::put_both_cards_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1,const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2){
  put_both_cards_without_validation({card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1},{card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2});
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const Choice<stage> choice_P1,const Choice<stage> choice_P2){
  put_both_cards_without_validation(
    choice_P1.card_id,choice_P1.status_id,choice_P1.is_SP_attack,
    choice_P2.card_id,choice_P2.status_id,choice_P2.is_SP_attack
  );
}
template<class stage> void Board<stage>::put_both_cards_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1,const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2){
  //status_id_P1,status_id_P2が-1の時はパス
  const std::bitset<N_SQUARE> &card_covered_square_P1 = stage::card_covered_square[card_id_P1][status_id_P1];
  const std::bitset<N_SQUARE> &card_covered_square_normal_P1 = stage::card_covered_square_normal[card_id_P1][status_id_P1];
  const std::bitset<N_SQUARE> &card_covered_square_SP_P1 = stage::card_covered_square_SP[card_id_P1][status_id_P1];
  const std::bitset<N_SQUARE> &card_covered_square_P2 = stage::card_covered_square[card_id_P2][status_id_P2];
  const std::bitset<N_SQUARE> &card_covered_square_normal_P2 = stage::card_covered_square_normal[card_id_P2][status_id_P2];
  const std::bitset<N_SQUARE> &card_covered_square_SP_P2 = stage::card_covered_square_SP[card_id_P2][status_id_P2];
  //SPアタックの時の前処理
  //スペシャルアタックが突っ込まれる通常マスを空にする
  if(is_SP_attack_P1){
    SP_point_used_P1 += cards[card_id_P1].SP_COST;
    //square_P2_specialが取り返されることはないので、square_P2だけ空きマスで塗り返す
    square_P2 &= ~card_covered_square_P1;
  }
  if(is_SP_attack_P2){
    SP_point_used_P2 += cards[card_id_P2].SP_COST;
    //square_P1_specialが取り返されることはないので、square_P1だけ空きマスで塗り返す
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
  else if(cards[card_id_P1].N_SQUARE == cards[card_id_P2].N_SQUARE){
    //SPとSPまたは通常と通常が衝突したとき、壁となる
    wall_square |= (card_covered_square_SP_P1&card_covered_square_SP_P2)|(card_covered_square_normal_P1&card_covered_square_normal_P2);
    //壁以外を処理
    square_SP_P1 |= card_covered_square_SP_P1&(~card_covered_square_SP_P2);
    square_SP_P2 |= card_covered_square_SP_P2&(~card_covered_square_SP_P1);
    square_P1 |= square_SP_P1|(card_covered_square_normal_P1&(~card_covered_square_P2));
    square_P2 |= square_SP_P2|(card_covered_square_normal_P2&(~card_covered_square_P1));
  }
  //P1のカードの方が面積が小さい場合
  else if(cards[card_id_P1].N_SQUARE < cards[card_id_P2].N_SQUARE){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    std::bitset<N_SQUARE> added_square,removed_square;
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
  else if(cards[card_id_P1].N_SQUARE > cards[card_id_P2].N_SQUARE){
    //面積小SP>面積大SP>面積小通常>面積大通常の順にマスを取っていく
    //取られたマスはremoved_squareに保管
    std::bitset<N_SQUARE> added_square,removed_square;
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
  std::bitset<N_SQUARE> are_there_8_blocks_nearby;
  are_there_8_blocks_nearby = is_there_a_block_nearby[0];
  for(int i=1;i<8;i++) are_there_8_blocks_nearby &= is_there_a_block_nearby[i];
  //square_SP_P1,square_SP_P2と照合
  square_SP_burning_P1 = square_SP_P1&are_there_8_blocks_nearby;
  square_SP_burning_P2 = square_SP_P2&are_there_8_blocks_nearby;

  //hard_squareとall_squareを更新
  hard_square = square_SP_P1|square_SP_P2|wall_square;
  all_square = square_P1|square_P2|wall_square;

  //SPポイントを更新
  SP_point_P1 = square_SP_burning_P1.count()+pass_time_P1-SP_point_used_P1;
  SP_point_P2 = square_SP_burning_P2.count()+pass_time_P2-SP_point_used_P2;
  //used_cards_P1とused_cards_P2を更新
  used_cards_P1[current_turn] = card_id_P1;
  used_cards_P2[current_turn] = card_id_P2;
  //ターン数を更新
  current_turn++;

  //アサーション
  assert((square_P1&square_SP_P1) == square_SP_P1);
  assert((square_P2&square_SP_P2) == square_SP_P2);
  assert(((square_P1|square_SP_P1)&(square_P2|square_SP_P2)).count() == 0);
  assert((square_SP_burning_P1&square_SP_P1) == square_SP_burning_P1);
  assert((square_SP_burning_P2&square_SP_P2) == square_SP_burning_P2);
}

template<class stage> void Board<stage>::put_P1_card_without_validation(const int card_id_P1,const int card_direction_P1,const int card_pos_H_P1,const int card_pos_W_P1,const bool is_pass_P1,const bool is_SP_attack_P1){
  int status_id_P1 = (is_pass_P1 ? -1:stage::card_direction_and_place_to_id[card_id_P1][card_direction_P1][card_pos_H_P1][card_pos_W_P1]);
  put_P1_card_without_validation(card_id_P1,status_id_P1,is_SP_attack_P1);
}
template<class stage> void Board<stage>::put_P1_card_without_validation(const int card_id_P1,const int status_id_P1,const bool is_SP_attack_P1){
  const std::bitset<N_SQUARE> &card_covered_square_P1 = stage::card_covered_square[card_id_P1][status_id_P1];
  const std::bitset<N_SQUARE> &card_covered_square_SP_P1 = stage::card_covered_square_SP[card_id_P1][status_id_P1];
  //SPアタックの時の前処理
  if(is_SP_attack_P1){
    SP_point_used_P1 += cards[card_id_P1].SP_COST;
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
  std::bitset<N_SQUARE> are_there_8_blocks_nearby;
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
template<class stage> void Board<stage>::put_P1_card_without_validation(const Choice<stage> choice_P1){
  put_P1_card_without_validation(choice_P1.card_id,choice_P1.status_id,choice_P1.is_SP_attack);
}

template<class stage> void Board<stage>::put_P2_card_without_validation(const int card_id_P2,const int card_direction_P2,const int card_pos_H_P2,const int card_pos_W_P2,const bool is_pass_P2,const bool is_SP_attack_P2){
  int status_id_P2 = (is_pass_P2 ? -1:stage::card_direction_and_place_to_id[card_id_P2][card_direction_P2][card_pos_H_P2][card_pos_W_P2]);
  put_P2_card_without_validation(card_id_P2,status_id_P2,is_SP_attack_P2);
}
template<class stage> void Board<stage>::put_P2_card_without_validation(const int card_id_P2,const int status_id_P2,const bool is_SP_attack_P2){
  const std::bitset<N_SQUARE> &card_covered_square_P2 = stage::card_covered_square[card_id_P2][status_id_P2];
  const std::bitset<N_SQUARE> &card_covered_square_SP_P2 = stage::card_covered_square_SP[card_id_P2][status_id_P2];
  //SPアタックの時の前処理
  if(is_SP_attack_P2){
    SP_point_used_P2 += cards[card_id_P2].SP_COST;
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
  std::bitset<N_SQUARE> are_there_8_blocks_nearby;
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
template<class stage> void Board<stage>::put_P2_card_without_validation(const Choice<stage> choice_P2){
  put_P2_card_without_validation(choice_P2.card_id,choice_P2.status_id,choice_P2.is_SP_attack);
}

template<class stage> int Board<stage>::square_count_P1() const {return square_P1.count();}
template<class stage> int Board<stage>::square_count_P2() const {return square_P2.count();}

template<class stage> float Board<stage>::get_final_value(float diff_bonus) const {
  assert(current_turn > TURN_MAX);
  int square_diff = square_count_P1()-square_count_P2();
  return std::clamp(square_diff,-1,1)+square_diff*diff_bonus;
}

template<class stage> void Board<stage>::show() const {
  const std::string color_normal_P1 = "\x1b[38;2;237;248;81m";
  const std::string color_SP_P1 = "\x1b[38;2;243;163;58m";
  const std::string color_SP_burning_P1 = "\x1b[38;2;255;255;89m";
  const std::string color_normal_P2 = "\x1b[38;2;77;91;246m";
  const std::string color_SP_P2 = "\x1b[38;2;117;239;252m";
  const std::string color_SP_burning_P2 = "\x1b[38;2;244;255;255m";
  const std::string wall_color = "\x1b[38;2;216;216;216m";
  const std::string empty_color = "\x1b[38;2;20;15;39m";
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      std::string now_color = empty_color;
      std::string block_char = "■";
      if(stage::exists_square[i][j]){
        //現在のマスが何番目か
        int now_order = stage::place_to_order[i][j];
        if(square_SP_P1[now_order]) now_color = color_SP_P1;
        else if(square_SP_P2[now_order]) now_color = color_SP_P2;
        else if(wall_square[now_order]) now_color = wall_color;
        else if(square_P1[now_order]) now_color = color_normal_P1;
        else if(square_P2[now_order]) now_color = color_normal_P2;
        else now_color = empty_color;
        if(square_SP_burning_P1[now_order] || square_SP_burning_P2[now_order]) block_char = "S";
      }
      std::cout << now_color << block_char << "\x1b[m";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}