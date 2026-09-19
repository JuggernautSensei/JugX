#include "pch.h"
#include "ConsoleLogger.h"

#include <format>
#include <iostream>
#include <iterator>

#include "EnumArray.h"
#include "Logger.h"
#include "Typedef.h"

namespace jug
{

namespace
{
    constexpr ENUM_ARRAY<eLogLevel, StringView> kColors = {
        "\033[0m",          // TRACE 흰색
        "\033[38;5;46m",    // INFO  초록
        "\033[38;5;226m",   // WARN  노랑
        "\033[38;5;9m",     // ERROR 빨강
        "\033[38;5;201m"    // FATAL 마젠타
    };

    constexpr StringView kResetColor = "\033[0m";
}   // namespace

void ConsoleLogger::LogImpl(
    const eLogLevel        _level,
    const StringView _msg,
    const bool             _bEndLog)
{
    std::cout << kColors[_level] << _msg << kResetColor;
    if (_bEndLog)
    {
        std::cout << '\n';
    }
}

void ConsoleLogger::VLogImpl(
    const eLogLevel        _level,
    const StringView _msg,
    const std::format_args _args,
    const bool             _bEndLog)
{
    std::cout << kColors[_level];
    std::vformat_to(std::ostream_iterator<char>(std::cout), _msg, _args);
    if (_bEndLog)
    {
        std::cout << kResetColor << '\n';
    }
}

void ConsoleLogger::Flush()
{
    std::cout.flush();
}

}   // namespace jug