#pragma once

namespace jug
{

struct FormatResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이. 널 문자는 포함되지 않음.
};

// 버퍼 길이 부족시 메시지는 잘림.

// ===========================================
//  Format Span
// ===========================================

FormatResult VFormatTo(Span<char> _out, StringView _format, std::format_args _args);
FormatResult VFormatTo(Span<wchar_t> _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
FormatResult FormatTo(
    const Span<char>                  _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    return VFormatTo(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
FormatResult FormatTo(
    const Span<wchar_t>                _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return VFormatTo(_out, _format.get(), std::make_wformat_args(_args...));
}

// ===========================================
//  Format Span Null Terminate
// ===========================================

FormatResult ZVFormatTo(Span<char> _out, StringView _format, std::format_args _args);
FormatResult ZVFormatTo(Span<wchar_t> _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
FormatResult ZFormatTo(
    const Span<char>                  _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    return ZVFormatTo(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
FormatResult ZFormatTo(
    const Span<wchar_t>                _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return ZVFormatTo(_out, _format.get(), std::make_wformat_args(_args...));
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
[[nodiscard]] WString Format(
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    return VFormat(_format.get(), std::make_wformat_args(_args...));
}

// ===========================================
//  Append To String
// ===========================================

void AppendVFormat(String& _out, StringView _format, std::format_args _args);
void AppendVFormat(WString& _out, WStringView _format, std::wformat_args _args);

template<typename... Args>
void AppendFormat(
    String&                           _out,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    AppendVFormat(_out, _format.get(), std::make_format_args(_args...));
}

template<typename... Args>
void AppendFormat(
    WString&                           _out,
    const std::wformat_string<Args...> _format,
    Args&&... _args)
{
    AppendVFormat(_out, _format.get(), std::make_wformat_args(_args...));
}

}   // namespace jug