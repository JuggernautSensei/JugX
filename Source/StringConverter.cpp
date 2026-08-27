#include "StringConverter.h"

namespace jug
{

ToUtf16Result ToUtf16(
    std::span<wchar_t>     _outBufOrEmpty,   // _outBufOrEmpty가 비어있으면 크기만 측정함
    const std::string_view _utf8,
    const bool             _bTerminate)
{
    size_t written = 0;
    size_t needed  = 0;
    size_t i       = 0;

    bool         bTrunc      = false;
    const size_t writableCap = _outBufOrEmpty.empty() ? 0
                             : _bTerminate            ? _outBufOrEmpty.size() - 1
                                                      : _outBufOrEmpty.size();

    while (i < _utf8.size())
    {
        uint32_t cp = 0;
        uint8_t  b1 = static_cast<uint8_t>(_utf8[i++]);

        if (b1 < 0x80)
        {
            cp = b1;
        }
        else if ((b1 & 0xE0) == 0xC0)
        {
            if (i >= _utf8.size()) break;
            uint8_t b2 = static_cast<uint8_t>(_utf8[i++]);
            cp         = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
        }
        else if ((b1 & 0xF0) == 0xE0)
        {
            if (i + 1 >= _utf8.size()) break;
            uint8_t b2 = static_cast<uint8_t>(_utf8[i++]);
            uint8_t b3 = static_cast<uint8_t>(_utf8[i++]);
            cp         = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        }
        else if ((b1 & 0xF8) == 0xF0)
        {
            if (i + 2 >= _utf8.size()) break;
            uint8_t b2 = static_cast<uint8_t>(_utf8[i++]);
            uint8_t b3 = static_cast<uint8_t>(_utf8[i++]);
            uint8_t b4 = static_cast<uint8_t>(_utf8[i++]);
            cp         = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
        }
        else
        {
            // Invalid UTF-8 starter byte; skip
            continue;
        }

        // Encode Code Point to UTF-16
        if (cp <= 0xFFFF)
        {
            if (!bTrunc)
            {
                if (written >= writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written] = static_cast<wchar_t>(cp);
                    ++written;
                }
            }
            ++needed;
        }
        else
        {
            // Write surrogate pairs for characters outside the BMP
            cp -= 0x10000;
            const wchar_t high = static_cast<wchar_t>((cp >> 10) + 0xD800);
            const wchar_t low  = static_cast<wchar_t>((cp & 0x3FF) + 0xDC00);

            if (!bTrunc)
            {
                if (written + 1 >= writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written]     = high;
                    _outBufOrEmpty[written + 1] = low;
                    written += 2;
                }
            }
            needed += 2;
        }
    }

    if (_outBufOrEmpty.empty())
    {
        return ToUtf16Result { nullptr, needed };
    }

    if (_bTerminate)
    {
        const size_t termPos    = (written < _outBufOrEmpty.size()) ? written : (_outBufOrEmpty.size() - 1);
        _outBufOrEmpty[termPos] = L'\0';
    }

    return ToUtf16Result { _outBufOrEmpty.data() + written, needed };
}

[[nodiscard]] std::wstring ToUtf16(
    const std::string_view _str)
{
    const ToUtf16Result need = ToUtf16({}, _str, false);
    if (need.size == 0)
    {
        return {};
    }

    std::wstring ret(need.size, L'\0');
    ToUtf16(ret, _str, false);
    return ret;
}

ToUtf8Result ToUtf8(
    std::span<char>         _outBufOrEmpty,
    const std::wstring_view _utf16,
    const bool              _bTerminate)
{
    size_t written = 0;
    size_t needed  = 0;
    size_t i       = 0;

    bool         bTrunc      = false;
    const size_t writableCap = _outBufOrEmpty.empty() ? 0
                             : _bTerminate            ? _outBufOrEmpty.size() - 1
                                                      : _outBufOrEmpty.size();

    while (i < _utf16.size())
    {
        uint32_t cp = static_cast<uint16_t>(_utf16[i]);
        ++i;

        // Surrogate pair 처리
        if (cp >= 0xD800 && cp <= 0xDBFF)
        {
            if (i < _utf16.size())
            {
                const uint32_t low = static_cast<uint16_t>(_utf16[i]);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                    ++i;
                }
                else
                {
                    cp = 0xFFFD;
                }
            }
            else
            {
                cp = 0xFFFD;
            }
        }
        else if (cp >= 0xDC00 && cp <= 0xDFFF)
        {
            cp = 0xFFFD;
        }

        // Encode Code Point to UTF-8
        if (cp <= 0x7F)
        {
            if (!bTrunc)
            {
                if (written >= writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written] = static_cast<char>(cp);
                    ++written;
                }
            }
            ++needed;
        }
        else if (cp <= 0x7FF)
        {
            if (!bTrunc)
            {
                if (written + 2 > writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written++] = static_cast<char>(0xC0 | (cp >> 6));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | (cp & 0x3F));
                }
            }
            needed += 2;
        }
        else if (cp <= 0xFFFF)
        {
            if (!bTrunc)
            {
                if (written + 3 > writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written++] = static_cast<char>(0xE0 | (cp >> 12));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | (cp & 0x3F));
                }
            }
            needed += 3;
        }
        else
        {
            if (!bTrunc)
            {
                if (written + 4 > writableCap)
                {
                    bTrunc = true;
                }
                else
                {
                    _outBufOrEmpty[written++] = static_cast<char>(0xF0 | (cp >> 18));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    _outBufOrEmpty[written++] = static_cast<char>(0x80 | (cp & 0x3F));
                }
            }
            needed += 4;
        }
    }

    if (_outBufOrEmpty.empty())
    {
        return ToUtf8Result { nullptr, needed };
    }

    if (_bTerminate)
    {
        const size_t termPos    = (written < _outBufOrEmpty.size()) ? written : (_outBufOrEmpty.size() - 1);
        _outBufOrEmpty[termPos] = '\0';
    }

    return ToUtf8Result { _outBufOrEmpty.data() + written, needed };
}

[[nodiscard]] std::string ToUtf8(
    const std::wstring_view _str)
{
    const ToUtf8Result need = ToUtf8({}, _str, false);
    if (need.size == 0)
    {
        return {};
    }

    std::string result(need.size, '\0');
    ToUtf8(result, _str, false);
    return result;
}

}   // namespace jug