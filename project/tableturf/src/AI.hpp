#pragma once
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
template<class stage> using AI = Choice(*)(const Board<stage>& board,const bool is_P1,const Deck& deck);