#include "CoreLogger.h"

#include "Logger.h"
#include "Typedef.h"
#include <format>

namespace jug
{

namespace
{

    Logger* g_pLoggerOrNull = nullptr;

    void LogToCoreImpl_(
        const eLogLevel        _level,
        const StringView       _msg,
        const std::format_args _args)
    {
        if (g_pLoggerOrNull)
        {
            g_pLoggerOrNull->VLog(_level, _msg, _args);
        }
    }
}   // namespace

void SetCoreLogger(
    Logger* _pLoggerOrNull)
{
    g_pLoggerOrNull = _pLoggerOrNull;
}

Logger* GetCoreLoggerOrNull()
{
    return g_pLoggerOrNull;
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

void VLogCore(
    const eLogLevel        _level,
    const StringView       _msg,
    const std::format_args _args)
{
    if (g_pLoggerOrNull)
    {
        g_pLoggerOrNull->VLog(_level, _msg, _args);
    }
}

}   // namespace jug