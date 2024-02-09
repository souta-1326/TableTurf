#pragma once
#include <Siv3D.hpp>
#include <vector>
#include "../deck.hpp"
#include "../card.hpp"
#include "../card_database.hpp"
#include "common_siv3d.hpp"

class Visualizer_Deck{
  static constexpr int rect_H_length = 200;//カードの縦横サイズ
  static constexpr int rect_W_length = 160;
  static constexpr int square_size = 19;
  static constexpr int space_size_btw_card = 5;//マスとマスの間の隙間
  static constexpr int space_size_btw_cell = 1;//マスとマスの間の隙間


  //映し出す対象のデッキ
  static Deck *deck_ptr;
  //カードを載せる長方形の設定
  static void squares_setting();
  static std::vector<Rect> squares_in_deck;
  static std::vector<std::vector<std::vector<Rect>>> squares_in_card;
  //クリックされたカードを管理
  static std::vector<short> is_clicked;
public:
  //選ばれたカード群を返す
  static std::vector<int> visualize(const Deck &deck);
  //is_clickedをreset
  static void reset_is_clicked();
};
std::vector<Rect> Visualizer_Deck::squares_in_deck(Deck::N_CARD_IN_DECK);
std::vector<std::vector<std::vector<Rect>>> Visualizer_Deck::squares_in_card
(Deck::N_CARD_IN_DECK,std::vector<std::vector<Rect>>(Card::MAX_H,std::vector<Rect>(Card::MAX_W)));
std::vector<short> Visualizer_Deck::is_clicked(Deck::N_CARD_IN_DECK);
void Visualizer_Deck::squares_setting(){
  static bool done_setting = false;
  if(done_setting) return;

  //squares_in_deck
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    int h_index = i/5,w_index = i%5;
    squares_in_deck[i] = 
    Rect{
    w_index*rect_W_length,h_index*rect_H_length,
    rect_W_length-space_size_btw_card,rect_H_length-space_size_btw_card};
  }
  //squares_in_card
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    for(int j=0;j<Card::MAX_H;j++){
      for(int k=0;k<Card::MAX_W;k++){
        int h_index = i/5,w_index = i%5;
        squares_in_card[i][j][k] = 
        Rect{
        w_index*rect_W_length+k*square_size,
        h_index*rect_H_length+j*square_size,
        square_size-space_size_btw_cell,square_size-space_size_btw_cell};
      }
    }
  }
  done_setting = true;
}

std::vector<int> Visualizer_Deck::visualize(const Deck &deck){
  squares_setting();

  //カードのクリック処理
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    if(squares_in_deck[i].leftClicked()) is_clicked[i] ^= 1;
  }

  //squares_in_deck
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    squares_in_deck[i].draw(is_clicked[i] ? card_color_light:card_color);
  }
  //squares_in_card
  std::vector<int> card_id_in_deck = deck.get_deck();
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    for(int j=0;j<Card::MAX_H;j++){
      for(int k=0;k<Card::MAX_W;k++){
        //カードのそれぞれのセルに対応する色にする
        s3d::Color now_color = empty_color;

        const Card &now_card = cards[card_id_in_deck[i]];
        //Cardは不要な部分がカットされているので、平行移動して復元する
        int slide_H = (Card::MAX_H-now_card.H)/2,slide_W = (Card::MAX_W-now_card.W)/2;
        if(now_card.R0_SP_H != -1 && now_card.R0_SP_H+slide_H == j && now_card.R0_SP_W+slide_W == k){
          now_color = color_SP_P1;
        }
        else if(j>=slide_H && k>=slide_W && now_card.R0[j-slide_H][k-slide_W]){
          now_color = color_normal_P1;
        }
        squares_in_card[i][j][k].draw(now_color);
      }
    }
  }

  //選ばれたカードのIDを返す
  std::vector<int> chosen_card;
  for(int i=0;i<Deck::N_CARD_IN_DECK;i++){
    if(is_clicked[i]) chosen_card.emplace_back(card_id_in_deck[i]);
  }
  return chosen_card;
}

void Visualizer_Deck::reset_is_clicked(){
  std::fill(is_clicked.begin(),is_clicked.end(),false);
}