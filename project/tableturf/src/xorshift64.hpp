#pragma once
#include <cstdint>
#include <random>
#include <mutex>
std::uint64_t xorshift64()
{
	static std::random_device rng;
	static std::mt19937_64 engine(rng());
	static std::uint64_t x = engine();
	//排他制御
	// static std::mutex load_mutex;
	// std::lock_guard<std::mutex> guard(load_mutex);

	x ^= x << 13;
	x ^= x >> 7;
	return x ^= x << 17;
}