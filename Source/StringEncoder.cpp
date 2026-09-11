#include "StringEncoder.h"

#include "Config.h"
#include "CoreLogger.h"

#include "Vendor/utfcpp/source/utf8.h"

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

        Span<T>  buffer = {};
        EncodeResult result = {};
    };
}   // namespace

EncodeResult ToUtf8(
    Span<char>       _outBuffer,
    const StringView _utf16,
    const bool       _bNullTerminated)
{
    // guarantee null-termination if requested
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");   // 널 문자를 보장해야하기 때문에 최소 1 이상의 크기가 필요함
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    try
    {
        // encode
        Encoder<char> encoder { _outBuffer };
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(encoder));

        // write null-terminator if need
        if (_bNullTerminated)
        {
            _outBuffer[encoder.result.written] = '\0';
        }
        return encoder.result;
    }
    catch (const utf8::exception& e)
    {
        // invalidate
        if (!_outBuffer.empty())
        {
            _outBuffer[0] = '\0';
        }

        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
        return {};
    }
}

EncodeResult ToUtf16(
    Span<wchar_t>    _outBuffer,
    const StringView _utf8,
    const bool       _bNullTerminated)
{
    // guarantee null-termination if requested
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output buffer is empty.\n");   // 널 문자를 보장해야하기 때문에 최소 1 이상의 크기가 필요함
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    try
    {
        // encode
        Encoder<wchar_t> encoder { _outBuffer };
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(encoder));

        // write null-terminator if need
        if (_bNullTerminated)
        {
            _outBuffer[encoder.result.written] = L'\0';
        }
        return encoder.result;
    }
    catch (const utf8::exception& e)
    {
        // invalidate
        if (!_outBuffer.empty())
        {
            _outBuffer[0] = L'\0';
        }

        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
        return {};
    }
}

String ToUtf8(
    const StringView _utf16)
{
    try
    {
        Encoder<char> encoder = {};
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
    try
    {
        Encoder<wchar_t> encoder = {};
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
    String& _outStr,
    WStringView  _utf16)
{
    const size_t oldSize = _outStr.size();

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(_outStr));
    }
    catch (const utf8::exception& e)
    {
        // invalidate
        _outStr.erase(_outStr.begin() + static_cast<ptrdiff_t>(oldSize), _outStr.end());
        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
    }
}

void AppendUtf8(
    WString&    _outStr,
    const StringView _utf8)
{
    const size_t oldSize = _outStr.size();

    try
    {
        utf8 ::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(_outStr));
    }
    catch (const utf8::exception& e)
    {
        // invalidate
        _outStr.erase(_outStr.begin() + static_cast<ptrdiff_t>(oldSize), _outStr.end());
        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
    }
}

}   // namespace jug
