#pragma once
#include <Siv3D.hpp>
#include <vector>
#include "board.hpp"
template<class stage> class Visualizer{
  static constexpr s3d::Color color_normal_P1 = Color{237,248,81};
  static constexpr s3d::Color color_SP_P1 = Color{243,163,58};
  static constexpr s3d::Color color_SP_burning_P1 = Color{255,255,89};
  static constexpr s3d::Color color_normal_P2 = Color{77,91,246};
  static constexpr s3d::Color color_SP_P2 = Color{117,239,252};
  static constexpr s3d::Color color_SP_burning_P2 = Color{244,255,255};
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
  static int card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1;
  static bool is_pass_P1,is_SP_attack_P1;
  static int card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2;
  static bool is_pass_P2,is_SP_attack_P2;
public:
  //画面に盤面をビジュアライズする
  static void visualize();
  static void set_board(Board<stage> &board);
};
template<class stage> Board<stage> *Visualizer<stage>::board_ptr = nullptr;
template<class stage> int Visualizer<stage>::card_id_P1 = 0;
template<class stage> int Visualizer<stage>::card_direction_P1 = 0;
template<class stage> int Visualizer<stage>::card_pos_H_P1 = -1;
template<class stage> int Visualizer<stage>::card_pos_W_P1 = -1;
template<class stage> bool Visualizer<stage>::is_pass_P1 = false;
template<class stage> bool Visualizer<stage>::is_SP_attack_P1 = false;
template<class stage> int Visualizer<stage>::card_id_P2 = 0;
template<class stage> int Visualizer<stage>::card_direction_P2 = 0;
template<class stage> int Visualizer<stage>::card_pos_H_P2 = -1;
template<class stage> int Visualizer<stage>::card_pos_W_P2 = -1;
template<class stage> bool Visualizer<stage>::is_pass_P2 = false;
template<class stage> bool Visualizer<stage>::is_SP_attack_P2 = false;
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
        if(board.square_SP_P1[now_order]) now_color = color_SP_P1;
        else if(board.square_SP_P2[now_order]) now_color = color_SP_P2;
        else if(board.wall_square[now_order]) now_color = wall_color;
        else if(board.square_P1[now_order]) now_color = color_normal_P1;
        else if(board.square_P2[now_order]) now_color = color_normal_P2;
        else now_color = empty_color;
        squares[i][j].draw(now_color);
        if(board.square_SP_burning_P1[now_order]){
          font1(U"S").drawAt(visualizer_W_left+j*square_size+square_size/2,visualizer_H_top+i*square_size+square_size/2,Palette::Black);
        }
        else if(board.square_SP_burning_P2[now_order]){
          font1(U"S").drawAt(visualizer_W_left+j*square_size+square_size/2,visualizer_H_top+i*square_size+square_size/2,Palette::Black);
        }
      }
    }
  }
  //ターン数、それぞれのマス数とSPポイントを表示
  static Font font2(30);
  font2(U"P1\n□:{}\nSP:{}"_fmt(board.square_P1.count(),board.SP_point_P1)).draw(20,20);
  font2(U"P2\n□:{}\nSP:{}"_fmt(board.square_P2.count(),board.SP_point_P2)).draw(655,20);
  font2(U"Turn:{}/{}"_fmt(board.current_turn,Board<stage>::TURN_MAX)).draw(20,550);
}
template<class stage> void Visualizer<stage>::put_both_cards_on_visualizer(){
  //まだboard_ptrが設定されていないなら何もしない
  if(board_ptr == nullptr) return;
  Board<stage> &board = *board_ptr;
  //Passボタンの描画
  static Font font(30);
  const String pass_text = U"Pass";
  constexpr Vec2 pass_button_pos_P1{20,150},pass_button_pos_P2{655,150};
  static RectF pass_button_P1 = font(pass_text).region(pass_button_pos_P1);
  static RectF pass_button_P2 = font(pass_text).region(pass_button_pos_P2);
  pass_button_P1.draw(is_pass_P1 ? Palette::Yellow:Palette::Gray);
  pass_button_P2.draw(is_pass_P2 ? Palette::Yellow:Palette::Gray);
  font(pass_text).draw(pass_button_pos_P1,Palette::Black);
  font(pass_text).draw(pass_button_pos_P2,Palette::Black);
  if(pass_button_P1.leftClicked()) is_pass_P1 ^= 1;
  if(pass_button_P2.leftClicked()) is_pass_P2 ^= 1;
  //Specialボタンの描画
  const String special_text = U"Special";
  constexpr Vec2 special_button_pos_P1{20,200},special_button_pos_P2{655,200};
  static RectF special_button_P1 = font(special_text).region(special_button_pos_P1);
  static RectF special_button_P2 = font(special_text).region(special_button_pos_P2);
  special_button_P1.draw(is_SP_attack_P1 ? Palette::Yellow:Palette::Gray);
  special_button_P2.draw(is_SP_attack_P2 ? Palette::Yellow:Palette::Gray);
  font(special_text).draw(special_button_pos_P1,Palette::Black);
  font(special_text).draw(special_button_pos_P2,Palette::Black);
  if(special_button_P1.leftClicked()) is_SP_attack_P1 ^= 1;
  if(special_button_P2.leftClicked()) is_SP_attack_P2 ^= 1;
  //IDボタンの描画
  const String ID_text = U"ID:";
  constexpr Vec2 ID_button_pos_P1{20,250},ID_button_pos_P2{655,250};
  static RectF ID_button_P1 = font(ID_text).region(ID_button_pos_P1);
  static RectF ID_button_P2 = font(ID_text).region(ID_button_pos_P2);
  ID_button_P1.draw();
  ID_button_P2.draw();
  font(ID_text).draw(ID_button_pos_P1,Palette::Black);
  font(ID_text).draw(ID_button_pos_P2,Palette::Black);
  font(card_id_P1).draw(60,250);
  font(card_id_P2).draw(695,250);
  //Directionボタンの描画
  const String direction_text = U"Dir:";
  constexpr Vec2 direction_button_pos_P1{20,300},direction_button_pos_P2{655,300};
  static RectF direction_button_P1 = font(direction_text).region(direction_button_pos_P1);
  static RectF direction_button_P2 = font(direction_text).region(direction_button_pos_P2);
  direction_button_P1.draw();
  direction_button_P2.draw();
  font(direction_text).draw(direction_button_pos_P1,Palette::Black);
  font(direction_text).draw(direction_button_pos_P2,Palette::Black);
  font(card_direction_P1).draw(80,300);
  font(card_direction_P2).draw(715,300);
  //数値入力
  static int *inputted_num = &card_id_P1;
  if(ID_button_P1.leftClicked()) inputted_num = &card_id_P1;
  if(ID_button_P2.leftClicked()) inputted_num = &card_id_P2;
  if(direction_button_P1.leftClicked()) inputted_num = &card_direction_P1;
  if(direction_button_P2.leftClicked()) inputted_num = &card_direction_P2;
  constexpr std::pair<Input,int> key_to_num[10] = {{Key0,0},{Key1,1},{Key2,2},{Key3,3},{Key4,4},{Key5,5},{Key6,6},{Key7,7},{Key8,8},{Key9,9}};
  for(auto [key,num]:key_to_num){
    if(key.down() && *inputted_num < 1000) *inputted_num = *inputted_num*10+num;
  }
  if(KeyBackspace.down()) *inputted_num /= 10;
  //Positionボタンの描画
  const String position_text = U"Pos:";
  constexpr Vec2 position_button_pos_P1{20,350},position_button_pos_P2{655,350};
  static RectF position_button_P1 = font(position_text).region(position_button_pos_P1);
  static RectF position_button_P2 = font(position_text).region(position_button_pos_P2);
  position_button_P1.draw();
  position_button_P2.draw();
  font(position_text).draw(position_button_pos_P1,Palette::Black);
  font(position_text).draw(position_button_pos_P2,Palette::Black);
  font(U"{} {}"_fmt(card_pos_H_P1,card_pos_W_P1)).draw(80,350);
  font(U"{} {}"_fmt(card_pos_H_P2,card_pos_W_P2)).draw(715,350);
  //Position入力
  static int *inputted_H = &card_pos_H_P1,*inputted_W = &card_pos_W_P1;
  if(position_button_P1.leftPressed()){
    inputted_H = &card_pos_H_P1;inputted_W = &card_pos_W_P1;
  }
  if(position_button_P2.leftPressed()){
    inputted_H = &card_pos_H_P2;inputted_W = &card_pos_W_P2;
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
  bool is_choice_filled_P1 = (card_id_P1 != 0 && card_direction_P1 != -1 && ((card_pos_H_P1 != -1 && card_pos_W_P1 != -1) || is_pass_P1 == 1));
  bool is_choice_filled_P2 = (card_id_P2 != 0 && card_direction_P2 != -1 && ((card_pos_H_P2 != -1 && card_pos_W_P2 != -1) || is_pass_P2 == 1));
  //入力された手が合法か
  bool is_choice_valid_P1 = is_choice_filled_P1 && board.is_valid_placement(true,card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1);
  bool is_choice_valid_P2 = is_choice_filled_P2 && board.is_valid_placement(false,card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
  //OKボタンが押されたら、合法手の場合実際にカードを置く
  if(is_choice_valid_P1 && is_choice_valid_P2 && OK_button.leftClicked()){
    board.put_both_cards_without_validation(card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1,
    card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
    card_pos_H_P1 = card_pos_W_P1 = card_pos_H_P2 = card_pos_W_P2 = -1;
    card_id_P1 = card_direction_P1 = is_pass_P1 = is_SP_attack_P1 = card_id_P2 = card_direction_P2 = is_pass_P2 = is_SP_attack_P2 = 0;
    show(board);
    return;
  }
  //合法でない方の手に「Invalid」を表示する
  if(is_choice_filled_P1 && !is_choice_valid_P1) font(U"Invalid").draw(20,400);
  if(is_choice_filled_P2 && !is_choice_valid_P2) font(U"Invalid").draw(655,400);
  //それぞれ、合法手の場合置いた時の盤面を表示する
  static Board<stage> virtual_board;
  virtual_board = board;
  if(is_choice_valid_P1 && is_choice_valid_P2) virtual_board.put_both_cards_without_validation(card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1,card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
  else if(is_choice_valid_P1) virtual_board.put_P1_card_without_validation(card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1);
  else if(is_choice_valid_P2) virtual_board.put_P2_card_without_validation(card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
  show(virtual_board);
  return;
  // //もし全ての項目が埋まっている場合
  // if(is_choice_filled_P1 && is_choice_filled_P2){
  //   //適切でなかったら,合法でない方に「Invalid」を表示する
  //   if(!(is_choice_valid_P1 && is_choice_valid_P2)){
  //     if(!is_choice_valid_P1) font(U"Invalid").draw(20,400);
  //     if(!is_choice_valid_P2) font(U"Invalid").draw(655,400);
  //     show(board);
  //   }
  //   //OKボタンが押されたら、合法手の場合実際にカードを置く
  //   // else if(OK_button.leftClicked()){
  //   //   board.put_both_cards_without_validation(card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1,
  //   //   card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
  //   //   card_pos_H_P1 = card_pos_W_P1 = card_pos_H_P2 = card_pos_W_P2 = -1;
  //   //   card_id_P1 = card_direction_P1 = is_pass_P1 = is_SP_attack_P1 = card_id_P2 = card_direction_P2 = is_pass_P2 = is_SP_attack_P2 = 0;
  //   //   show(board);
  //   // }
  //   //OKボタンが押されてない場合、置いた場合の盤面を表示する
  //   else{
  //     static Board<stage> virtual_board;
  //     virtual_board = board;
  //     virtual_board.put_both_cards_without_validation(card_id_P1,card_direction_P1,card_pos_H_P1,card_pos_W_P1,is_pass_P1,is_SP_attack_P1,
  //     card_id_P2,card_direction_P2,card_pos_H_P2,card_pos_W_P2,is_pass_P2,is_SP_attack_P2);
  //     show(virtual_board);
  //   }
  // }
  // //すべての項目が埋まっていない場合、元の盤面を表示
  // else{
  //   show(board);
  // }
}
template<class stage> void Visualizer<stage>::set_P1_hand(const int card_id,const int card_direction,const int card_pos_H,const int card_pos_W,const bool is_pass,const bool is_SP_attack){
  card_id_P1 = card_id;
  card_direction_P1 = card_direction;
  card_pos_H_P1 = card_pos_H;
  card_pos_W_P1 = card_pos_W;
  is_pass_P1 = is_pass;
  is_SP_attack_P1 = is_SP_attack;
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
  card_id_P2 = card_id;
  card_direction_P2 = card_direction;
  card_pos_H_P2 = card_pos_H;
  card_pos_W_P2 = card_pos_W;
  is_pass_P2 = is_pass;
  is_SP_attack_P2 = is_SP_attack;
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
