#pragma once
#include <string>

namespace stfefane::utils {

[[nodiscard]] inline double stringToDouble(const std::string& str) {
#ifdef __APPLE__
    char* end;
    double result = std::strtod(str.c_str(), &end);
    if (end == str.c_str()) {
        throw std::invalid_argument("Invalid number format");
    }
    return result;
#else
    double result = 0.0;
    auto [ptr, ec] = std::from_chars(str.c_str(), str.c_str() + str.size(), result);
    if (ec != std::errc{}) {
        throw std::invalid_argument("Invalid number format");
    }
    return result;
#endif
}

}