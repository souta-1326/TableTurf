#include <Siv3D.hpp>
#include "board.hpp"
#include <algorithm>
template<class stage> void Show(Board<stage> &board){
  constexpr int window_H = 600;
  constexpr int window_W = 800;
  //X:125~675,Y:25~575を使用
  constexpr int visualizer_H_min = 25;
  constexpr int visualizer_H_max = 575;
  constexpr int visualizer_W_min = 125;
  constexpr int visualizer_W_max = 675;
  int square_size = std::min(visualizer_W_max-visualizer_W_min,visualizer_H_max-visualizer_H_min)/std::max(stage::h,stage::w);
  constexpr int space_size = 1;
  //実際に盤面が映し出される部分の上と左の座標
  int visualizer_H_top = (visualizer_H_min+visualizer_H_max-square_size*stage::h)/2;
  int visualizer_W_left = (visualizer_W_min+visualizer_W_max-square_size*stage::w)/2;
  //盤面をwindowに映し出す
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      //マスがあったら正方形を描く
      //横がX座標なので、jが先
      if(stage::exists_square[i][j]){
        Rect{visualizer_W_left+j*square_size,visualizer_H_top+i*square_size,square_size-space_size,square_size-space_size}.draw();
      }
    }
  }
}
void Main(){
  Board<Thunder_Point> board;
  while(System::Update()){
    Show(board);
  }
}
