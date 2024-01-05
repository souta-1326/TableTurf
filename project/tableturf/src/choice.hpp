#pragma once
#include "stage_database.hpp"
#include "card_database.hpp"
template<class stage> struct Choice{
  int card_id;int status_id;bool is_SP_attack;
  Choice(const int card_id,const int status_id,const bool is_SP_attack):card_id(card_id),status_id(status_id),is_SP_attack(is_SP_attack){}
  Choice(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack);
  //相手視点のChoiceを生成する(非破壊的処理)
  Choice<stage> swap_player();
};

template<class stage> Choice<stage>::Choice(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack):
card_id(card_id),is_SP_attack(is_SP_attack){
  assert(!(is_pass && is_SP_attack));//パスなのにSPアタックをしていたらエラー
  if(is_pass) status_id = -1;
  else{
    status_id = stage::card_direction_and_place_to_id[card_id][card_direction][card_pos_H][card_pos_W];
    assert(status_id != -1);//不正な置き方を検知
  }
}

template<class stage> Choice<stage> Choice<stage>::swap_player(){
  //パス(status_id==-1)ならそのまま
  if(status_id == -1) return *this;
  //status_idから、1P視点の(カードの向き、左上の行、左上の列)を求める
  auto [direction,pos_H,pos_W] = stage::card_status[card_id][status_id];
  //directionを反転
  direction = (direction+2)%4;
  //pos_H,pos_Wの反転 右下から左上へのシフト
  pos_H = (stage::h-pos_H-1)-((direction%2==0 ? cards[card_id].H:cards[card_id].W)-1);
  pos_W = (stage::w-pos_W-1)-((direction%2==0 ? cards[card_id].W:cards[card_id].H)-1);
  //status_idを求める
  return {card_id,stage::card_direction_and_place_to_id[card_id][direction][pos_H][pos_W],is_SP_attack};
}