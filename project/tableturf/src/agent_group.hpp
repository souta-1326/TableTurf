#pragma once
#include <vector>
#include <type_traits>
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"

template<class stage> class Agent_Group{
 public:
  virtual int get_group_size() const = 0;
  virtual std::vector<bool> redraws(const std::vector<Deck> &deck) = 0;
  virtual void set_deck_P1s(const std::vector<Deck> &deck_P1s) = 0;
  virtual void set_deck_P2s(const std::vector<Deck> &deck_P2s) = 0;
  virtual std::vector<Choice<stage>> get_actions(const std::vector<Board<stage>> &board_P1s,const std::vector<Board<stage>> &board_P2s,const std::vector<Deck> &decks) = 0;
};