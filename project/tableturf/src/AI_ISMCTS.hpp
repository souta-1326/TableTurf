#pragma once
#include <utility>
#include <unordered_map>
#include <numeric>
#include "agent.hpp"
#include "AI_random.hpp"
#include "AI_greedy.hpp"
#include "board.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "hash.hpp"
#include "xorshift64.hpp"
//常に手札からランダムに選んでパスするAI
template<class stage> class AI_ISMCTS : public Agent<stage>{
 private:
  //MCTSの設定
  int num_simulations;
  float diff_bonus;//相手とのマス差にこれをかけた値が評価値に加算される
  bool use_evaluation_greedy;

  //search_actionの入力の盤面
  Board<stage> root_board_P1,root_board_P2;
  int root_pos = 0;
  //前回get_action()が呼び出された時のcurrent_turn
  //get_action()が呼び出された時のcurrent_turnが、これに1足した値に等しくないとエラー
  //redrawの選択はターン0とする
  int before_root_current_turn = -1;
  int root_current_turn;
  bool did_redraw_deck = false;
  //自分のデッキ 手札が正しいことが保証されている
  Deck deck_P1;
  //相手のデッキ 手札はわからない
  Deck deck_P2;

  //ログ出力
  bool logging;

  // valid actions
  // simulationごとに変わる手札によって、実際に合法な手は変わる
  // redraw用ノードを除いて全ノード共通なので、1次元vector
  std::vector<Choice<stage>> valid_actions_P1,valid_actions_P2;

  //{card_id,status_id,is_SP_attack}に対応するW,Nを探す
  //W[valid_actions_start_index[card_id][is_SP_attack]+(is_SP_attack ? 0:1)+status_id]を参照する
  //デッキに含まれていないカードには-1を代入する
  std::vector<std::vector<int>> valid_actions_start_index_P1,valid_actions_start_index_P2;

  // W is total action-value and Q=W/N is mean action-value
  // (自作のオセロAIとは異なり、子ではなく親が値を持つ)
  std::vector<std::vector<float>> W_P1,W_P2;
  // visit count
  std::vector<std::vector<int>> N_P1,N_P2;

  //parent_pos[child_pos] = {parentのpos,1Pのindex,2Pのindex}
  std::vector<std::tuple<int,int,int>> parent_pos;

  //pos_map[{parentのpos,1Pのindex,2Pのindex}] = child_pos
  std::unordered_map<std::tuple<int,int,int>,int,hash_tuple> pos_map;

  //一度でもselectionで探索されたノードの数
  int used_node_count = 0;

  float UCB_score(float w,int n,int n_parent) const;

  //ChoiceからW,Nで参照するindexに変換
  constexpr int choice_to_valid_actions_index(const bool is_placement_P1,Choice<stage> choice) const;

  //{探索した葉のpos,葉で選んだindex_P1,葉で選んだindex_P2,葉での盤面P1,葉での盤面P2,葉でのdeck_P1,葉でのdeck_P2}を返す
  std::tuple<int,int,int,Board<stage>,Board<stage>,Deck,Deck> selection();
  
  //P1視点の評価値を返す
  float evaluation_random(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const;
  float evaluation_greedy(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const;
  float evaluation(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const;

  void expansion(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2);
  void backup(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const float value_P1);
  
  //simulate,evaluation,expansion,backupをする
  void simulate();
 public:
  AI_ISMCTS(int num_simulations,float diff_bonus=0,bool use_evaluation_greedy=true,bool logging=false);
  bool redraw(const Deck &deck) override;
  void set_deck_P1(const Deck &deck_P1) override;
  void set_deck_P2(const Deck &deck_P2) override;
  Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) override;
  
  void set_root(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck);
};
template<class stage> AI_ISMCTS<stage>::AI_ISMCTS(int num_simulations,float diff_bonus,bool use_evaluation_greedy,bool logging):
num_simulations(num_simulations),diff_bonus(diff_bonus),use_evaluation_greedy(use_evaluation_greedy),logging(logging),
valid_actions_start_index_P1(N_card+1,std::vector<int>(2,-1)),valid_actions_start_index_P2(N_card+1,std::vector<int>(2,-1)){
  //rootとexpansionによって最終的にnum_simulations+1の長さになるので、その分メモリを確保する
  W_P1.reserve(num_simulations+1);W_P2.reserve(num_simulations+1);
  N_P1.reserve(num_simulations+1);N_P2.reserve(num_simulations+1);
  parent_pos.reserve(num_simulations+1);
  pos_map.reserve(num_simulations+2);//unordered_mapは処理系によっては1要素多く確保する必要があるらしいので、念の為
}

template<class stage> float AI_ISMCTS<stage>::UCB_score(float w,int n,int n_parent) const {
  //まだ未探索の子に関しては、ランダム性を持たせる
  return n==0 ? 1e9+xorshift64()%(1<<16) : w/n+std::sqrt(2*std::log(n_parent)/n);
}

template<class stage> constexpr int AI_ISMCTS<stage>::choice_to_valid_actions_index(const bool is_placement_P1,Choice<stage> choice) const{
  return (is_placement_P1 ? valid_actions_start_index_P1:valid_actions_start_index_P2)[choice.card_id][choice.is_SP_attack]+(choice.is_SP_attack ? 0:1)+choice.status_id;
}

template<class stage> std::tuple<int,int,int,Board<stage>,Board<stage>,Deck,Deck> AI_ISMCTS<stage>::selection(){
  Board<stage> simulated_board_P1 = root_board_P1,simulated_board_P2 = root_board_P2;
  Deck simulated_deck_P1 = deck_P1,simulated_deck_P2 = Deck(deck_P2,root_board_P1.used_cards_P2);
  assert(simulated_deck_P1.get_current_turn() == std::max(1,root_current_turn));
  assert(simulated_deck_P2.get_current_turn() == std::max(1,root_current_turn));

  int now_pos = root_pos;
  while(true){
    //ノードがはじめて訪問されたときに初期化
    if(W_P1[now_pos].empty()){
      W_P1[now_pos].resize(valid_actions_P1.size());
      N_P1[now_pos].resize(valid_actions_P1.size());
      W_P2[now_pos].resize(valid_actions_P2.size());
      N_P2[now_pos].resize(valid_actions_P2.size());
      used_node_count++;
    }
    //親の探索回数
    int n_parent = std::accumulate(N_P1[now_pos].begin(),N_P1[now_pos].end(),0);

    int chosen_index_P1 = -1,chosen_index_P2 = -1;

    //P1とP2の手札
    std::vector<int> hand_P1 = simulated_deck_P1.get_hand();
    std::vector<int> hand_P2 = simulated_deck_P2.get_hand();
    //現在のノードがredrawフェイズ(ターン0)か
    bool is_redraw_phase = (root_current_turn == 0 && now_pos == 0);
    if(is_redraw_phase) assert(W_P1[now_pos].size() == 2 && W_P2[now_pos].size() == 1);

    std::tuple<bool,int&,std::vector<int>&,std::vector<float>&,std::vector<int>&,std::vector<Choice<stage>>&> looper[2] =
    {{true,chosen_index_P1,hand_P1,W_P1[now_pos],N_P1[now_pos],valid_actions_P1},
    {false,chosen_index_P2,hand_P2,W_P2[now_pos],N_P2[now_pos],valid_actions_P2}};
    for(auto [is_placement_P1,chosen_index,hand,W,N,valid_actions]:looper){
      //UCTスコアが最大となる子を求める
      float max_UCB_score = -1e9;
      if(is_redraw_phase){
        for(int i=0;i<W.size();i++){
          float now_UCB_score = UCB_score(W[i],N[i],n_parent);
          if(max_UCB_score < now_UCB_score){
            max_UCB_score = now_UCB_score;
            chosen_index = i;
          }
        }
      }
      else{
        for(int card_id:hand)
        for(bool is_SP_attack:{false,true}){
          //SPポイントが不足している場合は除外
          if(is_SP_attack && !simulated_board_P1.is_enough_SP_point(is_placement_P1,card_id)) continue;
          
          for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
            int now_index = choice_to_valid_actions_index(is_placement_P1,{card_id,status_id,is_SP_attack});
            assert(valid_actions[now_index].status_id == status_id);
            //既に手札にあるカードになっている
            //設置不可能なChoiceは除外
            if(!simulated_board_P1.is_valid_placement_without_SP_point_validation(is_placement_P1,valid_actions[now_index])) continue;
            
            float now_UCB_score = UCB_score(W[now_index],N[now_index],n_parent);
            if(max_UCB_score < now_UCB_score){
              max_UCB_score = now_UCB_score;
              chosen_index = now_index;
            }
          }
        }
      }
    }
    assert(chosen_index_P1 != -1);
    assert(chosen_index_P2 != -1);

    //盤面、デッキの更新
    if(is_redraw_phase){
      if(chosen_index_P1 == 1) simulated_deck_P1.reset();
    }
    else{
      simulated_board_P1.put_both_cards_without_validation(valid_actions_P1[chosen_index_P1],valid_actions_P2[chosen_index_P2]);
      simulated_board_P2.put_both_cards_without_validation(valid_actions_P2[chosen_index_P2].swap_player(),valid_actions_P1[chosen_index_P1].swap_player());
      simulated_deck_P1.choose_card_by_card_id(valid_actions_P1[chosen_index_P1].card_id);
      simulated_deck_P2.choose_card_by_card_id(valid_actions_P2[chosen_index_P2].card_id);
    }

    //ノードの末端ならループを抜けて、現在の盤面とデッキを返す
    if(pos_map.find({now_pos,chosen_index_P1,chosen_index_P2}) == pos_map.end()){
      return {now_pos,chosen_index_P1,chosen_index_P2,simulated_board_P1,simulated_board_P2,simulated_deck_P1,simulated_deck_P2};
    }
    else now_pos = pos_map[{now_pos,chosen_index_P1,chosen_index_P2}];
  }
}

template<class stage> void AI_ISMCTS<stage>::expansion(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2){
  pos_map[{leaf_pos,leaf_index_P1,leaf_index_P2}] = W_P1.size();
  W_P1.emplace_back(std::vector<float>());
  N_P1.emplace_back(std::vector<int>());
  W_P2.emplace_back(std::vector<float>());
  N_P2.emplace_back(std::vector<int>());
  parent_pos.emplace_back(leaf_pos,leaf_index_P1,leaf_index_P2);
}

template<class stage> float AI_ISMCTS<stage>::evaluation_random(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const {
  AI_random<stage> agent;
  while(leaf_board_P1.get_current_turn() <= 12){
    Choice<stage> action_P1 = agent.get_action(leaf_board_P1,leaf_board_P2,leaf_deck_P1);
    Choice<stage> action_P2 = agent.get_action(leaf_board_P2,leaf_board_P1,leaf_deck_P2);
    leaf_board_P1.put_both_cards_without_validation(action_P1,action_P2.swap_player());
    leaf_board_P2.put_both_cards_without_validation(action_P2,action_P1.swap_player());
  }
  int square_diff = leaf_board_P1.square_count_P1()-leaf_board_P1.square_count_P2();
  return std::clamp(square_diff,-1,1)+square_diff*diff_bonus;
}
template<class stage> float AI_ISMCTS<stage>::evaluation_greedy(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const {
  AI_greedy<stage> agent;
  while(leaf_board_P1.get_current_turn() <= 12){
    Choice<stage> action_P1 = agent.get_action(leaf_board_P1,leaf_board_P2,leaf_deck_P1);
    Choice<stage> action_P2 = agent.get_action(leaf_board_P2,leaf_board_P1,leaf_deck_P2);
    leaf_board_P1.put_both_cards_without_validation(action_P1,action_P2.swap_player());
    leaf_board_P2.put_both_cards_without_validation(action_P2,action_P1.swap_player());
  }
  int square_diff = leaf_board_P1.square_count_P1()-leaf_board_P1.square_count_P2();
  return std::clamp(square_diff,-1,1)+square_diff*diff_bonus;
}
template<class stage> float AI_ISMCTS<stage>::evaluation(Board<stage> leaf_board_P1,Board<stage> leaf_board_P2,Deck leaf_deck_P1,Deck leaf_deck_P2) const {
  assert(leaf_board_P1.get_current_turn() == leaf_board_P2.get_current_turn() && leaf_board_P1.get_current_turn() == leaf_deck_P1.get_current_turn() && leaf_board_P1.get_current_turn() == leaf_deck_P2.get_current_turn());
  return (use_evaluation_greedy ? 
  evaluation_greedy(leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2):evaluation_random(leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2));
}

template<class stage> void AI_ISMCTS<stage>::backup(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const float value_P1){
  W_P1[leaf_pos][leaf_index_P1] += value_P1;
  N_P1[leaf_pos][leaf_index_P1]++;
  //P2にはvalue_P1の正負を逆転した値を足す
  W_P2[leaf_pos][leaf_index_P2] += -value_P1;
  N_P2[leaf_pos][leaf_index_P2]++;

  int pos = leaf_pos,index_P1 = leaf_index_P1,index_P2 = leaf_index_P2;
  while(pos != root_pos){
    auto [new_pos,new_index_P1,new_index_P2] = parent_pos[pos];
    pos = new_pos;index_P1 = new_index_P1;index_P2 = new_index_P2;

    W_P1[pos][index_P1] += value_P1;
    N_P1[pos][index_P1]++;
    //P2にはvalue_P1の正負を逆転した値を足す
    W_P2[pos][index_P2] += -value_P1;
    N_P2[pos][index_P2]++;
  }
}

template<class stage> void AI_ISMCTS<stage>::simulate(){
  auto [leaf_pos,leaf_index_P1,leaf_index_P2,leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2] = selection();
  if(leaf_board_P1.get_current_turn() <= Board<stage>::TURN_MAX) expansion(leaf_pos,leaf_index_P1,leaf_index_P2);
  float value_P1 = evaluation(leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2);
  backup(leaf_pos,leaf_index_P1,leaf_index_P2,value_P1);
}
template<class stage> void AI_ISMCTS<stage>::set_root(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  assert(before_root_current_turn+1 == root_current_turn);
  this->root_board_P1 = board_P1;
  this->root_board_P2 = board_P2;
  used_node_count = 0;
  //deck内のカード一致チェック
  {
  std::vector<int> before_card_id_in_deck = deck_P1.get_deck(),now_card_id_in_deck = deck.get_deck();
  std::sort(before_card_id_in_deck.begin(),before_card_id_in_deck.end());
  std::sort(now_card_id_in_deck.begin(),now_card_id_in_deck.end());
  assert(before_card_id_in_deck == now_card_id_in_deck);
  }
  deck_P1 = deck;
  //ターン1で、redrawをしなかったなら、posを変更して引き継ぐ
  if(!did_redraw_deck && root_current_turn == 1){
    assert((pos_map[{root_pos,0,0}] == 1 || pos_map[{root_pos,0,0}] == 2));
    root_pos = pos_map[{root_pos,0,0}];return;
  }
  //そうでない場合、root_posは常に0
  //木をリセット
  root_pos = 0;
  W_P1.clear();W_P2.clear();
  N_P1.clear();N_P2.clear();
  parent_pos.clear();pos_map.clear();
  //rootのparentはなし {-1,-1,-1}とする
  //redrawフェーズ(root_current_turn==0)なら、redraw用のnodeを作る
  if(root_current_turn == 0){
    W_P1.emplace_back(std::vector<float>(2));
    N_P1.emplace_back(std::vector<int>(2));
    W_P2.emplace_back(std::vector<float>(1));
    N_P2.emplace_back(std::vector<int>(1));
    parent_pos.emplace_back(-1,-1,-1);
  }
  else{
    expansion(-1,-1,-1);
  }
}

template<class stage> void AI_ISMCTS<stage>::set_deck_P1(const Deck &deck_P1){
  this->deck_P1 = deck_P1;
  //valid_actions_P1を作成
  valid_actions_P1.clear();
  for(int card_id:deck_P1.get_deck()){
    for(bool is_SP_attack:{false,true}){
      for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
        valid_actions_P1.emplace_back(card_id,status_id,is_SP_attack);
      }
    }
  }
  //valid_actions_start_index_P1を作成
  int now_start_index = 0;
  for(int card_id:deck_P1.get_deck()){
    for(bool is_SP_attack:{false,true}){
      valid_actions_start_index_P1[card_id][is_SP_attack] = now_start_index;
      //通常置きの場合はパスを含める
      now_start_index += stage::card_status_size[card_id]+(is_SP_attack ? 0:1);
    }
  }
}
template<class stage> void AI_ISMCTS<stage>::set_deck_P2(const Deck &deck_P2){
  this->deck_P2 = deck_P2;
  //valid_actions_P2を作成
  valid_actions_P2.clear();
  for(int card_id:deck_P2.get_deck()){
    for(bool is_SP_attack:{false,true}){
      for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
        valid_actions_P2.emplace_back(card_id,status_id,is_SP_attack);
      }
    }
  }
  //valid_actions_start_index_P2を作成
  int now_start_index = 0;
  for(int card_id:deck_P2.get_deck()){
    for(bool is_SP_attack:{false,true}){
      valid_actions_start_index_P2[card_id][is_SP_attack] = now_start_index;
      //通常置きの場合はパスを含める
      now_start_index += stage::card_status_size[card_id]+(is_SP_attack ? 0:1);
    }
  }
}

template<class stage> bool AI_ISMCTS<stage>::redraw(const Deck &deck){
  root_current_turn = 0;
  set_root(Board<stage>(),Board<stage>(),deck);
 
  //シミュレーション
  for(int i=0;i<num_simulations;i++) simulate();

  //カードを入れ替えた方が探索回数が多い(勝率が高い)ならtrue
  bool do_redraw_deck = N_P1[root_pos][0] < N_P1[root_pos][1];
  //最後にdid_redraw_deckに戻り値を代入
  did_redraw_deck = do_redraw_deck;

  //before_root_current_turnを更新
  before_root_current_turn = root_current_turn;

  if(logging){
    std::cerr << "USED_NODE_COUNT:" << used_node_count << std::endl;
  }

  return do_redraw_deck;
}
template<class stage> Choice<stage> AI_ISMCTS<stage>::get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  root_current_turn = board_P1.get_current_turn();
  set_root(board_P1,board_P2,deck);

  for(int i=0;i<num_simulations;i++) simulate();

  //最も多く探索されたactionを選ぶ
  int max_N = 0,chosen_action_pos = -1;
  for(int i=0;i<N_P1[root_pos].size();i++){
    if(max_N < N_P1[root_pos][i]){
      max_N = N_P1[root_pos][i];
      chosen_action_pos = i;
    }
  }

  //before_root_current_turnを更新
  before_root_current_turn = root_current_turn;

  if(logging){
    std::cerr << max_N << " " << valid_actions_P1[chosen_action_pos].card_id << " " << W_P1[root_pos][chosen_action_pos]/N_P1[root_pos][chosen_action_pos] << std::endl;
    std::cerr << "USED_NODE_COUNT:" << used_node_count << std::endl;
  }

  return valid_actions_P1[chosen_action_pos];
}