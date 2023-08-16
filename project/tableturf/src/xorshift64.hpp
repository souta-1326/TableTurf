#pragma once
#include<cstdint>
std::uint64_t xorshift64()
{
	static std::uint64_t x = 88172645463325252;
	x ^= x << 13;
	x ^= x >> 7;
	return x ^= x << 17;
}