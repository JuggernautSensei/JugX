#include "Logger.h"

#include "EnumArray.h"
#include "TimeStamp.h"

namespace jug
{

void Logger::Log(
    const eLogLevel  _level,
    const StringView _msg)
{
    if (!m_filter.Has(_level))
    {
        return;
    }

    WritePrefix_(_level);
    WriteImpl(_level, _msg, true);
}

void Logger::LogV(
    const eLogLevel        _level,
    const StringView       _msg,
    const std::format_args _args)
{
    if (!m_filter.Has(_level))
    {
        return;
    }

    WritePrefix_(_level);
    WriteImpl(_level, _msg, _args, true);
}

void Logger::SetName(
    const StringView _name)
{
    m_name = _name;
}

void Logger::SetLogPattern(
    const Flags<eLogPattern> _pattern)
{
    m_pattern = _pattern;
}

void Logger::SetFilter(
    const IndexedFlags<eLogLevel> _filter)
{
    m_filter = _filter;
}

void Logger::WritePrefix_(
    const eLogLevel _level)
{
    bool bAnyLogged = false;

    // time stamp
    if (m_pattern.HasAny({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
    {
        TimeStamp ts = TimeStamp::Now();

        if (m_pattern.HasAll({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
        {
            WriteImpl(_level, "[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec), false);
        }
        else if (m_pattern.Has(eLogPattern::YearMonthDay))
        {
            WriteImpl(_level, "[{:04d}-{:02d}-{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth), false);
        }
        else
        {
            WriteImpl(_level, "[{:02d}:{:02d}:{:02d}]", std::make_format_args(ts.hour, ts.min, ts.sec), false);
        }

        bAnyLogged = true;
    }

    // log level
    if (m_pattern.Has(eLogPattern::Level))
    {
        if (bAnyLogged)
        {
            WriteImpl(_level, " ", false);
        }

        constexpr ENUM_ARRAY<eLogLevel, StringView> kNames = {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"
        };

        WriteImpl(_level, "[{:<5}]", std::make_format_args(kNames[_level]), false);
        bAnyLogged = true;
    }

    // logger name
    if (m_pattern.Has(eLogPattern::Name))
    {
        if (bAnyLogged)
        {
            WriteImpl(_level, " ", false);
        }

        WriteImpl(_level, "[{}]", std::make_format_args(m_name), false);
        bAnyLogged = true;
    }

    if (bAnyLogged)
    {
        WriteImpl(_level, ": ", false);
    }
}

}   // namespace jug