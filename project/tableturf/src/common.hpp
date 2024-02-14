#pragma once
#include "choice.hpp"
#include "card.hpp"

constexpr int INPUT_C = 2+2+1+12+12*2+N_card*2;
template<class stage> constexpr int ACTION_SPACE_OF_EACH_CARD = stage::h*stage::w*8+1;


template<class stage> int choice_to_policy_action_index(const Choice<stage> choice){
  if(choice.status_id == -1) return (choice.card_id-1)*ACTION_SPACE_OF_EACH_CARD<stage>;
  else{
    auto [direction,h_index,w_index] = stage::card_status[choice.card_id][choice.status_id];
    return
    (choice.card_id-1)*ACTION_SPACE_OF_EACH_CARD<stage>+
    (choice.is_SP_attack ? 4*(stage::h*stage::w):0)+
    direction*(stage::h*stage::w)+
    h_index*stage::w+
    w_index
    +1;
  }
}

//入力を作成
template<class stage> std::vector<float> construct_image_vector(const Board<stage> &board,const Deck &deck_P1,const Deck &deck_P2){
  std::vector<float> state_array(INPUT_C*stage::h*stage::w);//1次元化
  //通常マス,SPマス,壁&盤面外(5,0~4)
  {
  for(int i=0;i<stage::h;i++){
    for(int j=0;j<stage::w;j++){
      int channel_index;
      //盤面外
      if(stage::place_to_order[i][j] == -1) channel_index = 4;
      else{
        //stageの(i,j)がbitsetのorder番目に対応
        int order = stage::place_to_order[i][j];
        //壁
        if(board.wall_square[order]) channel_index = 4;
        //SPマス
        else if(board.square_SP_P1[order]) channel_index = 1;
        else if(board.square_SP_P2[order]) channel_index = 3;
        //通常マス
        else if(board.square_P1[order]) channel_index = 0;
        else if(board.square_P2[order]) channel_index = 2;
        else channel_index = -1;//空きマスなので何もしない
      }
      if(channel_index != -1){
        //何かのミスで被っていたらバグ
        assert(state_array[channel_index*(stage::h*stage::w)+i*stage::w+j] == 0);

        state_array[channel_index*(stage::h*stage::w)+i*stage::w+j] = 1;
      }
    }
  }
  }
  //何ターン目か(12,5~16)
  {
  assert(1 <= board.get_current_turn() && board.get_current_turn() <= Board<stage>::TURN_MAX);
  int channel_index = 5+(board.get_current_turn()-1);
  std::fill(state_array.begin()+channel_index*(stage::h*stage::w),state_array.begin()+(channel_index+1)*(stage::h*stage::w),1);
  }
  //SPがいくつ溜まってるか
  //1P(12,17~28)
  {
  int SP_point = board.SP_point_P1;
  SP_point = std::min(SP_point,12);//13以上の情報は要らない
  int channel_index_start = 17,channel_index_end = 17+SP_point;//半開区間
  std::fill(state_array.begin()+channel_index_start*(stage::h*stage::w),state_array.begin()+channel_index_end*(stage::h*stage::w),1);
  }
  //2P(12,29~40)
  {
  int SP_point = board.SP_point_P2;
  SP_point = std::min(SP_point,12);//13以上の情報は要らない
  int channel_index_start = 29,channel_index_end = 29+SP_point;//半開区間
  std::fill(state_array.begin()+channel_index_start*(stage::h*stage::w),state_array.begin()+channel_index_end*(stage::h*stage::w),1);
  }
  
  //未使用カード
  //1P(N_card,41~(40+N_card))
  {
  std::vector<int> card_id_unused;
  std::vector<int> card_id_in_hand = deck_P1.get_hand();
  std::vector<int> card_id_in_stock = deck_P1.get_stock();
  std::copy(card_id_in_hand.begin(),card_id_in_hand.end(),std::back_inserter(card_id_unused));
  std::copy(card_id_in_stock.begin(),card_id_in_stock.end(),std::back_inserter(card_id_unused));

  for(int card_id:card_id_unused){
    int channel_index = 41+(card_id-1);//card_idが1-indexedなため
    std::fill(state_array.begin()+channel_index*(stage::h*stage::w),state_array.begin()+(channel_index+1)*(stage::h*stage::w),1);
  }
  }
  //2P(N_card,(41+N_card)~(40+N_card*2))
  {
  std::vector<int> card_id_unused;
  std::vector<int> card_id_in_hand = deck_P2.get_hand();
  std::vector<int> card_id_in_stock = deck_P2.get_stock();
  std::copy(card_id_in_hand.begin(),card_id_in_hand.end(),std::back_inserter(card_id_unused));
  std::copy(card_id_in_stock.begin(),card_id_in_stock.end(),std::back_inserter(card_id_unused));

  for(int card_id:card_id_unused){
    int channel_index = 41+N_card+(card_id-1);//card_idが1-indexedなため
    std::fill(state_array.begin()+channel_index*(stage::h*stage::w),state_array.begin()+(channel_index+1)*(stage::h*stage::w),1);
  }
  }

  return state_array;
}