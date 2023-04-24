#include <Siv3D.hpp>
#include <algorithm>
#include "board.hpp"
#include "visualizer.hpp"
void Main(){
  Board<Thunder_Point> board;
  Visualizer<Thunder_Point>::set_board(board);
  Scene::SetBackground(Color{57,48,131});
  board.put_both_cards_without_validation(196,1,11,4,0,0,158,2,4,9,0,0);
  board.put_both_cards_without_validation(1,2,9,0,0,0,148,0,8,3,0,0);
  board.put_both_cards_without_validation(14,2,11,7,0,0,55,3,9,11,0,0);
  board.put_both_cards_without_validation(10,2,12,7,0,0,135,3,7,1,0,0);
  board.put_both_cards_without_validation(148,1,13,0,0,0,145,0,10,9,0,0);
  board.put_both_cards_without_validation(102,1,12,13,0,0,146,2,7,3,0,0);
  //board.put_both_cards_without_validation(185,3,7,13,0,1,61,0,6,10,0,0);
  while(System::Update()){
    Visualizer<Thunder_Point>::visualize();
  }
}
