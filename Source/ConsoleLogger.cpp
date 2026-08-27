#include "ConsoleLogger.h"

#include <iostream>
#include <iterator>

#include "EnumArray.h"

namespace jug
{

namespace
{
    constexpr ENUM_ARRAY<eLogLevel, std::string_view> kColors = {
        "\033[0;37m",   // INFO: White
        "\033[0;36m",   // TRACE: Cyan
        "\033[0;32m",   // DEBUG: Green
        "\033[0;33m",   // WARN: Yellow
        "\033[0;31m",   // ERROR: Red
        "\033[0;35m"    // FATAL: Magenta
    };

    constexpr std::string_view kResetColor = "\033[0m";
}   // namespace

void ConsoleLogger::LogImpl(
    const eLogLevel        _level,
    const std::string_view _message,
    const bool             _bNewLine)
{
    std::cout << kColors[_level] << _message << kResetColor;
    if (_bNewLine)
    {
        std::cout << '\n';
    }
}

void ConsoleLogger::FormatImpl(
    const eLogLevel        _level,
    const std::string_view _message,
    const std::format_args _args,
    const bool             _bNewLine)
{
    std::cout << kColors[_level];
    std::vformat_to(std::ostream_iterator<char>(std::cout), _message, _args);
    if (_bNewLine)
    {
        std::cout << kResetColor << '\n';
    }
}

void ConsoleLogger::Flush()
{
    std::cout.flush();
}

}   // namespace jug