#pragma once
#include <time.h>
#include <torch/script.h>
#include <torch/cuda.h>
#if __has_include(<torch/mps.h>)
#include <torch/mps.h>
#endif
struct Timer{
  std::chrono::system_clock::time_point timer;
  c10::Device device;
  int gpu_id;
  double get_time(){
    #if __has_include(<torch/mps.h>)
    if(device.is_mps()) torch::mps::synchronize();
    else torch::cuda::synchronize(gpu_id);
    #else
    torch::cuda::synchronize(gpu_id);
    #endif
    std::chrono::system_clock::time_point new_time = std::chrono::system_clock::now();
    double duration = double(std::chrono::duration_cast<std::chrono::milliseconds>(new_time-timer).count())/1000;
    timer = new_time;
    return duration;
  }
  Timer(c10::Device device,int gpu_id):device(device),gpu_id(gpu_id){get_time();}
};