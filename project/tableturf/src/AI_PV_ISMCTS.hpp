#pragma once
#include <vector>
#include <utility>
#include <unordered_map>
#include <numeric>
#include <functional>
#include <torch/script.h>
#include <torch/nn/functional/activation.h>
#include <optional>
#include "agent.hpp"
#include "board.hpp"
#include "common.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "hash.hpp"
#include "card.hpp"
#include "card_database.hpp"
#include "dirichlet.hpp"
#include "xorshift64.hpp"

template<class stage> class AI_PV_ISMCTS_Group;

//常に手札からランダムに選んでパスするAI
template<class stage> class AI_PV_ISMCTS : public Agent<stage>{
 friend AI_PV_ISMCTS_Group<stage>;
 private:
  //PVの設定
  float c_base,c_init;
  bool add_dirichlet_noise;
  float alpha,eps;//ディリクレノイズ

  //MCTSの設定
  int num_simulations;
  float diff_bonus;//相手とのマス差にこれをかけた値が評価値に加算される

  //network
  std::optional<torch::jit::Module> model;
  c10::Device device;
  c10::ScalarType dtype;
  torch::Tensor policy_action_tensor,policy_redraw_tensor,value_tensor;

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

  // valid actions
  // simulationごとに変わる手札によって、実際に合法な手は変わる
  // redraw用ノードを除いて全ノード共通なので、1次元vector
  std::vector<Choice<stage>> valid_actions_P1,valid_actions_P2;

  //{card_id,status_id,is_SP_attack}に対応するW,Nを探す
  //W[valid_actions_start_index[card_id][is_SP_attack]+(is_SP_attack ? 0:1)+status_id]を参照する
  //デッキに含まれていないカードには-1を代入する
  std::vector<std::vector<int>> valid_actions_start_index_P1,valid_actions_start_index_P2;

  // prior probability
  std::vector<std::vector<float>> P_P1,P_P2;
  // P_card[i][P_card_index[card_id]][is_SP_attack] = sum(P[i][j] where valid_actions[j] == {card_id,?,is_SP_attack})
  std::vector<std::vector<std::vector<float>>> P_card_P1,P_card_P2;
  std::vector<int> P_card_index_P1,P_card_index_P2;
  // W is total action-value and Q=W/N is mean action-value
  // (自作のオセロAIとは異なり、子ではなく親が値を持つ)
  std::vector<std::vector<float>> W_P1,W_P2;
  // visit count
  std::vector<std::vector<int>> N_P1,N_P2;
  // ディリクレノイズ
  std::vector<float> noises;

  //parent_pos[child_pos] = {parentのpos,1Pのindex,2Pのindex}
  std::vector<std::tuple<int,int,int>> parent_pos;

  //pos_map[{parentのpos,1Pのindex,2Pのindex}] = child_pos
  std::unordered_map<std::tuple<int,int,int>,int,hash_tuple> pos_map;

  float PUCB_score(float w,int n,int n_parent,float p) const;

  //ChoiceからW,Nで参照するindexに変換
  constexpr int choice_to_valid_actions_index(const bool is_placement_P1,Choice<stage> choice) const;

  //{探索した葉のpos,葉で選んだindex_P1,葉で選んだindex_P2,葉での盤面P1,葉での盤面P2,葉でのdeck_P1,葉でのdeck_P2}を返す
  std::tuple<int,int,int,Board<stage>,Board<stage>,Deck,Deck> selection();
  bool is_redraw_node(int pos) const;
  
  //展開しつつPとP_cardを更新
  //policy_tensorは(N_card*(1(パス)+H*W*4(通常)+H*W*4(SP)))の形
  void expansion_action(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const torch::Tensor &policy_tensor_P1,const torch::Tensor &policy_tensor_P2);
  //policy_tensorは(2(redrawしない,redrawする))の形
  void expansion_redraw(const torch::Tensor &policy_tensor_P1);

  void evaluation(const Board<stage> &leaf_board_P1,const Board<stage> &leaf_board_P2,const Deck &leaf_deck_P1,const Deck &leaf_deck_P2);
  //networkによるスコアを求める
  float get_network_value() const;
  float get_network_value(const torch::Tensor &value_tensor_from_group) const;//AI_PV_ISMCTS_Group用

  void backup(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const float value_P1);
  
  //simulate,evaluation,expansion,backupをする
  void simulate();

  //シミュレーション後に、N_P1[root_pos]を参照してactionを選ぶ
  bool get_do_redraw_deck() const;
  Choice<stage> get_best_action() const;

  //木の初期化
  void set_root(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck);
 public:
  AI_PV_ISMCTS
  (const std::optional<torch::jit::Module> &model,c10::Device device,c10::ScalarType dtype,
  int num_simulations,float diff_bonus,
  bool add_dirichlet_noise,float alpha,float eps,
  float c_base=19652,float c_init=1.25
  );
  bool redraw(const Deck &deck) override;
  Choice<stage> get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck) override;
  //redraw後に呼び出し、policyを得る
  std::vector<float> get_policy_redraw() const;
  //get_action後に呼び出し、choiceとそれが選ばれた確率を得る
  std::vector<std::pair<Choice<stage>,float>> get_policy_action() const;
  //rootのnetworkから得られたpolicyを返す
  std::vector<std::pair<Choice<stage>,float>> get_policy_action_network() const;

  void set_deck_P1(const Deck &deck_P1) override;
  void set_deck_P2(const Deck &deck_P2) override;
};

template<class stage> AI_PV_ISMCTS<stage>::AI_PV_ISMCTS
(const std::optional<torch::jit::Module> &model,c10::Device device,c10::ScalarType dtype,
int num_simulations,float diff_bonus,
bool add_dirichlet_noise,float alpha,float eps,
float c_base,float c_init
):
c_base(c_base),c_init(c_init),
add_dirichlet_noise(add_dirichlet_noise),alpha(alpha),eps(eps),
num_simulations(num_simulations),diff_bonus(diff_bonus),
model(model),device(device),dtype(dtype),
valid_actions_start_index_P1(N_card+1,std::vector<int>(2,-1)),valid_actions_start_index_P2(N_card+1,std::vector<int>(2,-1)),
P_card_index_P1(N_card+1),P_card_index_P2(N_card+1){
  //rootとexpansionによって最終的にnum_simulations+1の長さになるので、その分メモリを確保する
  P_P1.reserve(num_simulations);P_P2.reserve(num_simulations);
  P_card_P1.reserve(num_simulations);P_card_P2.reserve(num_simulations);
  W_P1.reserve(num_simulations);W_P2.reserve(num_simulations);
  N_P1.reserve(num_simulations);N_P2.reserve(num_simulations);
  parent_pos.reserve(num_simulations);
  pos_map.reserve(num_simulations+1);//unordered_mapは処理系によっては1要素多く確保する必要があるらしいので、念の為
}

template<class stage> float AI_PV_ISMCTS<stage>::PUCB_score(float w,int n,int n_parent,float p) const {
  return
  (n==0 ? 0:w/n)+//Q
  (std::log((1+n_parent+c_base)/c_base)+c_init)*p*std::sqrt(n_parent)/(1+n);//U
}

template<class stage> constexpr int AI_PV_ISMCTS<stage>::choice_to_valid_actions_index(const bool is_placement_P1,Choice<stage> choice) const{
  return (is_placement_P1 ? valid_actions_start_index_P1:valid_actions_start_index_P2)[choice.card_id][choice.is_SP_attack]+(choice.is_SP_attack ? 0:1)+choice.status_id;
}

template<class stage> std::tuple<int,int,int,Board<stage>,Board<stage>,Deck,Deck> AI_PV_ISMCTS<stage>::selection(){
  Board<stage> simulated_board_P1 = root_board_P1,simulated_board_P2 = root_board_P2;
  Deck simulated_deck_P1 = deck_P1,simulated_deck_P2 = Deck(deck_P2,root_board_P1.used_cards_P2);
  assert(simulated_deck_P1.get_current_turn() == std::max(1,root_current_turn));
  assert(simulated_deck_P2.get_current_turn() == std::max(1,root_current_turn));

  int now_pos = root_pos;
  if(W_P1.size() == now_pos){
    //探索1回目　すぐ返す (そもそも葉が存在しないのでleaf_pos=-1)
    return {-1,-1,-1,simulated_board_P1,simulated_board_P2,simulated_deck_P1,simulated_deck_P2};
  }
  while(true){
    //親の探索回数
    int n_parent = std::accumulate(N_P1[now_pos].begin(),N_P1[now_pos].end(),0);

    int chosen_index_P1 = -1,chosen_index_P2 = -1;

    //P1とP2の手札
    std::vector<int> card_id_in_hand_P1 = simulated_deck_P1.get_hand();
    std::vector<int> card_id_in_hand_P2 = simulated_deck_P2.get_hand();

    if(is_redraw_node(now_pos)) assert(W_P1[now_pos].size() == 2 && W_P2[now_pos].size() == 1);

    
    std::tuple<bool,int&,std::vector<int>&,std::vector<float>&,std::vector<int>&,std::vector<Choice<stage>>&,std::vector<float>&,std::vector<std::vector<float>>&,std::vector<int>&> looper[2] =
    {{true,chosen_index_P1,card_id_in_hand_P1,W_P1[now_pos],N_P1[now_pos],valid_actions_P1,P_P1[now_pos],P_card_P1[now_pos],P_card_index_P1},
    {false,chosen_index_P2,card_id_in_hand_P2,W_P2[now_pos],N_P2[now_pos],valid_actions_P2,P_P2[now_pos],P_card_P2[now_pos],P_card_index_P2}};
    for(auto [is_placement_P1,chosen_index,card_id_in_hand,W,N,valid_actions,P,P_card,P_card_index]:looper){
      //PUCBスコアが最大となる子を求める
      float max_PUCB_score = -1e9;
      if(is_redraw_node(now_pos)){
        for(int i=0;i<W.size();i++){
          float now_PUCB_score = PUCB_score(W[i],N[i],n_parent,P[i]);
          if(max_PUCB_score < now_PUCB_score){
            max_PUCB_score = now_PUCB_score;
            chosen_index = i;
          }
        }
      }
      else{
        //Policyを対象の(card,is_SP_attack)に限定する
        //sum_valid_policyで各policyで割った値を真のpolicyとする
        float sum_valid_policy = 0;
        for(int card_id:card_id_in_hand){
          sum_valid_policy += P_card[P_card_index[card_id]][false];
          if(simulated_board_P1.is_enough_SP_point(is_placement_P1,card_id)){
            sum_valid_policy += P_card[P_card_index[card_id]][true];
          }
        }

        for(int card_id:card_id_in_hand)
        for(bool is_SP_attack:{false,true}){
          //SPポイントが不足している場合は除外
          if(is_SP_attack && !simulated_board_P1.is_enough_SP_point(is_placement_P1,card_id)) continue;
          
          for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
            int now_index = choice_to_valid_actions_index(is_placement_P1,{card_id,status_id,is_SP_attack});
            assert(valid_actions[now_index].status_id == status_id);
            //既に手札にあるカードになっている
            //設置不可能なChoiceは除外
            if(!simulated_board_P1.is_valid_placement_without_SP_point_validation(is_placement_P1,valid_actions[now_index])) continue;
            
            //rootなら、Pにディリクレノイズを足す
            float now_PUCB_score = PUCB_score(W[now_index],N[now_index],n_parent,P[now_index]/sum_valid_policy+(now_pos == root_pos ? noises[now_index]:0));
            if(max_PUCB_score < now_PUCB_score){
              max_PUCB_score = now_PUCB_score;
              chosen_index = now_index;
            }
          }
        }
        // if(now_pos == root_pos && chosen_index == choice_to_valid_actions_index(is_placement_P1,{card_id_in_hand[0],-1,false})){
        //   std::cerr << PUCB_score(W[0],N[0],n_parent,P[0]/sum_valid_policy+noises[0]) << " " << W[0] << " " << N[0] << " " << n_parent << " " << P[0]/sum_valid_policy << " " << noises[0] << " " << std::endl;
        // }
      }
    }
    assert(chosen_index_P1 != -1);
    assert(chosen_index_P2 != -1);

    //盤面、デッキの更新
    if(is_redraw_node(now_pos)){
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
template<class stage> bool AI_PV_ISMCTS<stage>::is_redraw_node(int pos) const {
  return root_current_turn == 0 && pos == 0;
}
template<class stage> void AI_PV_ISMCTS<stage>::expansion_action(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const torch::Tensor &policy_tensor_P1,const torch::Tensor &policy_tensor_P2){
  //展開
  int now_pos = W_P1.size();
  pos_map[{leaf_pos,leaf_index_P1,leaf_index_P2}] = now_pos;
  P_P1.emplace_back(std::vector<float>(valid_actions_P1.size()));
  P_card_P1.emplace_back(std::vector<std::vector<float>>(Deck::N_CARD_IN_DECK,std::vector<float>(2)));
  W_P1.emplace_back(std::vector<float>(valid_actions_P1.size()));
  N_P1.emplace_back(std::vector<int>(valid_actions_P1.size()));
  P_P2.emplace_back(std::vector<float>(valid_actions_P2.size()));
  P_card_P2.emplace_back(std::vector<std::vector<float>>(Deck::N_CARD_IN_DECK,std::vector<float>(2)));
  W_P2.emplace_back(std::vector<float>(valid_actions_P2.size()));
  N_P2.emplace_back(std::vector<int>(valid_actions_P2.size()));
  parent_pos.emplace_back(leaf_pos,leaf_index_P1,leaf_index_P2);

  //policy_tensorをP,P_cardに反映させる
  std::tuple<bool,std::vector<float>&,std::vector<std::vector<float>>&,std::vector<int>&,std::vector<int>> looper[2] = 
  {{true,P_P1[now_pos],P_card_P1[now_pos],P_card_index_P1,deck_P1.get_deck()},
  {false,P_P2[now_pos],P_card_P2[now_pos],P_card_index_P2,deck_P2.get_deck()}};
  for(auto [is_placement_P1,P,P_card,P_card_index,card_id_in_deck]:looper){
    std::vector<float> P_network;
    torch::Tensor policy_tensor = (is_placement_P1 ? policy_tensor_P1:policy_tensor_P2);
    if(policy_tensor.dtype() == torch::kFloat16){
      P_network = std::vector<float>(policy_tensor.data_ptr<torch::Half>(),policy_tensor.data_ptr<torch::Half>()+(ACTION_SPACE_OF_EACH_CARD<stage>*N_card));
    }
    else{
      P_network = std::vector<float>(policy_tensor.data_ptr<float>(),policy_tensor.data_ptr<float>()+(ACTION_SPACE_OF_EACH_CARD<stage>*N_card));
    }
    for(int card_id:card_id_in_deck)
    for(bool is_SP_attack:{false,true}){
      for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
        int P_network_index = choice_to_policy_action_network_index<stage>({card_id,status_id,is_SP_attack});
        int P_index = choice_to_valid_actions_index(is_placement_P1,{card_id,status_id,is_SP_attack});

        //代入
        P[P_index] = P_network[P_network_index];

        P_card[P_card_index[card_id]][is_SP_attack] += P_network[P_network_index];
      }
    }
    //rootのP1だけには、Pにディリクレノイズを載せる
    if(now_pos == root_pos && is_placement_P1 && add_dirichlet_noise){
      noises = dirichlet_noise(alpha,P.size());
      // for(int card_id:card_id_in_deck)
      // for(bool is_SP_attack:{false,true}){
      //   for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
      //     int P_index = choice_to_valid_actions_index(is_placement_P1,{card_id,status_id,is_SP_attack});
      //     P_card[P_card_index[card_id]][is_SP_attack] -= P[P_index];
      //     P[P_index] = (1-eps)*P[P_index]+eps*noises[P_index];
      //     P_card[P_card_index[card_id]][is_SP_attack] += P[P_index];
      //   }
      // }
    }
  }
}
template<class stage> void AI_PV_ISMCTS<stage>::expansion_redraw(const torch::Tensor &policy_tensor_P1){
  //展開(というか最初のノード)
  int now_pos = P_P1.size();
  assert(now_pos == 0);
  P_P1.emplace_back(std::vector<float>(2));
  P_card_P1.emplace_back(std::vector<std::vector<float>>());//使わない
  W_P1.emplace_back(std::vector<float>(2));
  N_P1.emplace_back(std::vector<int>(2));
  P_P2.emplace_back(std::vector<float>(1));//この値は0でOK
  P_card_P2.emplace_back(std::vector<std::vector<float>>());//使わない
  W_P2.emplace_back(std::vector<float>(1));
  N_P2.emplace_back(std::vector<int>(1));
  parent_pos.emplace_back(-1,-1,-1);

  //policy_tensorをPに反映させる
  if(policy_tensor_P1.dtype() == torch::kFloat16){
    P_P1[0] = std::vector<float>(policy_tensor_P1.data_ptr<torch::Half>(),policy_tensor_P1.data_ptr<torch::Half>()+2);
  }
  else{
    P_P1[0] = std::vector<float>(policy_tensor_P1.data_ptr<float>(),policy_tensor_P1.data_ptr<float>()+2);
  }
}

template<class stage> void AI_PV_ISMCTS<stage>::evaluation(const Board<stage> &leaf_board_P1,const Board<stage> &leaf_board_P2,const Deck &leaf_deck_P1,const Deck &leaf_deck_P2){
  //入力を作成
  std::vector<float> batch_state_array(2*INPUT_C*stage::h*stage::w);
  std::vector<float> state_array_P1 = construct_image_vector(leaf_board_P1,leaf_deck_P1,leaf_deck_P2);
  std::copy(state_array_P1.begin(),state_array_P1.end(),batch_state_array.begin());
  std::vector<float> state_array_P2 = construct_image_vector(leaf_board_P2,leaf_deck_P2,leaf_deck_P1);
  std::copy(state_array_P2.begin(),state_array_P2.end(),batch_state_array.begin()+INPUT_C*stage::h*stage::w);
  torch::Tensor state_tensor = torch::tensor(torch::ArrayRef<float>(batch_state_array)).reshape({2,INPUT_C,stage::h,stage::w}).to(device,dtype);

  //推論
  assert(model.has_value());
  auto output_tensor = model.value().forward({state_tensor}).toTensor();

  //取り出す
  auto tensor_list = torch::split(output_tensor,{ACTION_SPACE_OF_EACH_CARD<stage>*N_card,2,1},1);
  policy_action_tensor = torch::nn::functional::softmax(tensor_list[0],torch::nn::functional::SoftmaxFuncOptions(1)).cpu();
  policy_redraw_tensor = torch::nn::functional::softmax(tensor_list[1],torch::nn::functional::SoftmaxFuncOptions(1)).cpu();
  value_tensor = tensor_list[2].cpu();
}
template<class stage> float AI_PV_ISMCTS<stage>::get_network_value() const{
  //P1のvalueとP2の-valueの平均を取る
  return (value_tensor[0].item().toFloat()-value_tensor[1].item().toFloat())/2;
}
template<class stage> float AI_PV_ISMCTS<stage>::get_network_value(const torch::Tensor &value_tensor_from_group) const{
  //P1のvalueとP2の-valueの平均を取る
  return (value_tensor_from_group[0].item().toFloat()-value_tensor_from_group[1].item().toFloat())/2;
}

template<class stage> void AI_PV_ISMCTS<stage>::backup(const int leaf_pos,const int leaf_index_P1,const int leaf_index_P2,const float value_P1){
  if(leaf_pos == -1) return;//木が空だった場合は何もしない

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

template<class stage> void AI_PV_ISMCTS<stage>::simulate(){
  //Selection
  auto [leaf_pos,leaf_index_P1,leaf_index_P2,leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2] = selection();

  float value_P1;
  //12ターン終了後は、直接勝敗を調べる
  if(leaf_board_P1.get_current_turn() > Board<stage>::TURN_MAX){
    value_P1 = leaf_board_P1.get_final_value(diff_bonus);
  }
  //そうでなければ、expansionしつつnetworkのvalueを参照
  else{
    //policy_action_tensor,policy_redraw_tensor,value_tensorを取得
    evaluation(leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2);

    int expanded_pos = W_P1.size();
    //redrawノードであればexpansion_redraw
    if(is_redraw_node(expanded_pos)) expansion_redraw(policy_redraw_tensor[0]);
    //そうでなければexpansion_action
    else expansion_action(leaf_pos,leaf_index_P1,leaf_index_P2,policy_action_tensor[0],policy_action_tensor[1]);

    value_P1 = get_network_value();
  }
  backup(leaf_pos,leaf_index_P1,leaf_index_P2,value_P1);
  //std::cerr << "OK" << std::endl;
}
template<class stage> void AI_PV_ISMCTS<stage>::set_root(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  assert(before_root_current_turn+1 == root_current_turn);
  this->root_board_P1 = board_P1;
  this->root_board_P2 = board_P2;
  deck_P1 = deck;
  //木の再利用はしないので、root_posは常に0
  //木をリセット
  root_pos = 0;
  P_P1.clear();P_P2.clear();
  P_card_P1.clear();P_card_P2.clear();
  W_P1.clear();W_P2.clear();
  N_P1.clear();N_P2.clear();
  parent_pos.clear();pos_map.clear();
}

template<class stage> void AI_PV_ISMCTS<stage>::set_deck_P1(const Deck &deck_P1){
  this->deck_P1 = deck_P1;
  //valid_actions_P1を作成
  {
  valid_actions_P1.clear();
  for(int card_id:deck_P1.get_deck()){
    for(bool is_SP_attack:{false,true}){
      for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
        valid_actions_P1.emplace_back(card_id,status_id,is_SP_attack);
      }
    }
  }
  }
  //valid_actions_start_index_P1を作成
  {
  int now_start_index = 0;
  for(int card_id:deck_P1.get_deck()){
    for(bool is_SP_attack:{false,true}){
      valid_actions_start_index_P1[card_id][is_SP_attack] = now_start_index;
      //通常置きの場合はパスを含める
      now_start_index += stage::card_status_size[card_id]+(is_SP_attack ? 0:1);
    }
  }
  }
  //P_card_index_P1を作成
  {
  int now_index = 0;
  for(int card_id:deck_P1.get_deck()){
    P_card_index_P1[card_id] = now_index++;
  }
  }
}
template<class stage> void AI_PV_ISMCTS<stage>::set_deck_P2(const Deck &deck_P2){
  this->deck_P2 = deck_P2;
  //valid_actions_P2を作成
  {
  valid_actions_P2.clear();
  for(int card_id:deck_P2.get_deck()){
    for(bool is_SP_attack:{false,true}){
      for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
        valid_actions_P2.emplace_back(card_id,status_id,is_SP_attack);
      }
    }
  }
  }
  //valid_actions_start_index_P2を作成
  {
  int now_start_index = 0;
  for(int card_id:deck_P2.get_deck()){
    for(bool is_SP_attack:{false,true}){
      valid_actions_start_index_P2[card_id][is_SP_attack] = now_start_index;
      //通常置きの場合はパスを含める
      now_start_index += stage::card_status_size[card_id]+(is_SP_attack ? 0:1);
    }
  }
  }
  //P_card_index_P2を作成
  {
  {
  int now_index = 0;
  for(int card_id:deck_P2.get_deck()){
    P_card_index_P2[card_id] = now_index++;
  }
  }
  }
}

template<class stage> bool AI_PV_ISMCTS<stage>::redraw(const Deck &deck){
  root_current_turn = 0;
  set_root(Board<stage>(),Board<stage>(),deck);
 
  //シミュレーション
  for(int i=0;i<num_simulations;i++) simulate();

  //カードを入れ替えた方が探索回数が多い(勝率が高い)ならtrue
  bool do_redraw_deck = get_do_redraw_deck();
  //最後にdid_redraw_deckに戻り値を代入
  did_redraw_deck = do_redraw_deck;

  //before_root_current_turnを更新
  before_root_current_turn = root_current_turn;

  return do_redraw_deck;
}
template<class stage> Choice<stage> AI_PV_ISMCTS<stage>::get_action(const Board<stage> &board_P1,const Board<stage> &board_P2,const Deck &deck){
  root_current_turn = board_P1.get_current_turn();
  set_root(board_P1,board_P2,deck);

  for(int i=0;i<num_simulations;i++) simulate();

  //最も多く探索されたactionを選ぶ
  Choice<stage> best_action = get_best_action();

  //before_root_current_turnを更新
  before_root_current_turn = root_current_turn;

  return best_action;
}

template<class stage> bool AI_PV_ISMCTS<stage>::get_do_redraw_deck() const {
  return N_P1[root_pos][0] < N_P1[root_pos][1];
}

template<class stage> Choice<stage> AI_PV_ISMCTS<stage>::get_best_action() const {
  int max_N = 0,chosen_action_pos = -1;
  for(int i=0;i<N_P1[root_pos].size();i++){
    if(max_N < N_P1[root_pos][i]){
      max_N = N_P1[root_pos][i];
      chosen_action_pos = i;
    }
  }
  std::cerr << max_N << " " << valid_actions_P1[chosen_action_pos].card_id << " " << W_P1[root_pos][chosen_action_pos]/N_P1[root_pos][chosen_action_pos] << std::endl;
  return valid_actions_P1[chosen_action_pos];
}

template<class stage> std::vector<float> AI_PV_ISMCTS<stage>::get_policy_redraw() const {
  assert(root_current_turn == 0);
  assert(N_P1[root_pos].size() == 2);
  int N_root = std::accumulate(N_P1[root_pos].begin(),N_P1[root_pos].end(),0);
  std::vector<float> policy_redraw(2);
  for(int i=0;i<2;i++) policy_redraw[i] = float(N_P1[root_pos][i])/N_root;

  for(float policy:policy_redraw) std::cerr << policy << " ";
  std::cerr << std::endl;
  return policy_redraw;
}
template<class stage> std::vector<std::pair<Choice<stage>,float>> AI_PV_ISMCTS<stage>::get_policy_action() const {
  assert(root_current_turn > 0);
  assert(N_P1[root_pos].size() == valid_actions_P1.size());
  int N_root = std::accumulate(N_P1[root_pos].begin(),N_P1[root_pos].end(),0);

  std::vector<std::pair<Choice<stage>,float>> policy_action;

  for(int card_id:deck_P1.get_hand())
  for(bool is_SP_attack:{false,true}){
    for(int status_id=(is_SP_attack ? 0:-1);status_id<stage::card_status_size[card_id];status_id++){
      int now_index = choice_to_valid_actions_index(true,{card_id,status_id,is_SP_attack});
      policy_action.emplace_back(valid_actions_P1[now_index],float(N_P1[root_pos][now_index])/N_root);
    }
  }

  for(auto [choice,policy]:policy_action) std::cerr << policy << " ";
  std::cerr << std::endl;
  return policy_action;
}

template<class stage> std::vector<std::pair<Choice<stage>,float>> AI_PV_ISMCTS<stage>::get_policy_action_network() const {
  assert(root_current_turn > 0);
  assert(P_P1[root_pos].size() == valid_actions_P1.size());
  
  std::vector<std::pair<Choice<stage>,float>> policy_action_network;
  policy_action_network.reserve(valid_actions_P1.size());
  for(int i=0;i<valid_actions_P1.size();i++){
    policy_action_network.emplace_back(valid_actions_P1[i],P_P1[root_pos][i]);
  }
  return policy_action_network;
}