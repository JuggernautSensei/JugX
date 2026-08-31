#pragma once
#include <format>

#include "Config.h"

namespace jug
{

struct FormatResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이
    // 버퍼 용량이 부족하면 잘린 문자열이 생성됨
};

namespace string_formatter_detail
{
    template<typename TChar>
    struct Formatter
    {
        using value_type = TChar;

        void push_back(
            const TChar _ch)
        {
            if (buffer.size() > result.written)
            {
                buffer[result.written++] = _ch;
            }
            ++result.totalSize;
        }

        Span<TChar>  buffer = {};
        FormatResult result = {};
    };
}   // namespace string_formatter_detail

template<typename TChar>
[[nodiscard]] FormatResult FormatV(
    Span<TChar>                   _outBuffer,
    std::basic_string_view<TChar> _format,
    std::format_args              _args)
{
    try
    {
        string_formatter_detail::Formatter<TChar> formatter = { _outBuffer };
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        return formatter.result;
    }
    catch (const std::format_error&)
    {
        JUG_ASSERT(false, "Format error occurred.\n");
        return { 0, 0 };
    }
}

template<typename TChar, typename... TArgs>
[[nodiscard]] FormatResult Format(
    Span<TChar>                               _outBuffer,
    std::basic_format_string<TChar, TArgs...> _format,
    TArgs&&... _args)
{
    return FormatV(_outBuffer, _format.get(), std::make_format_args(_args...));
}

template<typename TChar>
[[nodiscard]] FormatResult FormatVZ(
    Span<TChar>                   _outBuffer,
    std::basic_string_view<TChar> _format,
    std::format_args              _args)
{
    JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");   // 널 문자를 보장해야하기 때문에 최소 1 이상의 크기가 필요함

    try
    {
        string_formatter_detail::Formatter<TChar> formatter = { _outBuffer.subspan(0, _outBuffer.size() - 1) };
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        _outBuffer[formatter.result.written] = TChar { 0 };   // null-terminate
        return formatter.result;
    }
    catch (const std::format_error&)
    {
        JUG_ASSERT(false, "Format error occurred.\n");
        return { 0, 0 };
    }
}

template<typename TChar, typename... TArgs>
[[nodiscard]] FormatResult FormatZ(
    Span<TChar>                               _outBuffer,
    std::basic_format_string<TChar, TArgs...> _format,
    TArgs&&... _args)
{
    return FormatVZ(_outBuffer, _format.get(), std::make_format_args(_args...));
}

template<typename TChar>
void AppendFormatV(
    std::basic_string<TChar>&     _outBuffer,
    std::basic_string_view<TChar> _format,
    std::format_args              _args)
{
    try
    {
        std::vformat_to(std::back_inserter(_outBuffer), _format, _args);
    }
    catch (const std::format_error&)
    {
        JUG_ASSERT(false, "Format error occurred.\n");
    }
}

template<typename TChar, typename... TArgs>
void AppendFormat(
    std::basic_string<TChar>&                 _outBuffer,
    std::basic_format_string<TChar, TArgs...> _format,
    TArgs&&... _args)
{
    AppendFormatV(_outBuffer, _format.get(), std::make_format_args(_args...));
}

}   // namespace jug