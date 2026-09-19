#pragma once
#include "EnumFlags.h"
#include "TimeStamp.h"

namespace jug
{

enum class eLogLevel
{
    Trace,
    Info,
    Warn,
    Error,
    Fatal
};

// [YYYY-MM-DD HH:MM:SS] [LEVEL] [LoggerName]: Message
enum class eLogPattern : uint32_t
{
    None         = 0,
    YearMonthDay = 1 << 0,
    HourMinSec   = 1 << 1,
    Level        = 1 << 2,
    Name         = 1 << 3
};

class Logger
{
    JUG_CLASS(Logger, DEFAULT_COPY, DEFAULT_MOVE)
    friend class GroupLogger;

public:
    Logger()          = default;
    virtual ~Logger() = default;

    // ===========================================
    //  Log
    // ===========================================

    void Log(eLogLevel _level, StringView _msg);
    void VLog(eLogLevel _level, StringView _msg, std::format_args _args);

    template<typename... Args>
    void Log(
        const eLogLevel                   _level,
        const std::format_string<Args...> _format,
        Args&&... _args)
    {
        VLog(_level, _format.get(), std::make_format_args(_args...));
    }

    virtual void Flush() = 0;

    void SetName(StringView _name);
    void SetFilter(IndexedFlags<eLogLevel> _flags);
    void SetPattern(Flags<eLogPattern> _flags);

private:
    virtual void LogImpl(eLogLevel _level, StringView _msg, bool _bEndOfLog)                          = 0;
    virtual void VLogImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndOfLog) = 0;   // for formatted message

    void LogPrefix_(eLogLevel _level);

    String                  m_name             = "Logger";
    Flags<eLogPattern>      m_patternFlags     = kAllFlag;
    IndexedFlags<eLogLevel> m_levelFliterFlags = kAllFlag;
};

}   // namespace jug