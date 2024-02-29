#pragma once
#include <iostream>
#include <algorithm>
#include <omp.h>
#include "board.hpp"
#include "stage.hpp"
#include "deck.hpp"
#include "choice.hpp"
#include "common.hpp"
#include "AI_ISMCTS.hpp"
#include "print_board_log.hpp"
//単なるISMCTS同士で戦わせる
template<class stage> float testplay2(
 int num_games,int num_threads,
 int simple_ISMCTS_num_simulations,float diff_bonus,
 std::vector<Deck> deck_P1s,std::vector<Deck> deck_P2s,
 bool logging = false
 ){
  assert(deck_P1s.size() == num_games && deck_P2s.size() == num_games);
  assert(num_games % num_threads == 0);
  float win_count = 0;
  #pragma omp parallel num_threads(num_threads)
  for(int loop=0;loop<num_games/num_threads;loop++){
    AI_ISMCTS<stage> agent_P1(simple_ISMCTS_num_simulations,diff_bonus,get_action_greedy<stage>,logging);
    AI_ISMCTS<stage> agent_P2(simple_ISMCTS_num_simulations,diff_bonus,get_action_greedy<stage>,logging);
    Board<stage> board_P1,board_P2;
    Deck deck_P1 = deck_P1s[loop*num_threads+omp_get_thread_num()];
    Deck deck_P2 = deck_P2s[loop*num_threads+omp_get_thread_num()];

    //set_deck
    agent_P1.set_deck_P1(deck_P1);
    agent_P1.set_deck_P2(deck_P2);
    agent_P2.set_deck_P1(deck_P2);
    agent_P2.set_deck_P2(deck_P1);

    //試合開始

    //デッキをシャッフル
    deck_P1.reset();
    deck_P2.reset();

    //redraw phase
    //std::cerr << 0;
    if(agent_P1.redraw(deck_P1)) deck_P1.reset();
    if(agent_P2.redraw(deck_P2)) deck_P2.reset();

    if(logging) print_board_log(board_P1,deck_P1,deck_P2);
    //action phase
    for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
      //std::cerr << current_turn;
      Choice<stage> choice_P1 = agent_P1.get_action(board_P1,board_P2,deck_P1);
      Choice<stage> choice_P2 = agent_P2.get_action(board_P2,board_P1,deck_P2);

      //カードを置く
      assert(board_P1.is_valid_placement(true,choice_P1));
      assert(board_P1.is_valid_placement(false,choice_P2.swap_player()));
      assert(board_P2.is_valid_placement(true,choice_P2));
      assert(board_P2.is_valid_placement(false,choice_P1.swap_player()));

      board_P1.put_both_cards_without_validation(choice_P1,choice_P2.swap_player());
      board_P2.put_both_cards_without_validation(choice_P2,choice_P1.swap_player());

      //デッキの処理
      deck_P1.choose_card_by_card_id(choice_P1.card_id);
      deck_P2.choose_card_by_card_id(choice_P2.card_id);
      
      if(logging) print_board_log(board_P1,deck_P1,deck_P2);
    }

    //勝敗判定
    float value_P1 = board_P1.get_final_value(diff_bonus);
    #pragma omp critical
    {
      win_count += value_P1;
    }
  }
  // for(int loop=0;loop<num_games/num_threads;loop++){
  //   std::vector<AI_ISMCTS<stage>> agent_P1s(num_threads,AI_ISMCTS<stage>(simple_ISMCTS_num_simulations,diff_bonus,get_action_greedy<stage>,logging));
  //   std::vector<AI_ISMCTS<stage>> agent_P2s(num_threads,AI_ISMCTS<stage>(simple_ISMCTS_num_simulations,diff_bonus,get_action_greedy<stage>,logging));
  //   std::vector<Board<stage>> board_P1s(num_threads),board_P2s(num_threads);
  //   std::vector<Deck> current_deck_P1s(num_threads),current_deck_P2s(num_threads);
  //   for(int i=0;i<num_threads;i++){
  //     current_deck_P1s[i] = deck_P1s[loop*num_threads+i];
  //     current_deck_P2s[i] = deck_P2s[loop*num_threads+i];
  //   }

  //   //set_deck
  //   for(int i=0;i<num_threads;i++){
  //     agent_P1s[i].set_deck_P1(current_deck_P1s[i]);
  //     agent_P1s[i].set_deck_P2(current_deck_P2s[i]);
  //     agent_P2s[i].set_deck_P1(current_deck_P2s[i]);
  //     agent_P2s[i].set_deck_P2(current_deck_P1s[i]);
  //   }

  //   //試合開始

  //   //デッキをシャッフル
  //   for(Deck &deck_P1:current_deck_P1s) deck_P1.reset();
  //   for(Deck &deck_P2:current_deck_P2s) deck_P2.reset();

  //   //redraw phase
  //   #pragma omp parallel for num_threads(num_threads)
  //   for(int i=0;i<num_threads;i++){
  //     std::cerr << 0;
  //     if(agent_P1s[i].redraw(current_deck_P1s[i])) current_deck_P1s[i].reset();
  //     if(agent_P2s[i].redraw(current_deck_P2s[i])) current_deck_P2s[i].reset();
  //   }

  //   if(logging) print_board_log(board_P1s[0],current_deck_P1s[0],current_deck_P2s[0]);

  //   //action phase
  //   for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
  //     std::vector<Choice<stage>> choice_P1s(num_threads,{-1,-1,false});
  //     std::vector<Choice<stage>> choice_P2s(num_threads,{-1,-1,false});
  //     #pragma omp parallel for num_threads(num_threads)
  //     for(int i=0;i<num_threads;i++){
  //       std::cerr << current_turn;
  //       choice_P1s[i] = agent_P1s[i].get_action(board_P1s[i],board_P2s[i],current_deck_P1s[i]);
  //       choice_P2s[i] = agent_P2s[i].get_action(board_P2s[i],board_P1s[i],current_deck_P2s[i]);
  //     }

  //     for(int i=0;i<num_threads;i++){
  //       //カードを置く
  //       assert(board_P1s[i].is_valid_placement(true,choice_P1s[i]));
  //       assert(board_P1s[i].is_valid_placement(false,choice_P2s[i].swap_player()));
  //       assert(board_P2s[i].is_valid_placement(true,choice_P2s[i]));
  //       assert(board_P2s[i].is_valid_placement(false,choice_P1s[i].swap_player()));

  //       board_P1s[i].put_both_cards_without_validation(choice_P1s[i],choice_P2s[i].swap_player());
  //       board_P2s[i].put_both_cards_without_validation(choice_P2s[i],choice_P1s[i].swap_player());

  //       //デッキの処理
  //       current_deck_P1s[i].choose_card_by_card_id(choice_P1s[i].card_id);
  //       current_deck_P2s[i].choose_card_by_card_id(choice_P2s[i].card_id);
  //     }

  //     if(logging) print_board_log(board_P1s[0],current_deck_P1s[0],current_deck_P2s[0]);
  //   }
  //   //勝敗判定
  //   for(int i=0;i<num_threads;i++){
  //     float value_P1 = board_P1s[i].get_final_value(diff_bonus);
  //     win_count += value_P1;
  //   }
  // }

  return win_count/num_games;
}