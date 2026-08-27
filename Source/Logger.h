#pragma once
#include <string_view>
#include <format>

#include "EnumFlags.h"
#include "TimeStamp.h"

namespace jug
{

enum class eLogLevel
{
    Info,
    Trace,
    Debug,
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

    void Log(eLogLevel _level, std::string_view _message);
    void Format(eLogLevel _level, std::string_view _message, std::format_args _args);

    template<typename... TArgs>
    void Format(
        const eLogLevel              _level,
        std::format_string<TArgs...> _format,
        TArgs&&... _args)
    {
        if (!m_filter.Has(_level))
        {
            return;
        }

        LogPattern(_level);
        FormatImpl(_level, _format.get(), std::make_format_args(_args...), true);
    }

    virtual void Flush() = 0;

    void SetLoggerName(std::string_view _name);
    void SetLogPattern(Flags<eLogPattern> _pattern);
    void SetFilter(IndexedFlags<eLogLevel> _filter);

protected:
    virtual void LogImpl(eLogLevel _level, std::string_view _message, bool _bNewLine)                            = 0;
    virtual void FormatImpl(eLogLevel _level, std::string_view _message, std::format_args _args, bool _bNewLine) = 0;

private:
    void LogPattern(eLogLevel _level);

    std::string             m_name    = "Logger";
    Flags<eLogPattern>      m_pattern = eLogPattern::None;
    IndexedFlags<eLogLevel> m_filter  = kAllFlag;
};

}   // namespace jug