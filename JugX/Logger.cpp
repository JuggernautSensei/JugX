#include "pch.h"
#include "Logger.h"

#include <format>

#include "EnumArray.h"
#include "EnumFlags.h"
#include "TimeStamp.h"
#include "Typedef.h"

namespace jug
{

namespace
{
    constexpr Flags<eLogPattern>                kLogPatternFlags = { eLogPattern::YearMonthDay, eLogPattern::HourMinSec };
    constexpr ENUM_ARRAY<eLogLevel, StringView> kLogLevelNames   = { "TRACE", "INFO", "WARN", "ERROR", "FATAL" };
}   // namespace

void Logger::Log(
    const eLogLevel  _level,
    const StringView _msg)
{
    if (!m_levelFliterFlags.Has(_level))
    {
        return;
    }

    LogPrefix_(_level);
    LogImpl(_level, _msg, true);
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

    LogPrefix_(_level);
    VLogImpl(_level, _msg, _args, true);
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

void Logger::SetPattern(
    const Flags<eLogPattern> _flags)
{
    m_patternFlags = _flags;
}

void Logger::LogPrefix_(
    const eLogLevel _level)
{
    bool bAnyLogged = false;

    // time stamp

    if (m_patternFlags.HasAny(kLogPatternFlags))
    {
        const TimeStamp ts = TimeStamp::Now();
        if (m_patternFlags.HasAll(kLogPatternFlags))
        {
            VLogImpl(_level, "[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec), false);
        }
        else if (m_patternFlags.Has(eLogPattern::YearMonthDay))
        {
            VLogImpl(_level, "[{:04d}-{:02d}-{:02d}]", std::make_format_args(ts.year, ts.month, ts.dayOfTheMonth), false);
        }
        else
        {
            VLogImpl(_level, "[{:02d}:{:02d}:{:02d}]", std::make_format_args(ts.hour, ts.min, ts.sec), false);
        }

        bAnyLogged = true;
    }

    // log level
    if (m_patternFlags & eLogPattern::Level)
    {
        if (bAnyLogged)
        {
            LogImpl(_level, " ", false);
        }

        VLogImpl(_level, "[{:<5}]", std::make_format_args(kLogLevelNames[_level]), false);
        bAnyLogged = true;
    }

    // logger name
    if (m_patternFlags & eLogPattern::Name)
    {
        if (bAnyLogged)
        {
            LogImpl(_level, " ", false);
        }

        VLogImpl(_level, "[{}]", std::make_format_args(m_name), false);
        bAnyLogged = true;
    }

    if (bAnyLogged)
    {
        LogImpl(_level, ": ", false);
    }
}

}   // namespace jug