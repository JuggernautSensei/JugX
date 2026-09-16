#include "pch.h"
#include "StringFormat.h"

#include <cstddef>
#include <format>
#include <iterator>

#include "CoreLogger.h"

namespace jug
{
namespace
{
    template<CharT T>
    struct Formatter
    {
        using value_type = T;

        void push_back(const T _ch)
        {
            if (buffer.size() > result.written)
            {
                buffer[result.written++] = _ch;
            }
            ++result.totalSize;
        }

        Span<T>      buffer = {};
        FormatResult result = {};
    };
}   // namespace

FormatResult VFormat(
    const Span<char>       _out,
    const StringView       _format,
    const std::format_args _args)
{
    Formatter<char> formatter = { _out };

    try
    {
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        return formatter.result;
    }
    catch (const std::format_error& e)
    {
        JUG_CORE_LOG_ERROR("Format error occurred. Error: {}", e.what());
        if (!_out.empty())
        {
            _out[0] = '\0';
        }
        return { 0, 0 };
    }
}

FormatResult VFormat(
    const Span<wchar_t>     _out,
    const WStringView       _format,
    const std::wformat_args _args)
{
    Formatter<wchar_t> formatter = { _out };

    try
    {
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        return formatter.result;
    }
    catch (const std::format_error& e)
    {
        JUG_CORE_LOG_ERROR("Format error occurred. Error: {}", e.what());
        if (!_out.empty())
        {
            _out[0] = L'\0';
        }
        return { 0, 0 };
    }
}

FormatResult ZVFormat(
    const Span<char>       _out,
    const StringView       _format,
    const std::format_args _args)
{
    JUG_ASSERT(!_out.empty(), "Output buffer is empty.\n");
    FormatResult ret = VFormat(_out.subspan(0, _out.size() - 1), _format, _args);
    _out[0]          = '\0';
    return ret;
}

FormatResult ZVFormat(
    Span<wchar_t>           _out,
    const WStringView       _format,
    const std::wformat_args _args)
{
    JUG_ASSERT(!_out.empty(), "Output buffer is empty.\n");
    FormatResult ret = VFormat(_out.subspan(0, _out.size() - 1), _format, _args);
    _out[0]          = L'\0';
    return ret;
}

String VFormat(
    const StringView       _format,
    const std::format_args _args)
{
    String str = {};
    str.reserve(_format.size());

    try
    {
        std::vformat_to(std::back_inserter(str), _format, _args);
        return str;
    }
    catch (const std::format_error& e)
    {
        JUG_CORE_LOG_ERROR("Format error occurred. Error: {}", e.what());
        return {};
    }
}

WString VFormat(
    const WStringView       _format,
    const std::wformat_args _args)
{
    WString str = {};
    str.reserve(_format.size());

    try
    {
        std::vformat_to(std::back_inserter(str), _format, _args);
        return str;
    }
    catch (const std::format_error& e)
    {
        JUG_CORE_LOG_ERROR("Format error occurred. Error: {}", e.what());
        return {};
    }
}

void AppendVFormat(String& _out, const StringView _format, const std::format_args _args)
{
    const size_t old = _out.size();

    try
    {
        std::vformat_to(std::back_inserter(_out), _format, _args);
    }
    catch (const std::format_error&)   // NOLINT
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Format error occurred while appending to string.");
    }
}

void AppendVFormat(
    WString&                _out,
    const WStringView       _format,
    const std::wformat_args _args)
{
    const size_t old = _out.size();

    try
    {
        std::vformat_to(std::back_inserter(_out), _format, _args);
    }
    catch (const std::format_error&)   // NOLINT
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Format error occurred while appending to string.");
    }
}

}   // namespace jug