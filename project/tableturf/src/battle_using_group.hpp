#pragma once
#include <iostream>
#include <string>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "agent.hpp"
template<class stage,class C_Group> std::vector<Board<stage>> Battle(C_Group agent,Deck deck_P1,Deck deck_P2){
  assert(agent.get_group_size() == 2);
  //deckのchoose_card_by_なんとかを忘れない
  //board_P1はP1視点のboard,board_P2はP2視点のboard
  Board<stage> board_P1,board_P2;
  std::vector<Board<stage>> board_P1_log(Board<stage>::TURN_MAX+1);//board_P1のログ
  
  //agentに互いのデッキを教える
  agent.set_deck_P1({deck_P1,deck_P2});
  agent.set_deck_P2({deck_P2,deck_P1});
  //試合開始

  //デッキをシャッフル
  deck_P1.reset();
  deck_P2.reset();
  //デッキの手札を任意で1回のみ入れ替える
  std::vector<bool> do_redraw_decks = agent.redraws({deck_P1,deck_P2});
  if(do_redraw_decks[0]) deck_P1.reset();
  if(do_redraw_decks[1]) deck_P2.reset();
  board_P1_log[0] = board_P1;
  //ログ出力
  print_board_log(board_P1,deck_P1,deck_P2);
  for(int current_turn = 1;current_turn <= Board<stage>::TURN_MAX;current_turn++){
    std::vector<Choice<stage>> choices = agent.get_actions({board_P1,board_P2},{board_P2,board_P1},{deck_P1,deck_P2});
    Choice<stage> choice_P1 = choices[0],choice_P2 = choices[1];
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