#pragma once
#include <format>

#include "EnumFlags.h"
#include "TimeStamp.h"
#include "Typedef.h"

namespace jug
{

enum class eLogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

enum class eLogPattern : uint32_t
{
    None,
    YearMonthDay,
    HourMinSec,
    Level,
    Name
};

// [YYYY-MM-DD HH:MM:SS] [LEVEL] [LoggerName]: Message

class Logger
{
    friend class GroupLogger;

public:
    Logger()                         = default;
    Logger(const Logger&)            = default;
    Logger& operator=(const Logger&) = default;
    Logger(Logger&&)                 = default;
    Logger& operator=(Logger&&)      = default;

    virtual ~Logger() = default;

    void Log(eLogLevel _level, StringView _msg);
    void LogV(eLogLevel _level, StringView _msg, std::format_args _args);

    template<typename... TArgs>
    void Log(
        const eLogLevel              _level,
        std::format_string<TArgs...> _format,
        TArgs&&... _args)
    {
        LogV(_level, _format.get(), std::make_format_args(_args...));
    }

    virtual void Flush() = 0;

    void SetName(StringView _name);
    void SetLogPattern(Flags<eLogPattern> _pattern);
    void SetFilter(IndexedFlags<eLogLevel> _filter);

private:
    virtual void WriteImpl(eLogLevel _level, StringView _msg, bool _bEnd)                         = 0;
    virtual void WriteImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEnd) = 0;   // for formatted message

    void WritePrefix_(eLogLevel _level);

    String                  m_name    = "Logger";
    Flags<eLogPattern>      m_pattern = kAllFlag;
    IndexedFlags<eLogLevel> m_filter  = kAllFlag;
};

}   // namespace jug