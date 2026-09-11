#pragma once
#include "Typedef.h"
#include "Logger.h"

namespace jug
{

void    SetCoreLogger(Logger* _pLoggerOrNull);
Logger* GetCoreLoggerOrNull();

void LogCore(eLogLevel _level, StringView _msg);
void VLogCore(eLogLevel _level, StringView _msg, std::format_args _args);

template<typename... Args>
void LogCore(
    const eLogLevel              _level,
    std::format_string<Args...> _format,
    Args&&... _args)
{
    VLogCore(_level, _format.get(), std::make_format_args(_args...));
}

}   // namespace jug

// shortcut
#define JUG_CORE_LOG_TRACE(...) jug::Log(jug::eLogLevel::Trace, __VA_ARGS__)
#define JUG_CORE_LOG_DEBUG(...) jug::Log(jug::eLogLevel::Debug, __VA_ARGS__)
#define JUG_CORE_LOG_INFO(...)  jug::Log(jug::eLogLevel::Info, __VA_ARGS__)
#define JUG_CORE_LOG_WARN(...)  jug::Log(jug::eLogLevel::Warn, __VA_ARGS__)
#define JUG_CORE_LOG_ERROR(...) jug::Log(jug::eLogLevel::Error, __VA_ARGS__)
#define JUG_CORE_LOG_FATAL(...) jug::Log(jug::eLogLevel::Fatal, __VA_ARGS__)
