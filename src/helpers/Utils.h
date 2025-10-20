#pragma once

#ifndef __APPLE__
#include <charconv>
#endif
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>

namespace stfefane::utils {

constexpr auto kPI = std::numbers::pi_v<float>;
constexpr auto kTWO_PI = 2.f * kPI;
constexpr auto kPI_64 = std::numbers::pi_v<double>;
constexpr auto kTWO_PI_64 = 2. * kPI_64;

// Mapping helpers
enum class Mapping {
    Linear,
    Logarithmic,
    // Provides finer control around 0 for bipolar ranges by using an S-curve (odd-power) mapping
    BipolarSCurve
};

// [0,1] -> value
[[nodiscard]] inline double denormalize(Mapping mapping, double t, double min, double max) noexcept {
    t = std::clamp(t, 0.0, 1.0);
    switch (mapping) {
        case Mapping::Linear:
            return min + t * (max - min);
        case Mapping::Logarithmic: {
            // Guard against non-positive bounds; fallback to linear in that case
            if (min <= 0.0 || max <= 0.0) {
                return min + t * (max - min);
            }
            const double log_min = std::log(min);
            const double log_max = std::log(max);
            const double log_val = log_min + t * (log_max - log_min);
            return std::exp(log_val);
        }
        case Mapping::BipolarSCurve: {
            // Higher resolution around 0 for bipolar ranges using an odd-power S-curve.
            // Map t in [0,1] to s in [-1,1]
            const double s = 2.0 * t - 1.0;
            // Apply odd-power for symmetry; p > 1 compresses around 0 (finer control)
            constexpr double p = 3.0;
            const double s_shaped = std::copysign(std::pow(std::abs(s), p), s);
            // Map back to actual range via midpoint/half-range so that 0 sits at the center
            const double mid = 0.5 * (min + max);
            const double half = 0.5 * (max - min);
            return mid + s_shaped * half;
        }
    }
    // Fallback
    return min + t * (max - min);
}

// value -> [0,1]
[[nodiscard]] inline double normalize(Mapping mapping, double value, double min, double max) noexcept {
    const double range = max - min;
    if (range <= 0.0) return 0.0;
    switch (mapping) {
        case Mapping::Linear:
            return std::clamp((value - min) / range, 0.0, 1.0);
        case Mapping::Logarithmic: {
            if (min <= 0.0 || max <= 0.0 || value <= 0.0) {
                return std::clamp((value - min) / range, 0.0, 1.0);
            }
            const double log_min = std::log(min);
            const double log_max = std::log(max);
            const double log_v = std::log(value);
            const double t = (log_v - log_min) / (log_max - log_min);
            return std::clamp(t, 0.0, 1.0);
        }
        case Mapping::BipolarSCurve: {
            // Inverse of the S-curve denormalize above.
            const double mid = 0.5 * (min + max);
            const double half = 0.5 * (max - min);
            if (half <= 0.0) return 0.0;
            // Map value to [-1,1]
            double s_shaped = std::clamp((value - mid) / half, -1.0, 1.0);
            // Undo odd-power shaping
            constexpr double p = 3.0;
            const double s = std::copysign(std::pow(std::abs(s_shaped), 1.0 / p), s_shaped);
            // Map back to [0,1]
            return std::clamp(0.5 * (s + 1.0), 0.0, 1.0);
        }
    }
    return std::clamp((value - min) / range, 0.0, 1.0);
}

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

[[nodiscard]] inline double dbToLinear(double dB) {
    return std::pow(10.0, dB / 20.0);
}

[[nodiscard]] inline double linearToDB(double linear) {
    return 20.0 * std::log10(std::max(linear, 1e-10));
}

}