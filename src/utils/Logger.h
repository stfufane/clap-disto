#pragma once

#if DEBUG
#include <string_view>
#include <spdlog/cfg/helpers.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "Folders.h"

namespace stfefane::utils {

using std::literals::operator ""sv;

// Ensure we have a default configuration
constexpr auto LOGS_DEFAULT_CONF = R"(fs=info,dsp=info,ui=debug,param=info)"sv;

#define LOG_DEBUG(type, message, ...) \
    spdlog::get(type)->debug(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_INFO(type, message, ...) \
    spdlog::get(type)->info(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_WARN(type, message, ...) \
    spdlog::get(type)->warn(message __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR(type, message, ...) \
    spdlog::get(type)->error(message __VA_OPT__(,) __VA_ARGS__)


inline void initLoggers() {
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("fs", stdout_sink));
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("dsp", stdout_sink));
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("ui", stdout_sink));
    spdlog::register_or_replace(std::make_shared<spdlog::logger>("param", stdout_sink));
    stdout_sink->set_pattern("[%T.%f][%t][%n][%^%l%$] %v");
    // Always setup default levels so logger can always be used.
    spdlog::cfg::helpers::load_levels(std::string(LOGS_DEFAULT_CONF));
}

// Fetch the log levels from a configuration file
inline void loadLogLevelsFromFile() {
    const auto log_settings = folders::readFileContent(folders::LOG_SETTINGS_FILE);
    if (log_settings.empty()) {
        LOG_WARN("ui", "Impossible to load settings file at {}", folders::LOG_SETTINGS_FILE.generic_string());
        // Write default configuration and leave, it has already been set in initLogger
        if (!folders::writeFileContent(folders::LOG_SETTINGS_FILE, LOGS_DEFAULT_CONF)) {
            LOG_ERROR("fs", "Error writing to log settings file");
            return;
        }
    }
    spdlog::cfg::helpers::load_levels(log_settings);
}

}

#else

#define LOG_DEBUG(type, message, ...)
#define LOG_INFO(type, message, ...)
#define LOG_WARN(type, message, ...)
#define LOG_ERROR(type, message, ...)

#endif
