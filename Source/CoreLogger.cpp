#include "CoreLogger.h"

#include "Logger.h"

namespace jug
{

namespace
{
    Logger* g_pLoggerOrNull = nullptr;
}

void SetCoreLogger(
    Logger* _pLoggerOrNull)
{
    g_pLoggerOrNull = _pLoggerOrNull;
}

Logger* GetCoreLogger()
{
    return g_pLoggerOrNull;
}

void LogToCoreImpl_(
    const eLogLevel        _level,
    const StringView       _msg,
    const std::format_args _args)
{
    if (g_pLoggerOrNull)
    {
        g_pLoggerOrNull->LogV(_level, _msg, _args);
    }
}

void LogCore(
    const eLogLevel  _level,
    const StringView _msg)
{
    if (g_pLoggerOrNull)
    {
        g_pLoggerOrNull->Log(_level, _msg);
    }
}

}   // namespace jug