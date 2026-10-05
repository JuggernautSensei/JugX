#pragma once
#define SPDLOG_USE_STD_FORMAT
#include <spdlog/spdlog.h>

#define JUG_CORE_LOG_TRACE(...)   spdlog::trace(__VA_ARGS__)
#define JUG_CORE_LOG_INFO(...)    spdlog::info(__VA_ARGS__)
#define JUG_CORE_LOG_WARN(...)    spdlog::warn(__VA_ARGS__)
#define JUG_CORE_LOG_ERROR(...)   spdlog::error(__VA_ARGS__)
#define JUG_CORE_LOG_FATAL(...)   spdlog::critical(__VA_ARGS__)
#define JUG_CORE_LOG(_level, ...) spdlog::log(_level, __VA_ARGS__)
