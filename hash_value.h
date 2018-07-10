

//
//  方法源自boost hash
// 

#pragma once

#include <functional>

namespace shadow {
	template<typename T>
	inline void hash_combine(std::size_t& seed, const T& value) {
		seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template<typename T>
	inline void hash_value(std::size_t& seed, const T& value) {
		hash_combine(seed, value);
	}

	template<typename T, typename... Args>
	inline void hash_value(std::size_t& seed, const T& value, const Args&... args) {
		hash_combine(seed, value);
		hash_value(seed, args...);
	}
	template<typename... Args>
	inline std::size_t hash_value(const Args&... args) {
		std::size_t seed = 0;
		hash_value(seed, args...);
		return seed;
	}
}
