#include <Siv3D.hpp>
#include "board.hpp"
#include <algorithm>
template<class stage> void Show(Board<stage> &board){
  constexpr s3d::Color my_color_normal = Color{237,248,81};
  constexpr s3d::Color my_color_SP = Color{243,163,58};
  constexpr s3d::Color opponent_color_normal = Color{77,91,246};
  constexpr s3d::Color opponent_color_SP = Color{117,239,252};
  constexpr s3d::Color wall_color = Color{216,216,216};
  constexpr s3d::Color empty_color = Color{20,15,39};
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
        //現在のマスが何番目化
        int now_order = stage::place_to_order[i][j];
        s3d::Color now_color;
        if(board.my_square_SP[now_order]) now_color = my_color_SP;
        else if(board.opponent_square_SP[now_order]) now_color = opponent_color_SP;
        else if(board.wall_square[now_order]) now_color = wall_color;
        else if(board.my_square[now_order]) now_color = my_color_normal;
        else if(board.opponent_square[now_order]) now_color = opponent_color_normal;
        else now_color = empty_color;
        Rect{visualizer_W_left+j*square_size,visualizer_H_top+i*square_size,square_size-space_size,square_size-space_size}.draw(now_color);
      }
    }
  }
}
void Main(){
  Board<Thunder_Point> board;
  Scene::SetBackground(Color{57,48,131});
  board.put_both_card_without_validation(5,0,9,7,0,5,2,7,6,0);
  while(System::Update()){
    Show(board);
  }
}
