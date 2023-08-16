#pragma once
#include <utility>
#include <string.h>
#include <cassert>
#include <algorithm>
#include "xorshift64.hpp"
class Deck{
  //デッキのカードの枚数
  static constexpr int N_card_in_deck = 15;
  //手札のカードの枚数
  static constexpr int N_card_in_hand = 4;
  //デッキの各カードのID
  int card_id_in_deck[N_card_in_deck];
  //現在のターン数
  int current_turn;
public:
  Deck(std::initializer_list<int> init);
  //デッキをシャッフルし、リセット
  void reset();
  //手札を添字で選んで捨てる
  void choose_card_by_index(int index);
  //手札をcard_idで選んで捨てる
  void choose_card_by_card_id(int card_id);
  const int &operator[](int index) const {return card_id_in_deck[index];}
  int &operator[](int index) {return card_id_in_deck[index];}
};
Deck::Deck(std::initializer_list<int> init){
  memcpy(card_id_in_deck,init.begin(),sizeof(card_id_in_deck));
}
void Deck::reset(){
  //カードをシャッフル
  for(int i=0;i<N_card_in_deck-1;i++){
    std::swap(card_id_in_deck[i],card_id_in_deck[i+xorshift64()%(N_card_in_deck-i)]);
  }
  //ターン数をリセット
  current_turn = 1;
}
void Deck::choose_card_by_index(int index){
  assert(0 <= index && index < N_card_in_hand);
  //選んだ手札と(4+ターン数)枚目のカードを入れ替えて、入れ替えたカードを新しい手札とする
  //最終ターンでは手札を入れ替えない
  if(current_turn < 12) std::swap(card_id_in_deck[index],card_id_in_deck[N_card_in_hand+current_turn-1]);
  //ターン数に1を足す
  current_turn++;
}
void Deck::choose_card_by_card_id(int card_id){
  int index = std::find(card_id_in_deck,card_id_in_deck+N_card_in_hand,card_id)-card_id_in_deck;
  assert(index < N_card_in_hand);
  choose_card_by_index(index);
}