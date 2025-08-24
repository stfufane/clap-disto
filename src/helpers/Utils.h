#pragma once

#include <charconv>
#include <numbers>
#include <stdexcept>
#include <string>

namespace stfefane::utils {

constexpr auto kPI = std::numbers::pi_v<float>;
constexpr auto kTWO_PI = 2.f * kPI;
constexpr auto kPI_64 = std::numbers::pi_v<double>;
constexpr auto kTWO_PI_64 = 2. * kPI_64;

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