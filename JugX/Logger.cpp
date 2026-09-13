#include "Logger.h"

#include "EnumArray.h"
#include "EnumFlags.h"
#include "TimeStamp.h"
#include "Typedef.h"
#include <format>

namespace jug
{

void Logger::Log(
    const eLogLevel  _level,
    const StringView _msg)
{
    if (!m_levelFliterFlags.Has(_level))
    {
        return;
    }

    WritePrefix_(_level);
    WriteImpl(_level, _msg, true);
}

void Logger::VLog(
    const eLogLevel        _level,
    const StringView       _msg,
    const std::format_args _args)
{
    if (!m_levelFliterFlags.Has(_level))
    {
        return;
    }

    WritePrefix_(_level);
    VWriteImpl(_level, _msg, _args, true);
}

void Logger::SetName(
    const StringView _name)
{
    m_name = _name;
}

void Logger::SetFilter(
    const IndexedFlags<eLogLevel> _flags)
{
    m_levelFliterFlags = _flags;
}

void Logger::SetLogPattern(
    const Flags<eLogPattern> _flags)
{
    m_patternFlags = _flags;
}

void Logger::WritePrefix_(
    const eLogLevel _level)
{
    bool bAnyLogged = false;

    // time stamp
    if (m_patternFlags.HasAny({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
    {
        TimeStamp ts = TimeStamp::Now();

        if (m_patternFlags.HasAll({ eLogPattern::YearMonthDay, eLogPattern::HourMinSec }))
        {
            VWriteImpl(_level, "[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec), false);
        }
        else if (m_patternFlags.Has(eLogPattern::YearMonthDay))
        {
            VWriteImpl(_level, "[{:04d}-{:02d}-{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth), false);
        }
        else
        {
            VWriteImpl(_level, "[{:02d}:{:02d}:{:02d}]", std::make_format_args(ts.hour, ts.min, ts.sec), false);
        }

        bAnyLogged = true;
    }

    // log level
    if (m_patternFlags.Has(eLogPattern::Level))
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

        VWriteImpl(_level, "[{:<5}]", std::make_format_args(kNames[_level]), false);
        bAnyLogged = true;
    }

    // logger name
    if (m_patternFlags.Has(eLogPattern::Name))
    {
        if (bAnyLogged)
        {
            WriteImpl(_level, " ", false);
        }

        VWriteImpl(_level, "[{}]", std::make_format_args(m_name), false);
        bAnyLogged = true;
    }

    if (bAnyLogged)
    {
        WriteImpl(_level, ": ", false);
    }
}

}   // namespace jug