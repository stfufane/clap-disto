#pragma once

#if DEBUG
#include <filesystem>
#include <string_view>
#include <spdlog/cfg/helpers.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "Folders.h"

namespace stfefane::utils {

using std::literals::operator ""sv;

// Ensure we have a default configuration
constexpr auto LOGS_DEFAULT_CONF = R"(dsp=info,ui=debug,param=info)"sv;

// Fetch the log levels from a configuration file
inline void loadLevels() {
    auto log_settings = folders::readFileContent(folders::LOG_SETTINGS_FILE);
    if (log_settings.empty()) {
        log_settings = LOGS_DEFAULT_CONF;
    }
    spdlog::cfg::helpers::load_levels(log_settings);
}

inline void initLoggers() {
    if (!std::filesystem::exists(folders::LOG_SETTINGS_FILE)) {
        if (!folders::writeFileContent(folders::LOG_SETTINGS_FILE, LOGS_DEFAULT_CONF)) {
            throw std::runtime_error("Error writing to log file");
        }
    }

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("dsp", stdout_sink));
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("ui", stdout_sink));
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("param", stdout_sink));
    stdout_sink->set_pattern("[%T.%f][%t][%n][%^%l%$] %v");

    loadLevels();
}

#define LOG_DEBUG(type, message, ...) \
    spdlog::get(type)->debug(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_INFO(type, message, ...) \
    spdlog::get(type)->info(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_WARN(type, message, ...) \
    spdlog::get(type)->warn(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR(type, message, ...) \
    spdlog::get(type)->error(message __VA_OPT__(,) __VA_ARGS__)

}

#else

#define LOG_DEBUG(type, message, ...)
#define LOG_INFO(type, message, ...)
#define LOG_WARN(type, message, ...)
#define LOG_ERROR(type, message, ...)

#endif
