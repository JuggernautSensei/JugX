#pragma once
#include <format>

#include "Config.h"
#include "CoreLogger.h"

namespace jug
{

struct FormatResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이. 널 문자는 포함되지 않음.
};

// 버퍼 용량이 부족하면 잘린 문자열이 생성됨.
// 포맷 실패시 FormatResult { 0, 0 } 반환. _outBuffer는 빈 상태가 됨.

namespace string_formatter_detail
{
    template<CharT T>
    struct Formatter
    {
        using value_type = T;

        void push_back(
            const T _ch)
        {
            if (buffer.size() > result.written)
            {
                buffer[result.written++] = _ch;
            }
            ++result.totalSize;
        }

        Span<T>  buffer = {};
        FormatResult result = {};
    };

    template<CharT T, bool kbNullTerminated>
    [[nodiscard]] FormatResult VFormat(
        Span<T>            _outBuffer,
        BasicStringView<T> _format,
        const std::format_args _args)
    {
        if constexpr (kbNullTerminated)
        {
            JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");   // 널 문자를 보장해야하기 때문에 최소 1 이상의 크기가 필요함
            _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
        }

        try
        {
            Formatter<T> formatter = { _outBuffer };
            std::vformat_to(std::back_inserter(formatter), _format, _args);

            if constexpr (kbNullTerminated)
            {
                _outBuffer[formatter.result.written] = T { 0 };   // null-terminate
            }

            return formatter.result;
        }
        catch (const std::format_error& e)
        {
            // invalidate
            if (!_outBuffer.empty())
            {
                _outBuffer[0] = T { 0 };
            }

            JUG_CORE_LOG_ERROR("Format error occurred. Format: '{}', Error: {}", _format, e.what());
            return { 0, 0 };
        }
    }
}   // namespace string_formatter_detail

template<CharT T>
[[nodiscard]] FormatResult VFormat(
    Span<T>            _outBuffer,
    BasicStringView<T> _format,
    const std::format_args _args)
{
    return string_formatter_detail::VFormat<T, false>(_outBuffer, _format, _args);
}

template<CharT T, typename... Args>
[[nodiscard]] FormatResult Format(
    Span<T>                               _outBuffer,
    std::basic_format_string<T, Args...> _format,
    Args&&... _args)
{
    return string_formatter_detail::VFormat<T, false>(_outBuffer, _format.get(), std::make_format_args(_args...));
}

template<CharT T>
[[nodiscard]] FormatResult ZVFormat(
    Span<T>            _outBuffer,
    BasicStringView<T> _format,
    const std::format_args _args)
{
    return string_formatter_detail::VFormat<T, true>(_outBuffer, _format, _args);
}

template<CharT T, typename... Args>
[[nodiscard]] FormatResult ZFormat(
    Span<T>                               _outBuffer,
    std::basic_format_string<T, Args...> _format,
    Args&&... _args)
{
    return string_formatter_detail::VFormat<T, true>(_outBuffer, _format.get(), std::make_format_args(_args...));
}

template<CharT T, typename... Args>
void AppendVFormat(
    BasicString<T>&    _outString,
    BasicStringView<T> _format,
    std::format_args       _args)
{
    size_t oldSize = _outString.size();

    try
    {
        std::vformat_to(std::back_inserter(_outString), _format, _args);
    }
    catch (const std::format_error& e)
    {
        _outString.erase(_outString.begin() + static_cast<ptrdiff_t>(oldSize), _outString.end());
        JUG_CORE_LOG_ERROR("Format error occurred. Format: '{}', Error: {}", _format, e.what());
    }
}

template<CharT T, typename... Args>
void AppendFormat(
    BasicString<T>&                       _outString,
    std::basic_format_string<T, Args...> _format,
    Args&&... _args)
{
    AppendVFormat(_outString, _format.get(), std::make_format_args(_args...));
}

}   // namespace jug