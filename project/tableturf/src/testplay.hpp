#pragma once
#include <iostream>
#include <algorithm>
#include <torch/script.h>
#include <omp.h>
#include "board.hpp"
#include "stage.hpp"
#include "deck.hpp"
#include "choice.hpp"
#include "common.hpp"
#include "AI_ISMCTS.hpp"
#include "AI_PV_ISMCTS_group.hpp"
#include "print_board_log.hpp"
template<class stage> float testplay(
 int num_games,int num_threads,
 const torch::jit::script::Module &model,c10::Device device,c10::ScalarType dtype,
 int PV_ISMCTS_num_simulations,int simple_ISMCTS_num_simulations,float diff_bonus,
 std::vector<Deck> deck_P1s,std::vector<Deck> deck_P2s,
 bool logging = false
 ){
  assert(deck_P1s.size() == num_games && deck_P2s.size() == num_games);

  omp_set_num_threads(num_threads);

  AI_PV_ISMCTS_Group<stage> agent_P1s(num_games,model,device,dtype,PV_ISMCTS_num_simulations,diff_bonus,false,0,0,logging);

  std::vector<AI_ISMCTS<stage>> agent_P2s(num_games,AI_ISMCTS<stage>(simple_ISMCTS_num_simulations,diff_bonus,false,logging));

  std::vector<Board<stage>> board_P1s(num_games),board_P2s(num_games);

  //set_deck
  agent_P1s.set_deck_P1s(deck_P1s);
  agent_P1s.set_deck_P2s(deck_P2s);
  for(int i=0;i<num_games;i++){
    agent_P2s[i].set_deck_P1(deck_P2s[i]);
    agent_P2s[i].set_deck_P2(deck_P1s[i]);
  }

  //試合開始

  //デッキをシャッフル
  for(Deck &deck_P1:deck_P1s) deck_P1.reset();
  for(Deck &deck_P2:deck_P2s) deck_P2.reset();

  //redraw phase
  std::vector<bool> do_redraw_deck_P1s = agent_P1s.redraws(deck_P1s);
  for(int i=0;i<num_games;i++){
    if(do_redraw_deck_P1s[i]) deck_P1s[i].reset();
  }

  #pragma omp parallel for
  for(int i=0;i<num_games;i++){
    if(agent_P2s[i].redraw(deck_P2s[i])) deck_P2s[i].reset();
  }

  if(logging) print_board_log(board_P1s[0],deck_P1s[0],deck_P2s[0]);

  //action phase
  for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
    std::vector<Choice<stage>> choice_P1s = agent_P1s.get_actions(board_P1s,board_P2s,deck_P1s);
    std::vector<Choice<stage>> choice_P2s(num_games,{-1,-1,false});
    #pragma omp parallel for
    for(int i=0;i<num_games;i++){
      choice_P2s[i] = agent_P2s[i].get_action(board_P2s[i],board_P1s[i],deck_P2s[i]);
    }

    for(int i=0;i<num_games;i++){
      //カードを置く
      assert(board_P1s[i].is_valid_placement(true,choice_P1s[i]));
      assert(board_P1s[i].is_valid_placement(false,choice_P2s[i].swap_player()));
      assert(board_P2s[i].is_valid_placement(true,choice_P2s[i]));
      assert(board_P2s[i].is_valid_placement(false,choice_P1s[i].swap_player()));

      board_P1s[i].put_both_cards_without_validation(choice_P1s[i],choice_P2s[i].swap_player());
      board_P2s[i].put_both_cards_without_validation(choice_P2s[i],choice_P1s[i].swap_player());

      //デッキの処理
      deck_P1s[i].choose_card_by_card_id(choice_P1s[i].card_id);
      deck_P2s[i].choose_card_by_card_id(choice_P2s[i].card_id);
    }

    if(logging) print_board_log(board_P1s[0],deck_P1s[0],deck_P2s[0]);
  }

  //勝敗判定し、勝率を返す
  float win_count = 0;
  for(int i=0;i<num_games;i++){
    float value_P1 = board_P1s[i].get_final_value(diff_bonus);
    win_count += value_P1;
  }

  return win_count/num_games;
}