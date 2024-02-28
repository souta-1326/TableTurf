#pragma once
#include <map>
#include <utility>
#include <optional>
#include <fstream>
#include "xorshift64.hpp"
#include "deck.hpp"

template<class Individual,class Feature1,class Feature2,class Fitness> class MAP_Elites_Parallel;

template<class Individual,class Feature1,class Feature2,class Fitness>
class MAP_Elites{
  static_assert(std::is_same<Individual,Deck>::value);
  
  friend class MAP_Elites_Parallel<Individual,Feature1,Feature2,Fitness>;

  int N;//生成する個体の数
  int G;//最初にランダムに生成する個体の数
  int min_size,max_size;//セルの数をmin_sizeからmax_sizeまで、線形に変化させる
  int remap_frequency;//remap()を何個体生成ごとに呼び出すか

  std::vector<std::vector<std::optional<std::pair<Individual,Fitness>>>> MAP;
  std::vector<std::tuple<Individual,Feature1,Feature2,Fitness>> all_individuals;
  int current_size;//現在のセルの数
  std::vector<Feature1> boundaries1;//境界1
  std::vector<Feature2> boundaries2;//境界2
  
  Individual(*get_random_individual)();//個体ランダム生成関数
  Individual(*mutate)(const Individual&);//突然変異関数
  std::tuple<Feature1,Feature2,Fitness>(*get_features_and_fitness)(const Individual&);//特徴関数

  Individual generate_new_individual() const ;

  int get_feature1_index(const Feature1 &feature) const ;
  int get_feature2_index(const Feature2 &feature) const ;

  void add(const std::tuple<Individual,Feature1,Feature2,Fitness> &individual_with_parameters);//個体をマップに加える(弾かれることもある)
  void remap();//MAPの境界の数と場所を調整
  void process_evaluated_individual(const Individual &individual,const Feature1 feature1,const Feature2 feature2,const Fitness fitness);
 public:
  MAP_Elites
  (int N,int G,int min_size,int max_size,int remap_frequency,
  Individual(*get_random_individual)(),Individual(*mutate)(const Individual&),std::tuple<Feature1,Feature2,Fitness>(*get_features_and_fitness)(const Individual&)):
  N(N),G(G),min_size(min_size),max_size(max_size),remap_frequency(remap_frequency),
  get_random_individual(get_random_individual),mutate(mutate),get_features_and_fitness(get_features_and_fitness){
    all_individuals.reserve(N);
  }
  void step();
  void write_elites(const std::string file_name,const bool overwrite = false);//書き出し
  std::vector<std::vector<std::optional<std::pair<Individual,Fitness>>>> get_map(){return MAP;}
};

template<class Individual,class Feature1,class Feature2,class Fitness> int MAP_Elites<Individual,Feature1,Feature2,Fitness>::get_feature1_index(const Feature1 &feature) const {
  return lower_bound(boundaries1.begin(),boundaries1.end(),feature)-boundaries1.begin();
}
template<class Individual,class Feature1,class Feature2,class Fitness> int MAP_Elites<Individual,Feature1,Feature2,Fitness>::get_feature2_index(const Feature2 &feature) const {
  return lower_bound(boundaries2.begin(),boundaries2.end(),feature)-boundaries2.begin();
}

template<class Individual,class Feature1,class Feature2,class Fitness> void MAP_Elites<Individual,Feature1,Feature2,Fitness>::add(const std::tuple<Individual,Feature1,Feature2,Fitness> &individual_with_parameters){
  const auto &[individual,feature1,feature2,fitness] = individual_with_parameters;
  int feature1_index = get_feature1_index(feature1);
  int feature2_index = get_feature2_index(feature2);

  assert(0 <= feature1_index && feature1_index < current_size);
  assert(0 <= feature2_index && feature2_index < current_size);
  //セルに個体が存在しない、またはfitness値が上回る場合は更新
  if(MAP[feature1_index][feature2_index] == std::nullopt || fitness > std::get<1>(MAP[feature1_index][feature2_index].value())){
    MAP[feature1_index][feature2_index] = std::make_pair(individual,fitness);
  }
}

template<class Individual,class Feature1,class Feature2,class Fitness> void MAP_Elites<Individual,Feature1,Feature2,Fitness>::remap(){
  float progress = float(all_individuals.size())/N;
  current_size = std::min(max_size, min_size + int((max_size-min_size)*progress));

  //MAPのサイズ変更
  MAP.assign(current_size,std::vector<std::optional<std::pair<Individual,Fitness>>>(current_size,std::nullopt));

  //境界の設定
  {
  std::vector<Feature1> features(all_individuals.size());
  for(int i=0;i<all_individuals.size();i++) features[i] = std::get<1>(all_individuals[i]);
  sort(features.begin(),features.end());

  boundaries1.assign(current_size-1,0);
  //current_size-1個の境界でcurrent_size個のセルに分割する
  for(int i=0;i<current_size-1;i++){
    int sample_index = (i+1)*all_individuals.size()/current_size;
    boundaries1[i] = features[sample_index];
  }
  }

  {
  std::vector<Feature2> features(all_individuals.size());
  for(int i=0;i<all_individuals.size();i++) features[i] = std::get<2>(all_individuals[i]);
  sort(features.begin(),features.end());

  boundaries2.assign(current_size-1,0);
  //current_size-1個の境界でcurrent_size個のセルに分割する
  for(int i=0;i<current_size-1;i++){
    int sample_index = (i+1)*all_individuals.size()/current_size;
    boundaries2[i] = features[sample_index];
  }
  }

  //全個体をマップに突っ込む
  for(const auto &individual_with_parameters : all_individuals) add(individual_with_parameters);
}

template<class Individual,class Feature1,class Feature2,class Fitness> void MAP_Elites<Individual,Feature1,Feature2,Fitness>::process_evaluated_individual(const Individual &individual,const Feature1 feature1,const Feature2 feature2,const Fitness fitness){
  all_individuals.emplace_back(individual,feature1,feature2,fitness);
  assert(all_individuals.size() <= N);

  if(all_individuals.size() % remap_frequency == 1) remap();
  else add({individual,feature1,feature2,fitness});
}

template<class Individual,class Feature1,class Feature2,class Fitness> Individual MAP_Elites<Individual,Feature1,Feature2,Fitness>::generate_new_individual() const {
  //G個体まではランダムに生成
  if(all_individuals.size() < G) return get_random_individual();
  //それ以降は、MAPからランダムに個体をとって突然変異させる
  else{
    while(true){
      int feature1_index = xorshift64()%current_size,feature2_index = xorshift64()%current_size;
      if(MAP[feature1_index][feature2_index] != std::nullopt) return mutate(std::get<0>(MAP[feature1_index][feature2_index].value()));
    }
  }
}


template<class Individual,class Feature1,class Feature2,class Fitness> void MAP_Elites<Individual,Feature1,Feature2,Fitness>::step(){
  Individual new_individual = generate_new_individual();
  auto [feature1,feature2,fitness] = get_features_and_fitness(new_individual);

  process_evaluated_individual(new_individual,feature1,feature2,fitness);
}

template<class Individual,class Feature1,class Feature2,class Fitness> void MAP_Elites<Individual,Feature1,Feature2,Fitness>::write_elites(const std::string file_name,const bool overwrite){
  std::ofstream fout(file_name,(overwrite ? std::ios::out : std::ios::app));
  for(int i=0;i<current_size;i++){
    for(int j=0;j<current_size;j++){
      if(MAP[i][j] == std::nullopt) continue;
      fout << std::get<1>(MAP[i][j].value()) << ":";
      for(int card_id:std::get<0>(MAP[i][j].value()).get_deck()) fout << card_id << " ";
      fout << std::endl;
    }
  }
  fout << std::endl;
}