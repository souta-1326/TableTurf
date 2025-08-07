#pragma once
#include <utility>
#include <climits>
#include "agent.hpp"
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "xorshift64.hpp"

template<class stage> class AI_greedy2 : public Agent<stage>{
 public:
  AI_greedy2(){};
  bool redraw(const Deck &deck) override {return false;}
  void set_deck_P1(const Deck &deck_P1) override {}
  void set_deck_P2(const Deck &deck_P2) override {}
  Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) override;
};
template<class stage> Choice<stage> AI_greedy2<stage>::get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  constexpr float SP_point_value[Board<stage>::TURN_MAX+1] = {12,11.5,10.8,10,9.3,8.5,7.8,7,6,5,4,3,0};
  float current_SP_point_value = SP_point_value[board_P1.get_current_turn()];//SPポイントにこれをかけたものをスコアに足して貪欲

  //カードはランダムに選ぶ
  int card_index = xorshift64()%4;
  int card_id = deck[card_index];

  std::vector<std::vector<Choice<stage>>> valid_choices = board_P1.get_valid_choices(true,card_id,false);

  int choice_SP_off_size = valid_choices[0].size();
  int choice_SP_on_size = valid_choices[1].size();

  //パス以外択がなかったらパス
  if(choice_SP_off_size == 0 && choice_SP_on_size == 0){
    return {deck[card_index],-1,0};
  }

  int max_score = INT_MIN;Choice<stage> best_choice({-1,-1,false});
  for(bool is_SP_attack:{false,true}){
    for(Choice<stage> &choice:valid_choices[is_SP_attack]){
      Board<stage> simulated_board_P1 = board_P1;
      simulated_board_P1.put_P1_card_without_validation(choice);
      int now_square_count_diff = simulated_board_P1.square_count_P1()-simulated_board_P1.square_count_P2();
      int now_SP_point_diff = simulated_board_P1.get_SP_point_P1()-simulated_board_P1.get_SP_point_P2();
      float now_score = now_square_count_diff+now_SP_point_diff*current_SP_point_value+float(xorshift64()%65536)/65536000;//同率の中でランダムに選ぶ
      if(max_score < now_score){
        max_score = now_score;
        best_choice = choice;
      }
    }
  }
  assert(best_choice.card_id != -1);
  return best_choice;
}

template<class stage> Choice<stage> get_action_greedy2(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  static AI_greedy2<stage> agent;
  return agent.get_action(board_P1,board_P2,deck);
}