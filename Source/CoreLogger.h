#pragma once
#include "Typedef.h"
#include "Logger.h"

namespace jug
{

void    SetCoreLogger(Logger* _pLoggerOrNull);
Logger* GetCoreLogger();

void LogCore(eLogLevel _level, StringView _msg);
void LogCoreV(eLogLevel _level, StringView _msg, std::format_args _args);

template<typename... TArgs>
void LogCore(
    const eLogLevel              _level,
    std::format_string<TArgs...> _format,
    TArgs&&... _args)
{
    LogCoreV(_level, _format.get(), std::make_format_args(_args...));
}

}   // namespace jug

// shortcut
#define JUG_LOG_CORE_TRACE(...) jug::LogCore(jug::eLogLevel::Trace, __VA_ARGS__)
#define JUG_LOG_CORE_DEBUG(...) jug::LogCore(jug::eLogLevel::Debug, __VA_ARGS__)
#define JUG_LOG_CORE_INFO(...)  jug::LogCore(jug::eLogLevel::Info, __VA_ARGS__)
#define JUG_LOG_CORE_WARN(...)  jug::LogCore(jug::eLogLevel::Warn, __VA_ARGS__)
#define JUG_LOG_CORE_ERROR(...) jug::LogCore(jug::eLogLevel::Error, __VA_ARGS__)
#define JUG_LOG_CORE_FATAL(...) jug::LogCore(jug::eLogLevel::Fatal, __VA_ARGS__)
