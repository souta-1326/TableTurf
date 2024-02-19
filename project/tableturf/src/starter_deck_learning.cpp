#include <iostream>
#include <algorithm>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <torch/script.h>
#if __has_include(<torch/mps.h>)
#include <torch/mps.h>
#endif
#include "load_model.hpp"
#include "selfplay.hpp"
#include "testplay.hpp"
#include "buffer.hpp"
#include "timer.hpp"
#include "deck_database.hpp"
template<class stage> void run(
const bool USE_TENSORRT,
int num_threads_each_gpu,
int gpu_id,
int num_games_in_parallel,int num_games_in_selfplay,int num_games_in_testplay,
int PV_ISMCTS_num_simulations,int simple_ISMCTS_num_simulations,
float diff_bonus,
float dirichlet_alpha,float eps,
char* model_path,Buffer<stage> &buffer,std::ofstream &log_out,
c10::Device device,c10::ScalarType dtype){
  Timer timer(device,gpu_id);
  static std::mutex o_mutex;//出力用排他制御
  int max_batch_size = num_games_in_parallel*4;//P1とP2、それぞれDUCT
  torch::jit::Module model = 
  (USE_TENSORRT ? 
  load_TensorRT_model<stage>(model_path,gpu_id,device,dtype,max_batch_size,max_batch_size):
  load_model(model_path,gpu_id,device,dtype)
  );
  o_mutex.lock();
  std::cout << gpu_id << ":model loaded" << std::endl;
  log_out << gpu_id << ":compilation time: " << timer.get_time() << std::endl;
  o_mutex.unlock();
  //testplay(1つのgpuだけに任せる)
  if(gpu_id == 0){
    int num_testplay_func = num_games_in_testplay/num_games_in_parallel;
    assert(num_games_in_testplay%num_games_in_parallel == 0);
    float sum_win_rate = 0;
    for(int i=0;i<num_testplay_func;i++){
      sum_win_rate += testplay<stage>(num_games_in_testplay,num_threads_each_gpu,model,device,dtype,PV_ISMCTS_num_simulations,simple_ISMCTS_num_simulations,0,std::vector<Deck>(num_games_in_testplay),std::vector<Deck>(num_games_in_testplay));
    }
    float win_rate = sum_win_rate/num_testplay_func;
    o_mutex.lock();
    log_out << "Testplay: " << win_rate << std::endl;
    log_out << gpu_id << ":testplay time: " << timer.get_time() << std::endl;
    o_mutex.unlock();
  }
  //selfplay
  static int num_games_in_selfplay_done = 0;
  static std::mutex selfplay_count_mutex;
  while(true){
    selfplay<stage>(num_games_in_parallel,num_threads_each_gpu,model,device,dtype,PV_ISMCTS_num_simulations,dirichlet_alpha,eps,diff_bonus,std::vector<Deck>(num_games_in_parallel),std::vector<Deck>(num_games_in_parallel),buffer);
    bool break_loop = false;
    selfplay_count_mutex.lock();
    num_games_in_selfplay_done += num_games_in_parallel;
    if(num_games_in_selfplay_done >= num_games_in_selfplay) break_loop = true;
    selfplay_count_mutex.unlock();
    if(break_loop) break;
  }
  o_mutex.lock();
  log_out << gpu_id << ":selfplay time: " << timer.get_time() << std::endl;
  o_mutex.unlock();
}
//ここでselfplayを回す
int main(int argc,char *argv[]){
  assert(argc == 16);
  int num_cpus = atoi(argv[1]);
  int num_gpus = atoi(argv[2]);
  char* device_str = argv[3];
  int num_games_in_parallel = atoi(argv[4]);
  int num_games_in_selfplay = atoi(argv[5]);
  int num_games_in_testplay = atoi(argv[6]);
  int buffer_size = atoi(argv[7]);
  int PV_ISMCTS_num_simulations = atoi(argv[8]);
  int simple_ISMCTS_num_simulations = atoi(argv[9]);
  float diff_bonus = atof(argv[10]);
  float dirichlet_alpha = atof(argv[11]);
  float eps = atof(argv[12]);
  char* model_path = argv[13];
  char* data_path = argv[14];
  char* log_path = argv[15];

  using stage = Main_Street;
  int num_threads_each_gpu = num_cpus*2/num_gpus;
  bool use_cuda = (strcmp(device_str,"cuda")==0);
  std::ofstream log_out(log_path,std::ios::app);//上書き
  Buffer<stage> buffer(buffer_size);//学習用データ
  // load model
  constexpr bool USE_TENSORRT = false;
  #if __has_include(<torch_tensorrt/torch_tensorrt.h>)
  if(USE_TENSORRT) std::cout << "torch_tensorrt available" << std::endl;
  #endif
  std::vector<std::thread> runner_threads;
  for(int gpu_id=0;gpu_id<num_gpus;gpu_id++){
    c10::Device device = torch::Device((use_cuda ? torch::kCUDA:torch::kMPS),gpu_id);
    c10::ScalarType dtype = (use_cuda ? torch::kFloat16:torch::kFloat32);
    runner_threads.emplace_back
    (run<stage>,
    USE_TENSORRT,
    num_threads_each_gpu,
    gpu_id,
    num_games_in_parallel,num_games_in_selfplay,num_games_in_testplay,
    PV_ISMCTS_num_simulations,simple_ISMCTS_num_simulations,
    diff_bonus,
    dirichlet_alpha,eps,
    model_path,std::ref(buffer),std::ref(log_out),
    device,dtype);
  }
  for(auto &thread:runner_threads) thread.join();
  buffer.write(data_path,false);
}