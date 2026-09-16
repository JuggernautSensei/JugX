#include "pch.h"
#include "CoreLogger.h"

#include <format>

#include "ConsoleLogger.h"
#include "Logger.h"
#include "Typedef.h"

namespace jug
{

namespace
{
    Logger*       g_pLoggerOrNull   = nullptr;
    ConsoleLogger g_fallback        = {};
    bool          g_bFallbackInited = false;

    void InitFallbackIfNeed_()
    {
        if (!g_bFallbackInited)
        {
            // ConsoleLogger는 내부적으로 동적 String을 쓰지만
            // SSO 로인해 힙 할당이 발생하지 않을 것으로 예상됨.
            // 만약 메모리 릭이 감지되면 이 부분을 점검해볼것.
            g_fallback.SetName("Core");
            g_bFallbackInited = true;
        }
    }

}   // namespace

void SetCoreLogger(
    Logger* _pLoggerOrNull)
{
    g_pLoggerOrNull = _pLoggerOrNull;
}

void LogCore(
    const eLogLevel  _level,
    const StringView _msg)
{
    if (g_pLoggerOrNull)
    {
        g_pLoggerOrNull->Log(_level, _msg);
    }
    else
    {
        InitFallbackIfNeed_();
        g_fallback.Log(_level, _msg);
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
    else
    {
        InitFallbackIfNeed_();
        g_fallback.VLog(_level, _msg, _args);
    }
}

}   // namespace jug