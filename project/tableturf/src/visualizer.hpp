#pragma once
#include <Siv3D.hpp>
#include <vector>
#include "board.hpp"
template<class stage> class Visualizer{
  static constexpr s3d::Color P1_color_normal = Color{237,248,81};
  static constexpr s3d::Color P1_color_SP = Color{243,163,58};
  static constexpr s3d::Color P1_color_SP_burning = Color{255,255,89};
  static constexpr s3d::Color P2_color_normal = Color{77,91,246};
  static constexpr s3d::Color P2_color_SP = Color{117,239,252};
  static constexpr s3d::Color P2_color_SP_burning = Color{244,255,255};
  static constexpr s3d::Color wall_color = Color{216,216,216};
  static constexpr s3d::Color empty_color = Color{20,15,39};
  static constexpr int window_H = 600;
  static constexpr int window_W = 800;
  //X:150~650,Y:50~550を使用
  static constexpr int visualizer_H_min = 50;
  static constexpr int visualizer_H_max = 550;
  static constexpr int visualizer_W_min = 150;
  static constexpr int visualizer_W_max = 650;
  static constexpr int square_size = std::min(visualizer_W_max-visualizer_W_min,visualizer_H_max-visualizer_H_min)/std::max(stage::h,stage::w);
  static constexpr int space_size = 1;
  //実際に盤面が映し出される部分の上と左の座標
  static constexpr int visualizer_H_top = (visualizer_H_min+visualizer_H_max-square_size*stage::h)/2;
  static constexpr int visualizer_W_left = (visualizer_W_min+visualizer_W_max-square_size*stage::w)/2;
  static std::vector<std::vector<Rect>> squares;
  //映し出す対象の盤面
  static Board<stage> *board_ptr;
  static void squares_setting();
  //ビジュアライズのうち、盤面を映す関数
  static void show(const Board<stage> &board);
  //ビジュアライズのうち、手動で盤面のターンを進めるための関数
  static void put_both_cards_on_visualizer();
  //P1,P2の指し手をビジュアライザに送り込む
  static void set_P1_hand(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack);
  static void set_P1_hand(const int card_id,const int status_id,const bool is_SP_attack);
  static void set_P2_hand(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack);
  static void set_P2_hand(const int card_id,const int status_id,const bool is_SP_attack);
  //ビジュアライザに入力するための変数
  static int P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W;
  static bool is_P1_pass,is_P1_SP_attack;
  static int P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W;
  static bool is_P2_pass,is_P2_SP_attack;
public:
  //画面に盤面をビジュアライズする
  static void visualize();
  static void set_board(Board<stage> &board);
};
template<class stage> Board<stage> *Visualizer<stage>::board_ptr = nullptr;
template<class stage> int Visualizer<stage>::P1_card_id = 0;
template<class stage> int Visualizer<stage>::P1_card_direction = 0;
template<class stage> int Visualizer<stage>::P1_card_pos_H = -1;
template<class stage> int Visualizer<stage>::P1_card_pos_W = -1;
template<class stage> bool Visualizer<stage>::is_P1_pass = false;
template<class stage> bool Visualizer<stage>::is_P1_SP_attack = false;
template<class stage> int Visualizer<stage>::P2_card_id = 0;
template<class stage> int Visualizer<stage>::P2_card_direction = 0;
template<class stage> int Visualizer<stage>::P2_card_pos_H = -1;
template<class stage> int Visualizer<stage>::P2_card_pos_W = -1;
template<class stage> bool Visualizer<stage>::is_P2_pass = false;
template<class stage> bool Visualizer<stage>::is_P2_SP_attack = false;
template<class stage> std::vector<std::vector<Rect>> Visualizer<stage>::squares(stage::h,std::vector<Rect>(stage::w));
template<class stage> void Visualizer<stage>::squares_setting(){
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      squares[i][j] = Rect{visualizer_W_left+j*square_size,visualizer_H_top+i*square_size,square_size-space_size,square_size-space_size};
    }
  }
}
template<class stage> void Visualizer<stage>::set_board(Board<stage> &board){
  if(Visualizer<stage>::board_ptr == nullptr){
    squares_setting();
  }
  Visualizer<stage>::board_ptr = &board;
}
template<class stage> void Visualizer<stage>::show(const Board<stage> &board){
  static Font font1(square_size-space_size);
  //盤面をwindowに映し出す
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      //マスがあったら正方形を描く
      //横がX座標なので、jが先
      if(stage::exists_square[i][j]){
        //現在のマスが何番目化
        int now_order = stage::place_to_order[i][j];
        s3d::Color now_color;
        if(board.P1_square_SP[now_order]) now_color = P1_color_SP;
        else if(board.P2_square_SP[now_order]) now_color = P2_color_SP;
        else if(board.wall_square[now_order]) now_color = wall_color;
        else if(board.P1_square[now_order]) now_color = P1_color_normal;
        else if(board.P2_square[now_order]) now_color = P2_color_normal;
        else now_color = empty_color;
        squares[i][j].draw(now_color);
        if(board.P1_square_SP_burning[now_order]){
          font1(U"S").drawAt(visualizer_W_left+j*square_size+square_size/2,visualizer_H_top+i*square_size+square_size/2,Palette::Black);
        }
        else if(board.P2_square_SP_burning[now_order]){
          font1(U"S").drawAt(visualizer_W_left+j*square_size+square_size/2,visualizer_H_top+i*square_size+square_size/2,Palette::Black);
        }
      }
    }
  }
  //ターン数、それぞれのマス数とSPポイントを表示
  static Font font2(30);
  font2(U"P1\n□:{}\nSP:{}"_fmt(board.P1_square.count(),board.P1_SP_point)).draw(20,20);
  font2(U"P2\n□:{}\nSP:{}"_fmt(board.P2_square.count(),board.P2_SP_point)).draw(655,20);
  font2(U"Turn:{}/{}"_fmt(board.current_turn,Board<stage>::max_turn)).draw(20,550);
}
template<class stage> void Visualizer<stage>::put_both_cards_on_visualizer(){
  //まだboard_ptrが設定されていないなら何もしない
  if(board_ptr == nullptr) return;
  Board<stage> &board = *board_ptr;
  //Passボタンの描画
  static Font font(30);
  const String pass_text = U"Pass";
  constexpr Vec2 P1_pass_button_pos{20,150},P2_pass_button_pos{655,150};
  static RectF P1_pass_button = font(pass_text).region(P1_pass_button_pos);
  static RectF P2_pass_button = font(pass_text).region(P2_pass_button_pos);
  P1_pass_button.draw(is_P1_pass ? Palette::Yellow:Palette::Gray);
  P2_pass_button.draw(is_P2_pass ? Palette::Yellow:Palette::Gray);
  font(pass_text).draw(P1_pass_button_pos,Palette::Black);
  font(pass_text).draw(P2_pass_button_pos,Palette::Black);
  if(P1_pass_button.leftClicked()) is_P1_pass ^= 1;
  if(P2_pass_button.leftClicked()) is_P2_pass ^= 1;
  //Specialボタンの描画
  const String special_text = U"Special";
  constexpr Vec2 P1_special_button_pos{20,200},P2_special_button_pos{655,200};
  static RectF P1_special_button = font(special_text).region(P1_special_button_pos);
  static RectF P2_special_button = font(special_text).region(P2_special_button_pos);
  P1_special_button.draw(is_P1_SP_attack ? Palette::Yellow:Palette::Gray);
  P2_special_button.draw(is_P2_SP_attack ? Palette::Yellow:Palette::Gray);
  font(special_text).draw(P1_special_button_pos,Palette::Black);
  font(special_text).draw(P2_special_button_pos,Palette::Black);
  if(P1_special_button.leftClicked()) is_P1_SP_attack ^= 1;
  if(P2_special_button.leftClicked()) is_P2_SP_attack ^= 1;
  //IDボタンの描画
  const String ID_text = U"ID:";
  constexpr Vec2 P1_ID_button_pos{20,250},P2_ID_button_pos{655,250};
  static RectF P1_ID_button = font(ID_text).region(P1_ID_button_pos);
  static RectF P2_ID_button = font(ID_text).region(P2_ID_button_pos);
  P1_ID_button.draw();
  P2_ID_button.draw();
  font(ID_text).draw(P1_ID_button_pos,Palette::Black);
  font(ID_text).draw(P2_ID_button_pos,Palette::Black);
  font(P1_card_id).draw(60,250);
  font(P2_card_id).draw(695,250);
  //Directionボタンの描画
  const String direction_text = U"Dir:";
  constexpr Vec2 P1_direction_button_pos{20,300},P2_direction_button_pos{655,300};
  static RectF P1_direction_button = font(direction_text).region(P1_direction_button_pos);
  static RectF P2_direction_button = font(direction_text).region(P2_direction_button_pos);
  P1_direction_button.draw();
  P2_direction_button.draw();
  font(direction_text).draw(P1_direction_button_pos,Palette::Black);
  font(direction_text).draw(P2_direction_button_pos,Palette::Black);
  font(P1_card_direction).draw(80,300);
  font(P2_card_direction).draw(715,300);
  //数値入力
  static int *inputted_num = &P1_card_id;
  if(P1_ID_button.leftClicked()) inputted_num = &P1_card_id;
  if(P2_ID_button.leftClicked()) inputted_num = &P2_card_id;
  if(P1_direction_button.leftClicked()) inputted_num = &P1_card_direction;
  if(P2_direction_button.leftClicked()) inputted_num = &P2_card_direction;
  constexpr std::pair<Input,int> key_to_num[10] = {{Key0,0},{Key1,1},{Key2,2},{Key3,3},{Key4,4},{Key5,5},{Key6,6},{Key7,7},{Key8,8},{Key9,9}};
  for(auto [key,num]:key_to_num){
    if(key.down() && *inputted_num < 1000) *inputted_num = *inputted_num*10+num;
  }
  if(KeyBackspace.down()) *inputted_num /= 10;
  //Positionボタンの描画
  const String position_text = U"Pos:";
  constexpr Vec2 P1_position_button_pos{20,350},P2_position_button_pos{655,350};
  static RectF P1_position_button = font(position_text).region(P1_position_button_pos);
  static RectF P2_position_button = font(position_text).region(P2_position_button_pos);
  P1_position_button.draw();
  P2_position_button.draw();
  font(position_text).draw(P1_position_button_pos,Palette::Black);
  font(position_text).draw(P2_position_button_pos,Palette::Black);
  font(U"{} {}"_fmt(P1_card_pos_H,P1_card_pos_W)).draw(80,350);
  font(U"{} {}"_fmt(P2_card_pos_H,P2_card_pos_W)).draw(715,350);
  //Position入力
  static int *inputted_H = &P1_card_pos_H,*inputted_W = &P1_card_pos_W;
  if(P1_position_button.leftPressed()){
    inputted_H = &P1_card_pos_H;inputted_W = &P1_card_pos_W;
  }
  if(P2_position_button.leftPressed()){
    inputted_H = &P2_card_pos_H;inputted_W = &P2_card_pos_W;
  }
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      if(squares[i][j].leftClicked()){
        *inputted_H = i;*inputted_W = j;
      }
    }
  }
  //OKボタンの描画
  const String OK_text = U"OK";
  constexpr Vec2 OK_button_pos{68,500};
  static RectF OK_button = font(OK_text).region(OK_button_pos);
  OK_button.draw(Palette::Yellow);
  font(OK_text).draw(OK_button_pos,Palette::Black);
  //それぞれの手が入力されているか(合法かどうかは考えない)
  bool is_P1_choice_filled = (P1_card_id != 0 && P1_card_direction != -1 && ((P1_card_pos_H != -1 && P1_card_pos_W != -1) || is_P1_pass == 1));
  bool is_P2_choice_filled = (P2_card_id != 0 && P2_card_direction != -1 && ((P2_card_pos_H != -1 && P2_card_pos_W != -1) || is_P2_pass == 1));
  //入力された手が合法か
  bool is_P1_choice_valid = is_P1_choice_filled && board.is_valid_placement(true,P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack);
  bool is_P2_choice_valid = is_P2_choice_filled && board.is_valid_placement(false,P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
  //OKボタンが押されたら、合法手の場合実際にカードを置く
  if(is_P1_choice_valid && is_P2_choice_valid && OK_button.leftClicked()){
    board.put_both_cards_without_validation(P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack,
    P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
    P1_card_pos_H = P1_card_pos_W = P2_card_pos_H = P2_card_pos_W = -1;
    P1_card_id = P1_card_direction = is_P1_pass = is_P1_SP_attack = P2_card_id = P2_card_direction = is_P2_pass = is_P2_SP_attack = 0;
    show(board);
    return;
  }
  //合法でない方の手に「Invalid」を表示する
  if(is_P1_choice_filled && !is_P1_choice_valid) font(U"Invalid").draw(20,400);
  if(is_P2_choice_filled && !is_P2_choice_valid) font(U"Invalid").draw(655,400);
  //それぞれ、合法手の場合置いた時の盤面を表示する
  static Board<stage> virtual_board;
  virtual_board = board;
  if(is_P1_choice_valid && is_P2_choice_valid) virtual_board.put_both_cards_without_validation(P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack,P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
  else if(is_P1_choice_valid) virtual_board.put_P1_card_without_validation(P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack);
  else if(is_P2_choice_valid) virtual_board.put_P2_card_without_validation(P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
  show(virtual_board);
  return;
  // //もし全ての項目が埋まっている場合
  // if(is_P1_choice_filled && is_P2_choice_filled){
  //   //適切でなかったら,合法でない方に「Invalid」を表示する
  //   if(!(is_P1_choice_valid && is_P2_choice_valid)){
  //     if(!is_P1_choice_valid) font(U"Invalid").draw(20,400);
  //     if(!is_P2_choice_valid) font(U"Invalid").draw(655,400);
  //     show(board);
  //   }
  //   //OKボタンが押されたら、合法手の場合実際にカードを置く
  //   // else if(OK_button.leftClicked()){
  //   //   board.put_both_cards_without_validation(P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack,
  //   //   P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
  //   //   P1_card_pos_H = P1_card_pos_W = P2_card_pos_H = P2_card_pos_W = -1;
  //   //   P1_card_id = P1_card_direction = is_P1_pass = is_P1_SP_attack = P2_card_id = P2_card_direction = is_P2_pass = is_P2_SP_attack = 0;
  //   //   show(board);
  //   // }
  //   //OKボタンが押されてない場合、置いた場合の盤面を表示する
  //   else{
  //     static Board<stage> virtual_board;
  //     virtual_board = board;
  //     virtual_board.put_both_cards_without_validation(P1_card_id,P1_card_direction,P1_card_pos_H,P1_card_pos_W,is_P1_pass,is_P1_SP_attack,
  //     P2_card_id,P2_card_direction,P2_card_pos_H,P2_card_pos_W,is_P2_pass,is_P2_SP_attack);
  //     show(virtual_board);
  //   }
  // }
  // //すべての項目が埋まっていない場合、元の盤面を表示
  // else{
  //   show(board);
  // }
}
template<class stage> void Visualizer<stage>::set_P1_hand(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack){
  P1_card_id = card_id;
  P1_card_direction = card_direction;
  P1_card_pos_H = card_pos_H;
  P1_card_pos_W = card_pos_W;
  is_P1_pass = is_pass;
  is_P1_SP_attack = is_SP_attack;
}
template<class stage> void Visualizer<stage>::set_P1_hand(const int card_id,const int status_id,const bool is_SP_attack){
  int card_direction,card_pos_H,card_pos_W;bool is_pass;
  if(status_id == -1){
    is_pass = true;
    card_direction = card_pos_H = card_pos_W = -1;
  }
  else{
    is_pass = false;
    std::tie(card_direction,card_pos_H,card_pos_W) = stage::card_status[card_id][status_id];
  }
  set_P1_hand(card_id,card_direction,card_pos_H,card_pos_W,is_pass,is_SP_attack);
}
template<class stage> void Visualizer<stage>::set_P2_hand(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack){
  P2_card_id = card_id;
  P2_card_direction = card_direction;
  P2_card_pos_H = card_pos_H;
  P2_card_pos_W = card_pos_W;
  is_P2_pass = is_pass;
  is_P2_SP_attack = is_SP_attack;
}
template<class stage> void Visualizer<stage>::set_P2_hand(const int card_id,const int status_id,const bool is_SP_attack){
  int card_direction,card_pos_H,card_pos_W;bool is_pass;
  if(status_id == -1){
    is_pass = true;
    card_direction = card_pos_H = card_pos_W = -1;
  }
  else{
    is_pass = false;
    std::tie(card_direction,card_pos_H,card_pos_W) = stage::card_status[card_id][status_id];
  }
  set_P2_hand(card_id,card_direction,card_pos_H,card_pos_W,is_pass,is_SP_attack);
}

template<class stage> void Visualizer<stage>::visualize(){
  put_both_cards_on_visualizer();
}
