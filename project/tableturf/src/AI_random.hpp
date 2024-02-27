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
  bool redraw(const Deck &deck) override {return false;}
  void set_deck_P1(const Deck &deck_P1) override {}
  void set_deck_P2(const Deck &deck_P2) override {}
  Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) override;
};
template<class stage> Choice<stage> AI_random<stage>::get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  std::vector<std::vector<std::vector<Choice<stage>>>> valid_choices = board_P1.get_valid_choices(true,deck,false);
  //カードをランダムに選ぶ
  int card_index = xorshift64()%4;
  //その中でランダムに択を選ぶ
  int choice_SP_off_size = valid_choices[card_index][0].size();
  int choice_SP_on_size = valid_choices[card_index][1].size();
  //パス以外択がなかったらパス
  if(choice_SP_off_size == 0 && choice_SP_on_size == 0){
    return {deck[card_index],-1,0};
  }
  int choice_index = xorshift64()%(choice_SP_off_size+choice_SP_on_size);
  if(choice_index < choice_SP_off_size) return valid_choices[card_index][0][choice_index];
  else return valid_choices[card_index][1][choice_index-choice_SP_off_size];
}

template<class stage> Choice<stage> get_action_random(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  static AI_random<stage> agent;
  return agent.get_action(board_P1,board_P2,deck);
}