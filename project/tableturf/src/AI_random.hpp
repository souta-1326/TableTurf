#pragma once
#include <utility>
#include "agent.hpp"
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "xorshift64.hpp"
//常に手札からランダムに選んでパスするAI
template<class stage> class AI_random : public Agent<stage>{
 public:
  AI_random(){};
  bool change_hand(const Deck &deck) override {return false;}
  Choice<stage> get_action(const Board<stage> &board,const Deck &deck) override;
};
template<class stage> Choice<stage> AI_random<stage>::get_action(const Board<stage> &board,const Deck &deck){
  std::vector<std::vector<std::vector<Choice<stage>>>> valid_choices = board.get_valid_choices(true,deck);
  //カードをランダムに選ぶ
  int card_index = xorshift64()%4;
  //その中でランダムに択を選ぶ
  int choice_SP_off_size = valid_choices[card_index][0].size();
  int choice_SP_on_size = valid_choices[card_index][1].size();
  int choice_index = xorshift64()%(choice_SP_off_size+choice_SP_on_size);
  if(choice_index < choice_SP_off_size) return valid_choices[card_index][0][choice_index];
  else return valid_choices[card_index][1][choice_index-choice_SP_off_size];
}