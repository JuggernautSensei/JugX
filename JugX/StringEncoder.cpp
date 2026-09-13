#include "StringEncoder.h"

#include "Assertion.h"
#include "CoreLogger.h"

#include "Typedef.h"
#include "Vendor/utfcpp/source/utf8/checked.h"
#include <iterator>
#include <cstddef>

namespace jug
{

namespace
{
    template<typename T>
    struct Encoder
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

        Span<T>      buffer = {};
        EncodeResult result = {};
    };
}   // namespace

EncodeResult ToUtf8(
    Span<char>       _outBuffer,
    const StringView _utf16,
    const bool       _bNullTerminated)
{
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    Encoder<char> encoder = { _outBuffer };

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(encoder));
        if (_bNullTerminated)
        {
            _outBuffer[encoder.result.written] = '\0';
        }
        return encoder.result;
    }
    catch (const utf8::exception& e)
    {
        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
        if (!_outBuffer.empty())
        {
            _outBuffer[0] = '\0';
        }
        return {};
    }
}

EncodeResult ToUtf16(
    Span<wchar_t>    _outBuffer,
    const StringView _utf8,
    const bool       _bNullTerminated)
{
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    Encoder<wchar_t> encoder = { _outBuffer };

    try
    {
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(encoder));
        if (_bNullTerminated)
        {
            _outBuffer[encoder.result.written] = L'\0';
        }
        return encoder.result;
    }
    catch (const utf8::exception& e)
    {
        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
        if (!_outBuffer.empty())
        {
            _outBuffer[0] = L'\0';
        }
        return {};
    }
}

String ToUtf8(
    const WStringView _utf16)
{
    Encoder<char> encoder = {};

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(encoder));
        if (encoder.result.totalSize == 0)
        {
            return {};
        }

        String str = {};
        str.reserve(encoder.result.totalSize);
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(str));
        return str;
    }
    catch (const utf8::exception& e)
    {
        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
        return {};
    }
}

WString ToUtf16(
    const StringView _utf8)
{
    Encoder<wchar_t> encoder = {};

    try
    {
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(encoder));
        if (encoder.result.totalSize == 0)
        {
            return {};
        }

        WString str = {};
        str.reserve(encoder.result.totalSize);
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(str));
        return str;
    }
    catch (const utf8::exception& e)
    {
        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
        return {};
    }
}

void AppendUtf16(
    String&           _out,
    const WStringView _utf16)
{
    const size_t old = _out.size();

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(_out));
    }
    catch (const utf8::exception& e)
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
    }
}

void AppendUtf8(
    WString&         _out,
    const StringView _utf8)
{
    const size_t old = _out.size();

    try
    {
        utf8 ::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(_out));
    }
    catch (const utf8::exception& e)
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
    }
}

}   // namespace jug
