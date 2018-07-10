
#include "string_split.h"

namespace shadow {

	std::vector<std::string> string_split(const std::string& s, char delimiter) {
		return string_split<std::string>(s, delimiter, [](const std::string& _s)-> std::string { return _s; });
	}
	std::vector<int32_t> string_split_int(const std::string& s, char delimiter) {
		return string_split<int32_t>(s, delimiter, [](const std::string& _s)-> int32_t { return std::stoi(_s); });
	}
}
