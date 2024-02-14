#pragma once
#include <string>
#include <vector>
#include <deque>
#include <utility>
#include <fstream>
#include <torch/script.h>
#include "board.hpp"
#include "deck.hpp"
#include "choice.hpp"
template<class stage> struct Sample{
  Board<stage> board;
  Deck deck_P1,deck_P2;
  std::vector<float> policy_redraw;
  std::vector<std::pair<Choice<stage>,float>> policy_action;
  float value;
};

template<class stage> class Buffer{
  std::deque<Sample<stage>> buffer;
  int max_size;
public:
  Buffer(int max_size):max_size(max_size){}
  constexpr int get_max_size(){return max_size;}
  int get_current_size(){return buffer.size();}
  void add_sample(const Sample<stage> &sample){
    buffer.push_back(sample);
    if(get_current_size() > get_max_size()) buffer.pop_front();
  }
  void write(std::string file_name);
  void read(std::string file_name);
};

