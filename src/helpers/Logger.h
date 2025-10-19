#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace stfefane::utils {

inline void initLoggers() {
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    spdlog::register_logger(std::make_shared<spdlog::logger>("dsp", stdout_sink));
    spdlog::register_logger(std::make_shared<spdlog::logger>("ui", stdout_sink));
    spdlog::register_logger(std::make_shared<spdlog::logger>("param", stdout_sink));
    stdout_sink->set_pattern("[%T.%f][%t][%n][%^%l%$] %v");

#if DEBUG
    spdlog::get("dsp")->set_level(spdlog::level::warn);
    spdlog::get("ui")->set_level(spdlog::level::debug);
    spdlog::get("param")->set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::off);
#endif
}

#define LOG_DEBUG(type, message, ...) \
    spdlog::get(type)->debug(message, __VA_ARGS__)

#define LOG_INFO(type, message, ...) \
    spdlog::get(type)->info(message, __VA_ARGS__)

#define LOG_WARN(type, message, ...) \
    spdlog::get(type)->warn(message, __VA_ARGS__)

#define LOG_ERROR(type, message, ...) \
    spdlog::get(type)->error(message, __VA_ARGS__)

}