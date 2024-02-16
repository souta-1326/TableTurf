#pragma once
#include <iostream>
#include <algorithm>
#include <mutex>
#include <torch/script.h>
#include "board.hpp"
#include "stage.hpp"
#include "deck.hpp"
#include "choice.hpp"
#include "buffer.hpp"
#include "common.hpp"
#include "AI_PV_ISMCTS_group.hpp"
#include "print_board_log.hpp"
template<class stage> void selfplay(
 int num_games,
 const torch::jit::script::Module &model,c10::Device device,c10::ScalarType dtype,
 int num_simulations,float dirichlet_alpha,float eps,float diff_bonus,
 std::vector<Deck> deck_P1s,std::vector<Deck> deck_P2s,
 Buffer<stage> &buffer,
 bool logging = false
 ){
  assert(deck_P1s.size() == num_games && deck_P2s.size() == num_games);

  const int group_size = num_games*2;//1ゲーム2つ
  
  std::vector<std::vector<Sample<stage>>> records(group_size);
  AI_PV_ISMCTS_Group<stage>
  agents(group_size,model,device,dtype,num_simulations,diff_bonus,true,dirichlet_alpha,eps);

  std::vector<Board<stage>> board_P1s(num_games),board_P2s(num_games);

  //set_deck
  agents.set_deck_P1s(incorporate(deck_P1s,deck_P2s));
  agents.set_deck_P2s(incorporate(deck_P2s,deck_P1s));

  //試合開始

  //デッキをシャッフル
  for(Deck &deck_P1:deck_P1s) deck_P1.reset();
  for(Deck &deck_P2:deck_P2s) deck_P2.reset();

  //redraw phase
  std::vector<bool> do_redraw_decks = agents.redraws(incorporate(deck_P1s,deck_P2s));
  std::vector<std::vector<float>> policy_redraws = agents.get_policy_redraws();
  std::vector<std::vector<std::pair<Choice<stage>,float>>> policy_actions_network = agents.get_policy_actions_network();
  for(int i=0;i<num_games;i++){
    //recordsを更新
    records[i*2].push_back({board_P1s[i],true,deck_P1s[i],deck_P2s[i],policy_redraws[i*2],construct_policy_action_for_learning(policy_actions_network[i*2]),0.0F});
    records[i*2+1].push_back({board_P2s[i],true,deck_P2s[i],deck_P1s[i],policy_redraws[i*2+1],construct_policy_action_for_learning(policy_actions_network[i*2+1]),0.0F});

    //デッキを操作
    if(do_redraw_decks[i*2]) deck_P1s[i].reset();
    if(do_redraw_decks[i*2+1]) deck_P2s[i].reset();
  }

  if(logging) print_board_log(board_P1s[0],deck_P1s[0],deck_P2s[0]);

  //action phase
  for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
    std::vector<Choice<stage>> choices = 
    agents.get_actions(incorporate(board_P1s,board_P2s),incorporate(board_P2s,board_P1s),incorporate(deck_P1s,deck_P2s));
    std::vector<std::vector<std::pair<Choice<stage>,float>>> policy_actions = agents.get_policy_actions();
    std::vector<std::vector<std::pair<Choice<stage>,float>>> policy_actions_network = agents.get_policy_actions_network();
    for(int i=0;i<num_games;i++){
      //recordsを更新
      records[i*2].push_back({board_P1s[i],false,deck_P1s[i],deck_P2s[i],std::vector<float>(),construct_policy_action_for_learning(policy_actions[i*2],policy_actions_network[i*2],deck_P1s[i]),0.0F});
      records[i*2+1].push_back({board_P2s[i],false,deck_P2s[i],deck_P1s[i],std::vector<float>(),construct_policy_action_for_learning(policy_actions[i*2+1],policy_actions_network[i*2+1],deck_P2s[i]),0.0F});

      //カードを置く
      assert(board_P1s[i].is_valid_placement(true,choices[i*2]));
      assert(board_P1s[i].is_valid_placement(false,choices[i*2+1].swap_player()));
      assert(board_P2s[i].is_valid_placement(true,choices[i*2+1]));
      assert(board_P2s[i].is_valid_placement(false,choices[i*2].swap_player()));

      board_P1s[i].put_both_cards_without_validation(choices[i*2],choices[i*2+1].swap_player());
      board_P2s[i].put_both_cards_without_validation(choices[i*2+1],choices[i*2].swap_player());

      //デッキの処理
      deck_P1s[i].choose_card_by_card_id(choices[i*2].card_id);
      deck_P2s[i].choose_card_by_card_id(choices[i*2+1].card_id);
    }

    if(logging) print_board_log(board_P1s[0],deck_P1s[0],deck_P2s[0]);
  }

  //勝敗判定して、sampleにvalueを代入
  for(int i=0;i<num_games;i++){
    float value_P1 = board_P1s[i].get_final_value(diff_bonus);
    for(Sample<stage> &sample:records[i*2]){
      sample.value = value_P1;
    }
    for(Sample<stage> &sample:records[i*2+1]){
      sample.value = -value_P1;
    }
  }
  //bufferに突っ込む
  static std::mutex buffer_mutex;//bufferの排他制御
  buffer_mutex.lock();
  for(int i=0;i<group_size;i++){
    for(const Sample<stage> &sample:records[i]){
      buffer.add_sample(sample);
    }
  }
  buffer_mutex.unlock();
}