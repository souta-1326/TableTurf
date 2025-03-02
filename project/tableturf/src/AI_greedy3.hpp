#pragma once
#include <utility>
#include "agent.hpp"
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "xorshift64.hpp"
//常に手札からランダムに選んでパスするAI
template<class stage> class AI_greedy : public Agent<stage>{
 public:
  AI_greedy(){};
  bool redraw(const Deck &deck) override {return false;}
  void set_deck_P1(const Deck &deck_P1) override {}
  void set_deck_P2(const Deck &deck_P2) override {}
  Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) override;
};
template<class stage> Choice<stage> AI_greedy<stage>::get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  std::vector<std::vector<std::vector<Choice<stage>>>> valid_choices = board_P1.get_valid_choices(true,deck,false);
  //カードはランダムに選ぶ
  int card_index = xorshift64()%4;

  int choice_SP_off_size = valid_choices[card_index][0].size();
  int choice_SP_on_size = valid_choices[card_index][1].size();

  //パス以外択がなかったらパス
  if(choice_SP_off_size == 0 && choice_SP_on_size == 0){
    return {deck[card_index],-1,0};
  }

  //通常置きとSPアタックを、それぞれの可能行動数に応じた確率で選択する
  int choice_index = xorshift64()%(choice_SP_off_size+choice_SP_on_size);

  //通常置きはランダムに選ぶ
  if(choice_index < choice_SP_off_size) return valid_choices[card_index][0][choice_index];
  //SP攻撃は、相手がパスしたと仮定した場合に自分のマスが最も多くなるように貪欲に選ぶ
  else{
    int max_score = INT_MIN,max_score_index = -1;
    for(int i=0;i<choice_SP_on_size;i++){
      Board<stage> simulated_board_P1 = board_P1;
      simulated_board_P1.put_P1_card_without_validation(valid_choices[card_index][1][i]);
      int now_square_count_diff = simulated_board_P1.square_count_P1()-simulated_board_P1.square_count_P2();
      int now_score = now_square_count_diff*65536+xorshift64()%65536;//同率の中でランダムに選ぶ
      if(max_score < now_score){
        max_score = now_score;
        max_score_index = i;
      }
    }
    assert(max_score_index != -1);
    return valid_choices[card_index][1][max_score_index];
  }
}

template<class stage> Choice<stage> get_action_greedy(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  static AI_greedy<stage> agent;
  return agent.get_action(board_P1,board_P2,deck);
}