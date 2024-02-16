#pragma once
#include <vector>
#include <utility>
#include <unordered_map>
#include <numeric>
#include <functional>
#include <torch/script.h>
#include <torch/nn/functional/activation.h>
#include "AI_PV_ISMCTS.hpp"
#include "agent_group.hpp"
#include "board.hpp"
#include "common.hpp"
#include "choice.hpp"
#include "deck.hpp"
#include "hash.hpp"
#include "card.hpp"
#include "card_database.hpp"
#include "dirichlet.hpp"
#include "xorshift64.hpp"

template<class stage> class AI_PV_ISMCTS_Group : public Agent_Group<stage>{
  int group_size;
  int num_simulations;
  std::vector<AI_PV_ISMCTS<stage>> searchers;
  std::vector<std::tuple<int,int,int,Board<stage>,Board<stage>,Deck,Deck>> leaf_states;
  std::vector<float> value_P1s;

  //network
  torch::jit::Module model;
  std::vector<torch::Tensor> policy_action_tensor,policy_redraw_tensor,value_tensor;
  c10::Device device;
  c10::ScalarType dtype;

  void selection();
  void evaluation();
  void simulate();

public:
  AI_PV_ISMCTS_Group
  (int group_size,
  const torch::jit::Module &model,c10::Device device,c10::ScalarType dtype,
  int num_simulations,float diff_bonus,
  bool add_dirichlet_noise,float alpha,float eps,
  float c_base=19652,float c_init=1.25);

  constexpr int get_group_size() const override {return group_size;}
  std::vector<bool> redraws(const std::vector<Deck> &deck_P1s) override;
  std::vector<Choice<stage>> get_actions(const std::vector<Board<stage>> &board_P1s,const std::vector<Board<stage>> &board_P2s,const std::vector<Deck> &decks) override;
  std::vector<std::vector<float>> get_policy_redraws() const;
  std::vector<std::vector<std::pair<Choice<stage>,float>>> get_policy_actions() const;
  std::vector<std::vector<std::pair<Choice<stage>,float>>> get_policy_actions_network() const;

  void set_deck_P1s(const std::vector<Deck> &deck_P1s) override;
  void set_deck_P2s(const std::vector<Deck> &deck_P2s) override;
};

template<class stage> AI_PV_ISMCTS_Group<stage>::AI_PV_ISMCTS_Group(
int group_size,
const torch::jit::Module &model,c10::Device device,c10::ScalarType dtype,
int num_simulations,float diff_bonus,
bool add_dirichlet_noise,float alpha,float eps,
float c_base,float c_init):
group_size(group_size),num_simulations(num_simulations),
searchers(group_size,AI_PV_ISMCTS<stage>(std::nullopt,device,dtype,num_simulations,diff_bonus,add_dirichlet_noise,alpha,eps,c_base,c_init)),
value_P1s(group_size),
model(model),device(device),dtype(dtype){
  leaf_states.reserve(group_size);
}

template<class stage> void AI_PV_ISMCTS_Group<stage>::selection(){
  leaf_states.clear();
  for(int i=0;i<group_size;i++) leaf_states.emplace_back(searchers[i].selection());
}

template<class stage> void AI_PV_ISMCTS_Group<stage>::evaluation(){
  //入力を作成
  std::vector<float> batch_state_array(group_size*2*INPUT_C*stage::h*stage::w);
  for(int i=0;i<group_size;i++){
    auto &[leaf_pos,leaf_index_P1,leaf_index_P2,leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2] = leaf_states[i];
    //盤面が終了していたら、networkは用いない
    if(leaf_board_P1.current_turn > Board<stage>::TURN_MAX) continue;

    int expanded_pos = searchers[i].W_P1.size();
    bool is_redraw_phase = searchers[i].is_redraw_node(expanded_pos);
    std::vector<float> state_array_P1 = construct_image_vector(leaf_board_P1,leaf_deck_P1,leaf_deck_P2,is_redraw_phase);
    std::copy(state_array_P1.begin(),state_array_P1.end(),batch_state_array.begin()+i*(2*INPUT_C*stage::h*stage::w));
    std::vector<float> state_array_P2 = construct_image_vector(leaf_board_P2,leaf_deck_P2,leaf_deck_P1,is_redraw_phase);
    std::copy(state_array_P2.begin(),state_array_P2.end(),batch_state_array.begin()+i*(2*INPUT_C*stage::h*stage::w)+(INPUT_C*stage::h*stage::w));
  }
  torch::Tensor state_tensor = torch::tensor(torch::ArrayRef<float>(batch_state_array)).reshape({group_size*2,INPUT_C,stage::h,stage::w}).to(device,dtype);


  //推論
  auto output_tensor = model.forward({state_tensor}).toTensor();

  //取り出し、group_size個に分割し直す
  auto tensor_list = torch::split(output_tensor,{ACTION_SPACE_OF_EACH_CARD<stage>*N_card,2,1},1);
  policy_action_tensor = torch::nn::functional::softmax(tensor_list[0],torch::nn::functional::SoftmaxFuncOptions(1)).cpu().chunk(group_size);
  policy_redraw_tensor = torch::nn::functional::softmax(tensor_list[1],torch::nn::functional::SoftmaxFuncOptions(1)).cpu().chunk(group_size);
  value_tensor = tensor_list[2].cpu().chunk(group_size);
}

template<class stage> void AI_PV_ISMCTS_Group<stage>::simulate(){
  selection();
  
  evaluation();
  for(int i=0;i<group_size;i++){
    auto [leaf_pos,leaf_index_P1,leaf_index_P2,leaf_board_P1,leaf_board_P2,leaf_deck_P1,leaf_deck_P2] = leaf_states[i];

    float value_P1;
    //12ターン終了後は、直接勝敗を調べる
    if(leaf_board_P1.get_current_turn() > Board<stage>::TURN_MAX){
      value_P1 = leaf_board_P1.get_final_value(searchers[i].diff_bonus);
    }
    //そうでなければ、expansionしつつnetworkのvalueを参照
    else{
      //redrawノードであればexpansion_redraw
      int expanded_pos = searchers[i].W_P1.size();
      if(searchers[i].is_redraw_node(expanded_pos)) searchers[i].expansion_redraw(policy_redraw_tensor[i][0]);
      else searchers[i].expansion_action(leaf_pos,leaf_index_P1,leaf_index_P2,leaf_board_P1,policy_action_tensor[i][0],policy_action_tensor[i][1]);

      value_P1 = searchers[i].get_network_value(value_tensor[i]);
    }

    searchers[i].backup(leaf_pos,leaf_index_P1,leaf_index_P2,value_P1);
  }
}
template<class stage> std::vector<bool> AI_PV_ISMCTS_Group<stage>::redraws(const std::vector<Deck> &deck_P1s){
  assert(deck_P1s.size() == group_size);

  for(int i=0;i<group_size;i++){
    searchers[i].root_current_turn = 0;
    searchers[i].set_root(Board<stage>(),Board<stage>(),deck_P1s[i]);
  }

  //シミュレーション
  for(int i=0;i<num_simulations;i++) simulate();

  std::vector<bool> do_redraw_decks(group_size);

  for(int i=0;i<group_size;i++){
    do_redraw_decks[i] = searchers[i].get_do_redraw_deck();
    searchers[i].did_redraw_deck = do_redraw_decks[i];
    // searchers[i].get_policy_redraw();
  }
  
  //before_root_current_turnを更新
  for(int i=0;i<group_size;i++){
    searchers[i].before_root_current_turn = 0;
  }

  return do_redraw_decks;
}

template<class stage> std::vector<Choice<stage>> AI_PV_ISMCTS_Group<stage>::get_actions(const std::vector<Board<stage>> &board_P1s,const std::vector<Board<stage>> &board_P2s,const std::vector<Deck> &decks){
  assert(board_P1s.size() == group_size);
  assert(board_P2s.size() == group_size);
  assert(decks.size() == group_size);

  for(int i=0;i<group_size;i++){
    searchers[i].root_current_turn = board_P1s[i].get_current_turn();
    searchers[i].set_root(board_P1s[i],board_P2s[i],decks[i]);
  }

  //シミュレーション
  for(int i=0;i<num_simulations;i++) simulate();

  std::vector<Choice<stage>> best_actions;
  best_actions.reserve(group_size);

  for(int i=0;i<group_size;i++){
    best_actions.emplace_back(searchers[i].get_best_action());
    // searchers[i].get_policy_action();
  }

  //before_root_current_turnを更新
  for(int i=0;i<group_size;i++){
    searchers[i].before_root_current_turn = searchers[i].root_current_turn;
  }
  
  return best_actions;
}

template<class stage> std::vector<std::vector<float>> AI_PV_ISMCTS_Group<stage>::get_policy_redraws() const {
  std::vector<std::vector<float>> policy_redraws(group_size);
  for(int i=0;i<group_size;i++) policy_redraws[i] = searchers[i].get_policy_redraw();
  return policy_redraws;
}
template<class stage> std::vector<std::vector<std::pair<Choice<stage>,float>>> AI_PV_ISMCTS_Group<stage>::get_policy_actions() const {
  std::vector<std::vector<std::pair<Choice<stage>,float>>> policy_actions(group_size);
  for(int i=0;i<group_size;i++) policy_actions[i] = searchers[i].get_policy_action();
  return policy_actions;
}

template<class stage> std::vector<std::vector<std::pair<Choice<stage>,float>>> AI_PV_ISMCTS_Group<stage>::get_policy_actions_network() const {
  std::vector<std::vector<std::pair<Choice<stage>,float>>> policy_actions_network(group_size);
  for(int i=0;i<group_size;i++) policy_actions_network[i] = searchers[i].get_policy_action_network();
  return policy_actions_network;
}

template<class stage> void AI_PV_ISMCTS_Group<stage>::set_deck_P1s(const std::vector<Deck> &deck_P1s){
  assert(deck_P1s.size() == group_size);
  for(int i=0;i<group_size;i++) searchers[i].set_deck_P1(deck_P1s[i]);
}
template<class stage> void AI_PV_ISMCTS_Group<stage>::set_deck_P2s(const std::vector<Deck> &deck_P2s){
  assert(deck_P2s.size() == group_size);
  for(int i=0;i<group_size;i++) searchers[i].set_deck_P2(deck_P2s[i]);
}