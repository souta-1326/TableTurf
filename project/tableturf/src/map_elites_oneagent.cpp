#include <omp.h>
#include <mpi.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include "card_database.hpp"
#include "deck_database.hpp"
#include "map_elites_parallel.hpp"
#include "map_elites_function.hpp"
#include "xorshift64.hpp"
#include "testplay2.hpp"
using stage = Main_Street;
using Feature1 = float;
using Feature2 = float;
using Fitness = float;
int n_procs,num_cpus_each_proc,rank,n_instances_each_proc;
int N,G,min_size,max_size,remap_frequency,write_interval;
int num_games_each_evaluation,ISMCTS_num_simulations;
std::string elites_file_name;
float diff_bonus = 0.001;
template<class stage> std::tuple<Feature1,Feature2,Fitness> get_features_and_fitness(const Deck &deck){
  //feature1 カードのマス数の平均
  std::vector<int> card_id_in_deck = deck.get_deck();
  int sum_n_square = 0;
  for(int card_id:card_id_in_deck) sum_n_square += cards[card_id].N_SQUARE;
  float feature1 = float(sum_n_square)/Deck::N_CARD_IN_DECK;

  //feature2 カードのマス数の分散
  int sum_n_square_pow2 = 0;
  for(int card_id:card_id_in_deck) sum_n_square_pow2 += cards[card_id].N_SQUARE*cards[card_id].N_SQUARE;
  float feature2 = (float(sum_n_square_pow2)/Deck::N_CARD_IN_DECK)-feature1*feature1;

  //fitness 勝率
  std::vector<Deck> deck_P1s(num_games_each_evaluation);
  int opponent_deckset_size = test_deckset.size();
  assert(num_games_each_evaluation % opponent_deckset_size == 0);
  int num_games_each_opponent_deck = num_games_each_evaluation / opponent_deckset_size;
  std::vector<Deck> deck_P2s;
  for(const Deck &opponent_deck:test_deckset){
    for(int i=0;i<num_games_each_opponent_deck;i++) deck_P2s.emplace_back(opponent_deck);
  }
  int num_threads = num_cpus_each_proc / n_instances_each_proc;
  float fitness = testplay2<stage>(num_games_each_evaluation,num_threads,ISMCTS_num_simulations,diff_bonus,deck_P1s,deck_P2s);

  //今日はとりあえずここまで　デバッグはしてない
  return {feature1,feature2,fitness};
}
//argv - num_cpus_each_proc n_instances_each_proc N G min_size max_size remap_frequency 
int main(int argc,char* argv[]){
  //MPI初期化
  int mpi_thread_limit;
  MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&mpi_thread_limit);
  assert(MPI_THREAD_MULTIPLE == mpi_thread_limit);
  MPI_Comm_size(MPI_COMM_WORLD,&n_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  num_cpus_each_proc = atoi(argv[1]);
  n_instances_each_proc = atoi(argv[2]);
  N = atoi(argv[3]);
  G = atoi(argv[4]);
  min_size = 2;
  max_size = 20;
  remap_frequency = atoi(argv[5]);
  write_interval = atoi(argv[6]);
  num_games_each_evaluation = atoi(argv[7]);
  ISMCTS_num_simulations = atoi(argv[8]);
  elites_file_name = argv[9];

  Initializer initializer;
  omp_set_max_active_levels(3);
  omp_set_num_threads(n_instances_each_proc);
  std::vector<std::vector<short>> is_available_instance(n_procs,std::vector<short>(n_instances_each_proc,true));
  is_available_instance[0][0] = false;
  #pragma omp parallel
  {
    int instance_id = omp_get_thread_num();
    if(rank == 0 && instance_id == 0){
      MAP_Elites_Parallel<Deck,float,float,float> map_elites_p
      (n_procs,n_instances_each_proc,is_available_instance,N,G,min_size,max_size,remap_frequency,get_random_individual,mutate,get_features_and_fitness<stage>);
      map_elites_p.steps(N,write_interval,elites_file_name);
    }
    else{
      MAP_Elites_Instance<Deck,float,float,float> map_elites_i(n_instances_each_proc,0,rank,instance_id,get_features_and_fitness<stage>);
      map_elites_i.steps();
    }
  }
  MPI_Finalize();
}
//local:mpirun -np 2 build/test_map_elites 4 2 10000 100 100 2500 8 1000 data/elites.txt
//ABCI:mpirun -np x build/test_map_elites 40 4 10000 100 100 2500 80 10000