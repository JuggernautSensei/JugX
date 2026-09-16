#pragma once
#include "Logger.h"

namespace jug
{

void SetCoreLogger(Logger* _pLoggerOrNull);

void LogCore(eLogLevel _level, StringView _msg);
void VLogCore(eLogLevel _level, StringView _msg, std::format_args _args);

template<typename... Args>
void LogCore(
    const eLogLevel             _level,
    std::format_string<Args...> _format,
    Args&&... _args)
{
    VLogCore(_level, _format.get(), std::make_format_args(_args...));
}

}   // namespace jug

// shortcut
#define JUG_CORE_LOG_TRACE(...) jug::LogCore(jug::eLogLevel::Trace, __VA_ARGS__)
#define JUG_CORE_LOG_DEBUG(...) jug::LogCore(jug::eLogLevel::Debug, __VA_ARGS__)
#define JUG_CORE_LOG_INFO(...)  jug::LogCore(jug::eLogLevel::Info, __VA_ARGS__)
#define JUG_CORE_LOG_WARN(...)  jug::LogCore(jug::eLogLevel::Warn, __VA_ARGS__)
#define JUG_CORE_LOG_ERROR(...) jug::LogCore(jug::eLogLevel::Error, __VA_ARGS__)
#define JUG_CORE_LOG_FATAL(...) jug::LogCore(jug::eLogLevel::Fatal, __VA_ARGS__)
