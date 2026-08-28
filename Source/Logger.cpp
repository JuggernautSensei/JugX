#include "Logger.h"

#include "EnumArray.h"
#include "TimeStamp.h"

namespace jug
{

void Logger::Log(
    const eLogLevel        _level,
    const std::string_view _msg)
{
    if (!m_filter.Has(_level))
    {
        return;
    }

    LogPattern(_level);
    LogImpl(_level, _msg, true);
}

void Logger::Log(
    const eLogLevel        _level,
    const std::string_view _msg,
    const std::format_args _args)
{
    if (!m_filter.Has(_level))
    {
        return;
    }

    LogPattern(_level);
    VFormatImpl(_level, _msg, _args, true);
}

void Logger::SetName(
    const std::string_view _name)
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

void Logger::LogPattern(
    const eLogLevel _level)
{
    bool bAnyLogged = false;

    // time stamp
    if (m_pattern.HasAny({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
    {
        TimeStamp ts = TimeStamp::Now();

        if (m_pattern.HasAll({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
        {
            VFormatImpl(_level, "[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec), false);
        }
        else if (m_pattern.Has(eLogPattern::YearMonthDay))
        {
            VFormatImpl(_level, "[{:04d}-{:02d}-{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth), false);
        }
        else
        {
            VFormatImpl(_level, "[{:02d}:{:02d}:{:02d}]", std::make_format_args(ts.hour, ts.min, ts.sec), false);
        }

        bAnyLogged = true;
    }

    // log level
    if (m_pattern.Has(eLogPattern::Level))
    {
        if (bAnyLogged)
        {
            LogImpl(_level, " ", false);
        }

        constexpr ENUM_ARRAY<eLogLevel, std::string_view> kNames = {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"
        };

        VFormatImpl(_level, "[{:<5}]", std::make_format_args(kNames[_level]), false);
        bAnyLogged = true;
    }

    // logger name
    if (m_pattern.Has(eLogPattern::Name))
    {
        if (bAnyLogged)
        {
            LogImpl(_level, " ", false);
        }

        VFormatImpl(_level, "[{}]", std::make_format_args(m_name), false);
        bAnyLogged = true;
    }

    if (bAnyLogged)
    {
        LogImpl(_level, ": ", false);
    }
}

}   // namespace jug