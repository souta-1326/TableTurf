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