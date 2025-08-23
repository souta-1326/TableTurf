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
#include "common.hpp"
template<class stage> struct Sample{
  Board<stage> board;
  bool is_redraw_phase;
  Deck deck_P1,deck_P2;
  std::vector<float> policy_redraw;
  std::vector<std::pair<Choice<stage>,float>> policy_action;
  float value;
};

template<class stage> class Buffer{
  std::deque<Sample<stage>> buffer;
  long long max_size;
  static constexpr long long N_ROWS_OF_DATA = 11;
public:
  Buffer(long long max_size):max_size(max_size){}
  constexpr long long get_max_size(){return max_size;}
  long long get_current_size(){return buffer.size();}
  void add_sample(const Sample<stage> &sample){
    buffer.push_back(sample);
    if(get_current_size() > get_max_size()) buffer.pop_front();
  }
  //ファイルに書き出し(keep_before_data=Trueのときは既にあるデータを残す)
  void write(std::string file_name,bool keep_before_data);
};

template<class stage> void Buffer<stage>::write(std::string file_name,bool keep_before_data){
  long long next_size;
  std::string kept_raws;
  if(keep_before_data){
    //既にあるデータの個数を取得
    std::ifstream fin(file_name);
    long long before_size;fin >> before_size;
    bool is_file_empty;
    //ファイルが空の場合は、既にあるデータの個数は0とみなす
    if(fin.fail()){
      before_size = 0;
      is_file_empty = true;
    }
    else is_file_empty = false;
    long long discarded_size = std::max(0LL,before_size+get_current_size()-get_max_size());//捨てられるデータの個数
    long long kept_size = before_size-discarded_size;//保持されるデータの個数
    next_size = std::min(before_size+get_current_size(),get_max_size());//ファイルに書き込まれるデータの個数
    long long ignored_raws = (is_file_empty ? 0:1+discarded_size*N_ROWS_OF_DATA);
    long long read_raws = kept_size*N_ROWS_OF_DATA;

    //ignored_raws行読んで捨てる
    {
    std::string trash;
    for(long long i=0;i<ignored_raws;i++) std::getline(fin,trash);
    }
    //read_raws行読んで保持
    for(long long i=0;i<read_raws;i++){
      std::string current_raw;
      std::getline(fin,current_raw);
      kept_raws += current_raw+"\n";
    }
  }
  std::ofstream fout(file_name);
  if(keep_before_data){
    fout << std::fixed << std::setprecision(5);//floatの桁数を固定

    //データの個数を出力
    fout << next_size << '\n';

    //以前のファイルデータの一部を出力
    fout << kept_raws;
  }
  else{
    fout << get_current_size() << '\n';
  }
  for(const auto &[board,is_redraw_phase,deck_P1,deck_P2,policy_redraw,policy_action,value]:buffer){
    //入力
    {
    //通常マス,SPマス,壁&盤面外(5,0~4)
    //bitsetを5行羅列する
    {
    const std::bitset<stage::N_SQUARE> target[5] = 
    {board.square_P1,board.square_SP_P1,board.square_P2,board.square_SP_P2,board.wall_square};
    for(int channel_index=0;channel_index<5;channel_index++){
      for(int i=0;i<stage::h;i++){
        for(int j=0;j<stage::w;j++){
          int order = stage::place_to_order[i][j];
          if(order == -1) fout << (channel_index == 4 ? 1:0);
          else fout << target[channel_index][order];
        }
      }
      fout << '\n';
    }
    }
    //この後はchannelを1でfillするもののみなので、fillするchannel_indexを列挙する
    std::vector<int> filled_channel_indexes;
    //何ターン目か(12,5~16)
    if(!is_redraw_phase){
    assert(1 <= board.get_current_turn() && board.get_current_turn() <= Board<stage>::TURN_MAX);
    int channel_index = 5+(board.get_current_turn()-1);
    filled_channel_indexes.emplace_back(channel_index);
    }
    //SPがいくつ溜まってるか
    //P1(12,17~28)
    {
    int SP_point = board.SP_point_P1;
    SP_point = std::min(SP_point,12);//13以上の情報は要らない
    int channel_index_start = 17,channel_index_end = 17+SP_point;//半開区間
    for(int channel_index=channel_index_start;channel_index<channel_index_end;channel_index++){
      filled_channel_indexes.emplace_back(channel_index);
    }
    }
    //P2(12,29~40)
    {
    int SP_point = board.SP_point_P2;
    SP_point = std::min(SP_point,12);//13以上の情報は要らない
    int channel_index_start = 29,channel_index_end = 29+SP_point;//半開区間
    for(int channel_index=channel_index_start;channel_index<channel_index_end;channel_index++){
      filled_channel_indexes.emplace_back(channel_index);
    }
    }

    //未使用カード
    //P1(N_card,41~(40+N_card))
    {
    std::vector<int> card_id_unused;
    std::vector<int> card_id_in_hand = deck_P1.get_hand();
    std::vector<int> card_id_in_stock = deck_P1.get_stock();
    std::copy(card_id_in_hand.begin(),card_id_in_hand.end(),std::back_inserter(card_id_unused));
    std::copy(card_id_in_stock.begin(),card_id_in_stock.end(),std::back_inserter(card_id_unused));

    for(int card_id:card_id_unused){
      int channel_index = 41+(card_id-1);//card_idが1-indexedなため
      filled_channel_indexes.emplace_back(channel_index);
    }
    }
    //P2(N_card,(41+N_card)~(40+N_card*2))
    {
    std::vector<int> card_id_unused;
    std::vector<int> card_id_in_hand = deck_P2.get_hand();
    std::vector<int> card_id_in_stock = deck_P2.get_stock();
    std::copy(card_id_in_hand.begin(),card_id_in_hand.end(),std::back_inserter(card_id_unused));
    std::copy(card_id_in_stock.begin(),card_id_in_stock.end(),std::back_inserter(card_id_unused));

    for(int card_id:card_id_unused){
      int channel_index = 41+N_card+(card_id-1);//card_idが1-indexedなため
      filled_channel_indexes.emplace_back(channel_index);
    }
    }

    //filled_channel_indexesの長さを出力し、その後要素を列挙(空白区切り)
    fout << filled_channel_indexes.size() << '\n';
    for(int channel_index:filled_channel_indexes) fout << channel_index << ' ';
    fout << '\n';
    }

    //出力
    {
    //policy_action
    {
    //policy_actionの長さを出力し、その後(network_index,policy)を列挙(空白区切り)
    fout << policy_action.size() << '\n';
    for(const auto &[choice,policy]:policy_action){
      fout << choice_to_policy_action_network_index(choice) << ' ' << policy << ' ';
    }
    fout << '\n';
    }
    //policy_redraw
    {
    if(policy_redraw.size() == 0) fout << 0 << ' ' << 0 << '\n';
    else fout << policy_redraw[0] << ' ' << policy_redraw[1] << '\n';
    }
    //value
    {
    fout << value << '\n';
    }
    }
  }
}