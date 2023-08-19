#pragma once
#include <cstdint>
#include <random>
std::uint64_t xorshift64()
{
	static std::random_device rng;
	static std::mt19937_64 engine(rng());
	static std::uint64_t x = engine();
	x ^= x << 13;
	x ^= x >> 7;
	return x ^= x << 17;
}