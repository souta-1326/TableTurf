#pragma once
#include <vector>
#include <type_traits>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"

template<class stage> class Agent{
 public:
  virtual bool redraw(const Deck &deck) = 0;
  virtual void get_deck_P1(const Deck &deck_P1) = 0;
  virtual void get_deck_P2(const Deck &deck_P2) = 0;
  virtual Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) = 0;
};