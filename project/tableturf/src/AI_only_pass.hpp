#pragma once
#include <utility>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "xorshift64.hpp"
//常に手札からランダムに選んでパスするAI
template<class stage> Choice AI_only_pass(const Board<stage> &board,const bool is_P1,const Deck &deck){
  assert(board.current_turn == deck.current_turn);
  //0~3の整数をランダムに選んでその添字のカードを選びパスする
  int choose_card_index = xorshift64()%4;
  return Choice{Deck[choose_card_index],-1,false};
}