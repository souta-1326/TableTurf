#pragma once
#include <vector>
#include <algorithm>
#include "card_database.hpp"
#include "deck.hpp"
#include "xorshift64.hpp"
Deck get_random_individual(){
  std::vector<int> card_id_in_deck(Deck::N_CARD_IN_DECK);
  std::vector<short> is_used_card_id(N_card+1);
  //card_idが相異なるようにランダムにデッキを生成
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    while(true){
      int new_card_id = xorshift64()%N_card+1;
      if(!is_used_card_id[new_card_id]){
        card_id_in_deck[i] = new_card_id;
        is_used_card_id[new_card_id] = true;
        break;
      }
    }
  }
  return card_id_in_deck;
}
Deck mutate(const Deck &deck){
  //1枚カードを変更することを繰り返す
  std::vector<int> card_id_in_deck = deck.get_deck();
  do{
    while(true){
      int new_card_id = xorshift64()%N_card+1;
      if(std::count(card_id_in_deck.begin(),card_id_in_deck.end(),new_card_id) == 0){
        int changed_index = xorshift64()%Deck::N_CARD_IN_DECK;
        card_id_in_deck[changed_index] = new_card_id;
        break;
      }
    }
  }while(xorshift64()%2);
  return card_id_in_deck;
}
