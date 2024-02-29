#pragma once
#include <mpi.h>
#include <iostream>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <unistd.h>
#include "deck.hpp"
#include "map_elites.hpp"

template<class Individual,class Feature1,class Feature2,class Fitness>
class MAP_Elites_Parallel{
  static_assert(std::is_same<Individual,Deck>::value);
  static_assert(std::is_same<Feature1,float>::value);
  static_assert(std::is_same<Feature2,float>::value);
  static_assert(std::is_same<Fitness,float>::value);

  int n_procs; //ノード数
  int n_instances_each_proc; //1ノードあたりのインスタンス(Fitness値評価機構)数
  std::vector<std::vector<short>> is_available_instance; //iノード目のj instance目をselfplayに使えるか

  MAP_Elites<Individual,Feature1,Feature2,Fitness> map_elites;

 public:
  static constexpr float FINISHER_VALUE = -333;
  //マスター
  MAP_Elites_Parallel(
  int n_procs,int n_instances_each_proc,std::vector<std::vector<short>> &is_available_instance,
  int N,int G,int min_size,int max_size,int remap_frequency,
  Individual(*get_random_individual)(),Individual(*mutate)(const Individual&),std::tuple<Feature1,Feature2,Fitness>(*get_features_and_fitness)(const Individual&)):
  n_procs(n_procs),n_instances_each_proc(n_instances_each_proc),is_available_instance(is_available_instance),
  map_elites(N,G,min_size,max_size,remap_frequency,get_random_individual,mutate,get_features_and_fitness){
    map_elites.all_individuals.reserve(map_elites.N);
  }
  //MAP_Elitesをstep_count回進める
  void steps(const int step_count,const int write_interval,const std::string file_name,const bool overwrite = false);
};

template<class Individual,class Feature1,class Feature2,class Fitness>
void MAP_Elites_Parallel<Individual,Feature1,Feature2,Fitness>::steps(const int step_count,const int write_interval,const std::string file_name,const bool overwrite){
  int run_or_finish_step_count = 0;//評価中か評価が終わっている個体生成数
  int finish_step_count = 0;//評価が終わっている個体生成数
  //評価中のインスタンス
  std::vector<std::vector<short>> is_running(n_procs,std::vector<short>(n_instances_each_proc));
  //評価値レシーバー
  std::vector<std::vector<std::vector<float>>> receivers(n_procs,std::vector<std::vector<float>>(n_instances_each_proc,std::vector<float>(3)));
  //もう評価をしないインスタンス
  std::vector<std::vector<short>> is_exit(n_procs,std::vector<short>(n_instances_each_proc));

  std::vector<std::vector<MPI_Request>> requests(n_procs,std::vector<MPI_Request>(n_instances_each_proc));
  std::vector<std::vector<Deck>> evaluated_deck(n_procs,std::vector<Deck>(n_instances_each_proc));
  while(true){
    bool is_running_overall = false;
    for(int i=0;i<n_procs;i++){
      for(int j=0;j<n_instances_each_proc;j++){
        if(!is_available_instance[i][j] || is_exit[i][j]) continue;
        is_running_overall = true;
        //インスタンスで評価中
        if(is_running[i][j]){
          int is_done;
          MPI_Test(&requests[i][j],&is_done,MPI_STATUS_IGNORE);
          //結果が出たら、受け取る
          if(is_done){
            const std::vector<float> &receiver = receivers[i][j];//feature1,feature2,fitness
            Feature1 feature1 = receiver[0];
            Feature2 feature2 = receiver[1];
            Fitness fitness = receiver[2];
            // std::cerr << "Recv:" << i << " " << j << ":";
            for(int card_id:evaluated_deck[i][j].get_deck()) std::cerr << card_id << " ";
            std::cerr << ":" << feature1 << " " << feature2 << " " << fitness << std::endl;

            //map_elitesの方で処理
            map_elites.process_evaluated_individual(evaluated_deck[i][j],feature1,feature2,fitness);

            //finish_step_countに加算し、is_runningをfalseにする
            finish_step_count++;
            if(finish_step_count % write_interval == 0) map_elites.write_elites(file_name,overwrite);
            is_running[i][j] = false;
          }
        }
        if(!is_running[i][j]){
          //評価がしていないインスタンスに、評価するデッキを送り込む
          if(run_or_finish_step_count < step_count){
            evaluated_deck[i][j] = map_elites.generate_new_individual();

            //デッキを送信
            std::vector<int> card_id_in_deck = evaluated_deck[i][j].get_deck();
            MPI_Send(&card_id_in_deck[0],Deck::N_CARD_IN_DECK,MPI_INT,i,j,MPI_COMM_WORLD);
            // std::cerr << "Send:" << i << " " << j << ":";
            // for(int card_id:card_id_in_deck) std::cerr << card_id << " ";
            // std::cerr << std::endl;

            //run_or_finish_step_countに加算し、is_runningをtrueにする
            run_or_finish_step_count++;
            is_running[i][j] = true;
          }
          //既に終える必要があるなら、インスタンスに終了命令を出す
          else{
            std::vector<int> finisher(Deck::N_CARD_IN_DECK,FINISHER_VALUE);
            MPI_Send(&finisher[0],Deck::N_CARD_IN_DECK,MPI_INT,i,j,MPI_COMM_WORLD);

            //is_exitをtrueにする
            is_exit[i][j] = true;
          }
          //結果が返ってきたときのためにIRecvを構える
          //デッキを送ったときはfeature2つとfitness,finisherのときは全部-333が返ってくる
          MPI_Irecv(&receivers[i][j][0],3,MPI_FLOAT,i,10000+(i*n_instances_each_proc+j),MPI_COMM_WORLD,&requests[i][j]);
        }
      }
    }
    if(!is_running_overall) break;
  }
  //最後にインスタンスから終了報告が来たことの確認をする
  for(int i=0;i<n_procs;i++){
    for(int j=0;j<n_instances_each_proc;j++){
      if(!is_available_instance[i][j]) continue;
      MPI_Wait(&requests[i][j],MPI_STATUS_IGNORE);
      assert(is_exit[i][j]);
      assert(std::count(receivers[i][j].begin(),receivers[i][j].end(),FINISHER_VALUE) == 3);
    }
  }
}

template<class Individual,class Feature1,class Feature2,class Fitness>
class MAP_Elites_Instance{
  static_assert(std::is_same<Individual,Deck>::value);
  static_assert(std::is_same<Feature1,float>::value);
  static_assert(std::is_same<Feature2,float>::value);
  static_assert(std::is_same<Fitness,float>::value);

  int n_instances_each_proc,master_rank,rank,instance_id;
  std::tuple<Feature1,Feature2,Fitness>(*get_features_and_fitness)(const Individual&);
 public:
  MAP_Elites_Instance
  (int n_instances_each_proc,int master_rank,int rank,int instance_id,
  std::tuple<Feature1,Feature2,Fitness>(*get_features_and_fitness)(const Individual&)):
  n_instances_each_proc(n_instances_each_proc),master_rank(master_rank),rank(rank),instance_id(instance_id),
  get_features_and_fitness(get_features_and_fitness){}
  //MAP_Elites_Parallelのリクエストに応じる
  void steps();
};

template<class Individual,class Feature1,class Feature2,class Fitness>
void MAP_Elites_Instance<Individual,Feature1,Feature2,Fitness>::steps(){
  while(true){
    std::vector<int> card_id_in_deck(Deck::N_CARD_IN_DECK);
    MPI_Recv(&card_id_in_deck[0],Deck::N_CARD_IN_DECK,MPI_INT,master_rank,instance_id,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    
    std::vector<float> sender(3);
    
    //終了命令が来たら、終了報告をして抜ける
    if(card_id_in_deck[0] == MAP_Elites_Parallel<Individual,Feature1,Feature2,Fitness>::FINISHER_VALUE){
      std::fill(sender.begin(),sender.end(),MAP_Elites_Parallel<Individual,Feature1,Feature2,Fitness>::FINISHER_VALUE);
      MPI_Send(&sender[0],3,MPI_FLOAT,master_rank,10000+(rank*n_instances_each_proc+instance_id),MPI_COMM_WORLD);
      break;
    }
    //そうでない場合、featureとfitnessを評価する
    else{
      auto [feature1,feature2,fitness] = get_features_and_fitness(Deck(card_id_in_deck));
      sender[0] = feature1;
      sender[1] = feature2;
      sender[2] = fitness;
      MPI_Send(&sender[0],3,MPI_FLOAT,master_rank,10000+(rank*n_instances_each_proc+instance_id),MPI_COMM_WORLD);
    }
  }
}