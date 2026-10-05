#include "pch.h"
#include "StringEncoder.h"

#include <cstddef>
#include <iterator>
#include <utf8cpp/utf8.h>

#include "CoreLogger.h"

namespace jug
{

namespace
{
    template<typename T>
    class Encoder
    {
    public:
        using value_type = T;

        explicit Encoder(
            Span<T> _buffer)
            : m_buffer(_buffer)
        {
        }

        void push_back(
            const T _ch)
        {
            if (m_buffer.size() > m_result.written)
            {
                m_buffer[m_result.written++] = _ch;
            }
            ++m_result.totalSize;
        }

        [[nodiscard]] EncodeResult GetResult() const
        {
            return m_result;
        }

    private:
        Span<T>      m_buffer = {};
        EncodeResult m_result = {};
    };
}   // namespace

EncodeResult EncodeUtf8To(
    Span<char>        _outBuffer,
    const WStringView _utf16,
    const bool        _bNullTerminated)
{
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output m_buffer is empty.\n");
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    Encoder<char> encoder { _outBuffer };

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(encoder));
        const EncodeResult result = encoder.GetResult();
        if (result.written != result.totalSize)
        {
            JUG_CORE_LOG_WARN("Format m_result is truncated. Needed: {}, Written: {}", result.totalSize, result.written);
        }
        if (_bNullTerminated)
        {
            _outBuffer[result.written] = '\0';
        }
        return result;
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

EncodeResult EncodeUtf16To(
    Span<wchar_t>    _outBuffer,
    const StringView _utf8,
    const bool       _bNullTerminated)
{
    if (_bNullTerminated)
    {
        JUG_ASSERT(!_outBuffer.empty(), "Output m_buffer is empty.\n");
        _outBuffer = _outBuffer.subspan(0, _outBuffer.size() - 1);
    }

    Encoder<wchar_t> encoder { _outBuffer };

    try
    {
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(encoder));
        const EncodeResult result = encoder.GetResult();
        if (result.written != result.totalSize)
        {
            JUG_CORE_LOG_WARN("Format m_result is truncated. Needed: {}, Written: {}", result.totalSize, result.written);
        }
        if (_bNullTerminated)
        {
            _outBuffer[result.written] = L'\0';
        }
        return result;
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

String EncodeUtf8(
    const WStringView _utf16)
{
    Encoder<char> encoder { {} };

    try
    {
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(encoder));
        const EncodeResult result = encoder.GetResult();
        if (result.totalSize == 0)
        {
            return {};
        }

        String str = {};
        str.reserve(result.totalSize);
        utf8::utf16to8(_utf16.begin(), _utf16.end(), std::back_inserter(str));
        return str;
    }
    catch (const utf8::exception& e)
    {
        JUG_CORE_LOG_ERROR("Failed to convert UTF-16 to UTF-8. Error: {}", e.what());
        return {};
    }
}

WString EncodeUtf16(
    const StringView _utf8)
{
    Encoder<char> encoder { {} };

    try
    {
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(encoder));
        const EncodeResult result = encoder.GetResult();
        if (result.totalSize == 0)
        {
            return {};
        }

        WString str = {};
        str.reserve(result.totalSize);
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
        utf8::utf8to16(_utf8.begin(), _utf8.end(), std::back_inserter(_out));
    }
    catch (const utf8::exception& e)
    {
        _out.erase(_out.begin() + static_cast<ptrdiff_t>(old), _out.end());
        JUG_CORE_LOG_ERROR("Failed to convert UTF-8 to UTF-16. Error: {}", e.what());
    }
}

}   // namespace jug
