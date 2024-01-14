#pragma once
#include <vector>
#include <type_traits>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"

template<class stage> class Agent{
 public:
  virtual bool redraw(const Deck &deck) = 0;
  virtual Choice<stage> get_action(const Board<stage> &board,const Deck &deck) = 0;
};