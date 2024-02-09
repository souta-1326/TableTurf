#pragma once
#include <iostream>
#include <string>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "agent.hpp"
template<class stage> void print_board_log(const Board<stage> &board,const Deck &deck_P1,const Deck &deck_P2){
  std::cout << "Turn " << board.current_turn << ":\n";
  
  //Player1
  {
  std::cout << "Player 1:\n";
  auto valid_choices = board.get_valid_choices(true,deck_P1);
  std::vector<int> deck_P1_hand = deck_P1.get_hand();

  std::cout << "Score:" << board.square_count_P1() << std::endl;
  std::cout << "SP:" << board.SP_point_P1 << std::endl; 

  std::cout << "Hand:\n";
  for(int card_index=0;card_index<Deck::N_CARD_IN_HAND;card_index++){
    int card_id = deck_P1_hand[card_index];
    int choice_SP_off_size = valid_choices[card_index][0].size();
    int choice_SP_on_size = valid_choices[card_index][1].size();
    std::string card_id_str = std::to_string(card_id);
    card_id_str = std::string(3-card_id_str.size(),'0')+card_id_str;
    std::string choice_SP_off_size_str = std::to_string(choice_SP_off_size);
    choice_SP_off_size_str = std::string(3-choice_SP_off_size_str.size(),'0')+choice_SP_off_size_str;
    std::string choice_SP_on_size_str = std::to_string(choice_SP_on_size);
    choice_SP_on_size_str = std::string(3-choice_SP_on_size_str.size(),'0')+choice_SP_on_size_str;

    std::cout << card_id_str << ":" << choice_SP_off_size_str << ":" << choice_SP_on_size_str << std::endl;
  }
  }
  //Player2
  {
  std::cout << "Player 2:\n";
  auto valid_choices = board.get_valid_choices(false,deck_P2);
  std::vector<int> deck_P2_hand = deck_P2.get_hand();

  std::cout << "Score:" << board.square_count_P2() << std::endl;
  std::cout << "SP:" << board.SP_point_P2 << std::endl; 

  std::cout << "Hand:\n";
  for(int card_index=0;card_index<Deck::N_CARD_IN_HAND;card_index++){
    int card_id = deck_P2_hand[card_index];
    int choice_SP_off_size = valid_choices[card_index][0].size();
    int choice_SP_on_size = valid_choices[card_index][1].size();
    std::string card_id_str = std::to_string(card_id);
    card_id_str = std::string(3-card_id_str.size(),'0')+card_id_str;
    std::string choice_SP_off_size_str = std::to_string(choice_SP_off_size);
    choice_SP_off_size_str = std::string(3-choice_SP_off_size_str.size(),'0')+choice_SP_off_size_str;
    std::string choice_SP_on_size_str = std::to_string(choice_SP_on_size);
    choice_SP_on_size_str = std::string(3-choice_SP_on_size_str.size(),'0')+choice_SP_on_size_str;
    
    std::cout << card_id_str << ":" << choice_SP_off_size_str << ":" << choice_SP_on_size_str << std::endl;
  }
  }
  std::cout << std::endl;
}
template<class stage,class C1,class C2> std::vector<Board<stage>> Battle(C1 &agent_P1,C2 &agent_P2,Deck deck_P1,Deck deck_P2){
  //deckのchoose_card_by_なんとかを忘れない
  //board_P1はP1視点のboard,board_P2はP2視点のboard
  Board<stage> board_P1,board_P2;
  std::vector<Board<stage>> board_P1_log(Board<stage>::TURN_MAX+1);//board_P1のログ
  
  //agentに互いのデッキを教える
  agent_P1.set_deck_P1(deck_P1);
  agent_P1.set_deck_P2(deck_P2);
  agent_P2.set_deck_P1(deck_P2);
  agent_P2.set_deck_P2(deck_P1);
  //試合開始

  //デッキをシャッフル
  deck_P1.reset();
  deck_P2.reset();
  //デッキの手札を任意で1回のみ入れ替える
  if(agent_P1.redraw(deck_P1)) deck_P1.reset();
  if(agent_P2.redraw(deck_P2)) deck_P2.reset();
  board_P1_log[0] = board_P1;
  //ログ出力
  print_board_log(board_P1,deck_P1,deck_P2);
  for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
    Choice<stage> choice_P1 = agent_P1.get_action(board_P1,board_P2,deck_P1);
    Choice<stage> choice_P2 = agent_P2.get_action(board_P2,board_P1,deck_P2);
    assert(board_P1.is_valid_placement(true,choice_P1));
    assert(board_P1.is_valid_placement(false,choice_P2.swap_player()));
    //board_P2視点でP1は相手
    assert(board_P2.is_valid_placement(false,choice_P1.swap_player()));
    assert(board_P2.is_valid_placement(true,choice_P2));

    //カードを置く
    board_P1.put_both_cards_without_validation(choice_P1,choice_P2.swap_player());
    board_P2.put_both_cards_without_validation(choice_P2,choice_P1.swap_player());

    //デッキの処理
    deck_P1.choose_card_by_card_id(choice_P1.card_id);
    deck_P2.choose_card_by_card_id(choice_P2.card_id);

    //盤面ログ
    board_P1_log[current_turn] = board_P1;

    //ログ出力
    print_board_log(board_P1,deck_P1,deck_P2);
  }

  return board_P1_log;
}