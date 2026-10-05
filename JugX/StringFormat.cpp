#include "pch.h"
#include "StringFormat.h"

#include <cstddef>
#include <format>
#include <iterator>

#include "CoreLogger.h"
#include "TypeTraits.h"

namespace jug
{
namespace
{
    template<CharT T>
    class Formatter
    {
    public:
        using value_type = T;

        explicit Formatter(const Span<T> _buffer)
            : m_buffer(_buffer)
        {
        }

        void push_back(const T _ch)
        {
            if (m_buffer.size() > m_result.written)
            {
                m_buffer[m_result.written++] = _ch;
            }
            ++m_result.totalSize;
        }

        [[nodiscard]] FormatResult GetResult() const
        {
            return m_result;
        }

    private:
        Span<T>      m_buffer = {};
        FormatResult m_result = {};
    };
}   // namespace

FormatResult VFormatTo(
    const Span<char>       _out,
    const StringView       _format,
    const std::format_args _args)
{
    Formatter<char> formatter { _out };

    try
    {
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        const FormatResult result = formatter.GetResult();
        if (result.written != result.totalSize)
        {
            JUG_CORE_LOG_WARN("Format m_result is truncated. Needed: {}, Written: {}", result.totalSize, result.written);
        }
        return result;
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

FormatResult VFormatTo(
    const Span<wchar_t>     _out,
    const WStringView       _format,
    const std::wformat_args _args)
{
    Formatter<wchar_t> formatter { _out };

    try
    {
        std::vformat_to(std::back_inserter(formatter), _format, _args);
        const FormatResult result = formatter.GetResult();
        if (result.written != result.totalSize)
        {
            JUG_CORE_LOG_WARN("Format m_result is truncated. Needed: {}, Written: {}", result.totalSize, result.written);
        }
        return result;
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

FormatResult ZVFormatTo(
    const Span<char>       _out,
    const StringView       _format,
    const std::format_args _args)
{
    JUG_ASSERT(!_out.empty(), "Output m_buffer is empty.\n");
    FormatResult ret = VFormatTo(_out.subspan(0, _out.size() - 1), _format, _args);
    _out[0]          = '\0';
    return ret;
}

FormatResult ZVFormatTo(
    Span<wchar_t>           _out,
    const WStringView       _format,
    const std::wformat_args _args)
{
    JUG_ASSERT(!_out.empty(), "Output m_buffer is empty.\n");
    FormatResult ret = VFormatTo(_out.subspan(0, _out.size() - 1), _format, _args);
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

void AppendVFormat(
    String&                _out,
    const StringView       _format,
    const std::format_args _args)
{
    const size_t old = _out.size();

    try
    {
        std::vformat_to(std::back_inserter(_out), _format, _args);
    }
    catch (const std::format_error& e)   // NOLINT
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Format error occurred while appending to string. Error: {}", e.what());
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
    catch (const std::format_error& e)   // NOLINT
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Format error occurred while appending to string. Error: {}", e.what());
    }
}

}   // namespace jug