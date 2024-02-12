#pragma once
#include <torch/script.h>
#include <torch/cuda.h>
#include <mutex>
#include <iostream>
#include "common.hpp"

#if __has_include(<torch_tensorrt/torch_tensorrt.h>)
#include <torch_tensorrt/torch_tensorrt.h>
#else
#warning <torch_tensorrt/torch_tensorrt.h> is not found
#endif
torch::jit::Module load_model(const char* model_path,int gpu_id,c10::Device device,c10::ScalarType dtype){
  //GPUの排他制御
  static std::mutex load_mutex;
  std::lock_guard<std::mutex> guard(load_mutex);

  torch::jit::Module model = torch::jit::load(model_path);
  model.to(device,dtype);
  model.eval();

  return model;
}
template<class stage> torch::jit::Module load_TensorRT_model(const char* model_path,int gpu_id,c10::Device device,c10::ScalarType dtype,int opt_batch_size,int max_batch_size){
  //GPUの排他制御
  static std::mutex load_mutex;
  std::lock_guard<std::mutex> guard(load_mutex);

  torch::jit::Module precompiled_model = load_model(model_path,gpu_id,device,dtype);

  //TensorRTがない場合は、そのまま返す
  #if not __has_include(<torch_tensorrt/torch_tensorrt.h>)
  return precompiled_model;
  #else
  std::vector<int64_t> in_min = { 1, INPUT_C, stage::h, stage::w};
  std::vector<int64_t> in_opt = { opt_batch_size, INPUT_C, stage::h, stage::w};
  std::vector<int64_t> in_max = { max_batch_size, INPUT_C, stage::h, stage::w};
  torch_tensorrt::Input input(in_min, in_opt, in_max, dtype);
  torch_tensorrt::ts::CompileSpec compile_spec({ input });
  compile_spec.enabled_precisions = { dtype };
  compile_spec.require_full_compilation = true;
  compile_spec.device.gpu_id = gpu_id;
  compile_spec.truncate_long_and_double = true;
  return torch_tensorrt::ts::compile(precompiled_model, compile_spec);
  #endif
}