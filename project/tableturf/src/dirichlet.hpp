#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>

inline std::vector<float> dirichlet_noise(float alpha,int length){
  static std::gamma_distribution<float> gamma(alpha,1.0);
  static std::mt19937_64 mt;

  //排他制御
	static std::mutex load_mutex;
	std::lock_guard<std::mutex> guard(load_mutex);

  std::vector<float> x(length);

  for(int i=0;i<length;i++) x[i] = gamma(mt);
  
  float sum_x = std::accumulate(x.begin(),x.end(),0.0);
  for(int i=0;i<length;i++) x[i] /= sum_x;

  return x;
}