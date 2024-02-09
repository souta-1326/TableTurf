#pragma once
#include <utility>
#include <vector>
#include <string.h>
#include <cassert>
#include <algorithm>
#include "xorshift64.hpp"
class Deck{
public:
  //デッキのカードの枚数
  static constexpr int N_CARD_IN_DECK = 15;
  //手札のカードの枚数
  static constexpr int N_CARD_IN_HAND = 4; 
  //最大ターン数
  static constexpr int TURN_MAX = 12;
private:
  //デッキの各カードのID 先頭の4枚が手札
  //int card_id_in_deck[N_CARD_IN_DECK];
  std::vector<int> card_id_in_deck;
public:
  //現在のターン数
  int current_turn;
public:
  //デフォルトはスターターデッキ
  Deck(const std::vector<int> &deck = std::vector<int>({6,13,22,28,40,34,45,52,55,56,159,137,141,103,92}));
  //used_cardsを設定
  Deck(const std::vector<int> &deck,const std::vector<int> &used_cards);
  Deck(const Deck &deck,const std::vector<int> &used_cards);
  //デッキをシャッフルし、リセット
  void reset();
  //デッキをリセットし、最初の手札4枚を指定
  void reset(const std::vector<int> &hand);
  //手札を添字で選んで捨てる
  void choose_card_by_index(int index,int next_card_id = -1);
  //手札をcard_idで選んで捨てる
  void choose_card_by_card_id(int card_id,int next_card_id = -1);
  //デッキ
  std::vector<int> get_deck() const {return card_id_in_deck;}
  //手札
  std::vector<int> get_hand() const {return {card_id_in_deck.begin(),card_id_in_deck.begin()+N_CARD_IN_HAND};}
  //山札(使用済み札除く)
  std::vector<int> get_stock() const {return {card_id_in_deck.begin()+N_CARD_IN_HAND,card_id_in_deck.begin()+(N_CARD_IN_DECK-current_turn+1)};}
  //使用済み札
  std::vector<int> get_used_card() const {return {card_id_in_deck.begin()+(N_CARD_IN_DECK-current_turn+1),card_id_in_deck.end()};}

  const int &operator[](int index) const {return card_id_in_deck[index];}
  int &operator[](int index) {return card_id_in_deck[index];}
};
Deck::Deck(const std::vector<int> &deck):card_id_in_deck(deck){
  assert(card_id_in_deck.size() == N_CARD_IN_DECK);
}
Deck::Deck(const std::vector<int> &deck,const std::vector<int> &used_cards):card_id_in_deck(deck),current_turn(1){
  assert(card_id_in_deck.size() == N_CARD_IN_DECK);
  current_turn = 1;
  for(int i=0;i<N_CARD_IN_DECK-current_turn+1;){
    if(std::find(used_cards.begin(),used_cards.end(),card_id_in_deck[i]) != used_cards.end()){
      std::swap(card_id_in_deck[i],card_id_in_deck[N_CARD_IN_DECK-current_turn]);
      current_turn++;
    }
    else i++;
  }
  for(int i=0;i<N_CARD_IN_HAND;i++){
    std::swap(card_id_in_deck[i],card_id_in_deck[i+xorshift64()%(N_CARD_IN_DECK-i-(current_turn-1))]);
  }
}
Deck::Deck(const Deck &deck,const std::vector<int> &used_cards){
  *this = Deck(deck.get_deck(),used_cards);
}
void Deck::reset(){
  //手札をシャッフル
  for(int i=0;i<N_CARD_IN_HAND;i++){
    std::swap(card_id_in_deck[i],card_id_in_deck[i+xorshift64()%(N_CARD_IN_DECK-i)]);
  }
  //ターン数をリセット
  current_turn = 1;
}
void Deck::reset(const std::vector<int> &hand){
  //handを手札に持ってくる
  for(int i=0;i<N_CARD_IN_HAND;i++){
    int target_index = std::find(card_id_in_deck.begin(),card_id_in_deck.end(),hand[i])-card_id_in_deck.begin();
    assert(target_index < N_CARD_IN_DECK);
    std::swap(card_id_in_deck[i],card_id_in_deck[target_index]);
  }
  //ターン数をリセット
  current_turn = 1;
}
void Deck::choose_card_by_index(int index,int next_card_id){
  assert(0 <= index && index < N_CARD_IN_HAND);
  //選んだ手札を4~(11-current_turn)番目までの手札のどれかと入れ替える
  //選んだ手札はcard_id_in_deckの右に詰めて入れる
  //最終ターンでは手札を入れ替えない
  //1ターン目 4+xorshift64()%(15-4-1+1) 4~14
  if(current_turn < 12){
    // next_card_id が指定されている場合は探す
    int next_card_index = 
    (next_card_id == -1 ? 
    N_CARD_IN_HAND+xorshift64()%(N_CARD_IN_DECK-N_CARD_IN_HAND-current_turn+1):
    std::find(card_id_in_deck.begin(),card_id_in_deck.end(),next_card_id)-card_id_in_deck.begin());

    assert(N_CARD_IN_HAND <= next_card_index && next_card_index < N_CARD_IN_DECK-current_turn+1);
    
    //まず、使った手札と次の手札を入れ替え、
    std::swap(card_id_in_deck[index],card_id_in_deck[next_card_index]);
    //使った手札を奥にしまう
    std::swap(card_id_in_deck[next_card_index],card_id_in_deck[N_CARD_IN_DECK-current_turn]);
  }
  //ターン数に1を足す
  current_turn++;
}
void Deck::choose_card_by_card_id(int card_id,int next_card_id){
  int index = std::find(card_id_in_deck.begin(),card_id_in_deck.begin()+N_CARD_IN_HAND,card_id)-card_id_in_deck.begin();
  assert(index < N_CARD_IN_HAND);
  choose_card_by_index(index,next_card_id);
}