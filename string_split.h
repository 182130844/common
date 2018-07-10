
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include <stdint.h>

namespace shadow {

	template<typename T>
	std::vector<T> string_split(const std::string& str, char delimiter,
		const std::function<T(const std::string&)>& transform) {
		std::vector<T> ret;
		if (str.empty()) return ret;
		std::istringstream iss(str);
		std::string temp;
		while (getline(iss, temp, delimiter)) {
			ret.emplace_back(transform(temp));
		}
		return ret;
	}

	std::vector<std::string> string_split(const std::string& s, char delimiter);
	std::vector<int32_t> string_split_int(const std::string& s, char delimiter);
}
