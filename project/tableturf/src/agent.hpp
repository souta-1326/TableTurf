#pragma once
#include <vector>
#include <type_traits>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"

template<class stage> class Agent{
 public:
  virtual bool redraw(const Deck &deck){return false;}
  virtual void set_deck_P1(const Deck &deck_P1){}
  virtual void set_deck_P2(const Deck &deck_P2){}
  virtual Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){return {-1,-1,false};}
  virtual ~Agent(){}
};