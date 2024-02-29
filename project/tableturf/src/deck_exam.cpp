//最終チェック
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <mpi.h>
#include "deck_database.hpp"
#include "testplay2.hpp"
int main(int argc,char* argv[]){
  int mpi_thread_limit;
  MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&mpi_thread_limit);
  assert(MPI_THREAD_MULTIPLE == mpi_thread_limit);
  int n_procs,rank;
  MPI_Comm_size(MPI_COMM_WORLD,&n_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int num_games = atoi(argv[1]);
  int num_threads = atoi(argv[2]);
  int num_simulations = atoi(argv[3]);
  std::string in_file = argv[4];
  std::string out_file = argv[5];
  float diff_bonus = 0.001;
  assert(num_games % test_deckset.size() == 0);
  int num_games_each_opponent_deck = num_games / test_deckset.size();
  std::vector<Deck> deck_P2s;
  for(const Deck &opponent_deck:test_deckset){
    for(int i=0;i<num_games_each_opponent_deck;i++) deck_P2s.emplace_back(opponent_deck);
  }
  std::ifstream fin(in_file);
  using stage = Main_Street;
  Initializer initializer;
  int N_deck;//調べるデッキの数
  fin >> N_deck;
  std::vector<float> win_rates(N_deck);
  std::vector<MPI_Request> requests(N_deck);
  if(rank == 0){
    for(int i=0;i<N_deck;i++){
      MPI_Irecv(&win_rates[i],1,MPI_FLOAT,i%n_procs,i,MPI_COMM_WORLD,&requests[i]);
    }
  }
  for(int i=0;i<N_deck;i++){
    std::vector<int> card_id_in_deck(Deck::N_CARD_IN_DECK);
    float map_win_rate;
    fin >> map_win_rate;
    for(int i=0;i<Deck::N_CARD_IN_DECK;i++) fin >> card_id_in_deck[i];

    if(i%n_procs != rank) continue;
    std::vector<Deck> deck_P1s(num_games,card_id_in_deck);
    float win_rate = testplay2<stage>(num_games,num_threads,num_simulations,diff_bonus,deck_P1s,deck_P2s);
    MPI_Send(&win_rate,1,MPI_FLOAT,0,i,MPI_COMM_WORLD);
  }
  if(rank == 0){
    MPI_Waitall(N_deck,&requests[0],MPI_STATUSES_IGNORE);
    std::ofstream fout(out_file);
    float sum_win_rate = 0;
    for(int i=0;i<N_deck;i++){
      sum_win_rate += win_rates[i];
      fout << win_rates[i] << std::endl;
    }
    fout << "sum:" << sum_win_rate << std::endl;
    fout << "avg:" << sum_win_rate/N_deck << std::endl;
  }
  MPI_Finalize();
}