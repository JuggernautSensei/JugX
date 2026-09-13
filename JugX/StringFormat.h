#pragma once

#include <format>
#include <concepts>

#include "Config.h"
#include "CoreLogger.h"

namespace jug
{

struct FormatResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이. 널 문자는 포함되지 않음.
};

// ===========================================
//  Format Span
// ===========================================

FormatResult VFormat(Span<char> _out, StringView _format, std::format_args _args);
FormatResult VFormat(Span<wchar_t> _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
FormatResult Format(
    const Span<char>                              _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    return VFormat(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
FormatResult WFormat(
    const Span<wchar_t>                              _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return VFormat(_out, _format.get(), std::make_wformat_args(_args...));
}

// ===========================================
//  Format Span Null Terminate
// ===========================================

FormatResult ZVFormat(Span<char> _out, StringView _format, std::format_args _args);
FormatResult ZVFormat(Span<wchar_t> _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
[[nodiscard]] FormatResult ZFormat(
    Span<char>                                    _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    return ZVFormat(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
[[nodiscard]] FormatResult ZWFormat(
    Span<wchar_t>                                    _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return ZVFormat(_out, _format.get(), std::make_wformat_args(_args...));
}

// ===========================================
//  Format String
// ===========================================

[[nodiscard]] String  VFormat(StringView _format, std::format_args _args);
[[nodiscard]] WString VFormat(WStringView _format, std::wformat_args _args);

template<typename... Args>
[[nodiscard]] String Format(
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    return VFormat(_format.get(), std::make_format_args(_args...));
}

template<typename... Args>
[[nodiscard]] WString WFormat(
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return VFormat(_format.get(), std::make_wformat_args(_args...));
}

// ===========================================
//  Append To String (char)
// ===========================================

void AppendVFormat(String& _out, StringView _format, std::format_args _args);
void AppendVFormat(WString& _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
void AppendFormat(
    String&                                       _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    AppendVFormat(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
void AppendWFormat(
    WString&                                         _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    AppendVFormat(_out, _format.get(), std::make_wformat_args(_args...));
}

}   // namespace jug