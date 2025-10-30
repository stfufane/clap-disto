#pragma once

#ifndef __APPLE__
#include <charconv>
#endif
#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <clap/stream.h>

namespace stfefane::utils {

constexpr auto kPI = std::numbers::pi_v<float>;
constexpr auto kTWO_PI = 2.f * kPI;
constexpr auto kPI_64 = std::numbers::pi_v<double>;
constexpr auto kTWO_PI_64 = 2. * kPI_64;

template<typename FLOATING_TYPE>
[[nodiscard]] bool almostEqual(FLOATING_TYPE a, FLOATING_TYPE b) {
    constexpr auto kEpsilon = static_cast<FLOATING_TYPE>(1e-12);
    return std::fabs(a - b) < kEpsilon;
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

inline bool writeToClapStream(const std::string& str, const clap_ostream* stream) {
    if (!stream || !stream->write) {
        return false;
    }

    // CLAP streams may have size limitations, so we need to write in chunks
    const auto* buffer = str.data();

    auto remaining = str.size();
    while (remaining > 0) {
        // Try to write remaining bytes
        const auto written = stream->write(stream, buffer + (str.size() - remaining), remaining);
        if (written < 0) {
            return false; // Write error occurred
        }
        remaining -= written;
    }

    return true;
}

inline std::optional<std::vector<char>> readFromClapStream(const clap_istream* stream) {
    if (!stream || !stream->read) {
        return std::nullopt;
    }

    // Read the JSON string from the stream in chunks
    constexpr auto chunkSize = 4096;
    std::vector<char> buffer;
    buffer.reserve(chunkSize);
    char chunk[chunkSize];

    while (true) {
        int64_t bytesRead = stream->read(stream, chunk, chunkSize);
        if (bytesRead < 0) {
            return std::nullopt; // Read error
        }
        if (bytesRead == 0) {
            break; // End of stream
        }
        buffer.insert(buffer.end(), chunk, chunk + bytesRead);
    }

    // No state to load but I guess that's ok
    if (buffer.empty()) {
        return std::nullopt;
    }

    buffer.push_back('\0'); // Ensure buffer is null-terminated
    return buffer;
}

}