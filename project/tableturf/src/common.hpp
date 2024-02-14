#pragma once
#include "choice.hpp"
#include "card.hpp"

constexpr int INPUT_C = 2+2+1+12+12*2+N_card*2;
template<class stage> constexpr int ACTION_SPACE_OF_EACH_CARD = stage::h*stage::w*8+1;


template<class stage> int choice_to_policy_action_index(const Choice<stage> choice){
  if(choice.status_id == -1) return (choice.card_id-1)*ACTION_SPACE_OF_EACH_CARD<stage>;
  else{
    auto [direction,h_index,w_index] = stage::card_status[choice.card_id][choice.status_id];
    return
    (choice.card_id-1)*ACTION_SPACE_OF_EACH_CARD<stage>+
    (choice.is_SP_attack ? 4*(stage::h*stage::w):0)+
    direction*(stage::h*stage::w)+
    h_index*stage::w+
    w_index
    +1;
  }
}