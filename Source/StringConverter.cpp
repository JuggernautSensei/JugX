#include "StringConverter.h"

namespace jug
{

size_t ToUtf8(
    const std::span<char>   _outUtf8,
    const std::wstring_view _utf16,
    const bool              _bTerminate)
{
    size_t       written = 0;
    const size_t cap     = _bTerminate ? _outUtf8.size() - 1 : _outUtf8.size();

    for (const wchar_t wc: _utf16)
    {
        if (wc <= 0x7F)
        {
            if (written >= cap)
            {
                break;
            }

            _outUtf8[written++] = static_cast<char>(wc);
        }
        else if (wc <= 0x7FF)
        {
            if (written + 1 >= cap)
            {
                break;
            }

            _outUtf8[written++] = static_cast<char>(0xC0 | ((wc >> 6) & 0x1F));
            _outUtf8[written++] = static_cast<char>(0x80 | (wc & 0x3F));
        }
        else
        {
            if (written + 2 >= cap)
            {
                break;
            }

            _outUtf8[written++] = static_cast<char>(0xE0 | ((wc >> 12) & 0x0F));
            _outUtf8[written++] = static_cast<char>(0x80 | ((wc >> 6) & 0x3F));
            _outUtf8[written++] = static_cast<char>(0x80 | (wc & 0x3F));
        }
    }

    if (_bTerminate)
    {
        _outUtf8[written] = '\0';
    }

    return written;
}

std::string ToUtf8(
    const std::wstring_view _utf16)
{
    const size_t size = CalcSizeToUtf8(_utf16);
    if (size == 0)
    {
        return {};
    }

    std::string utf8(size, '\0');
    ToUtf8(utf8, _utf16, false);
    return utf8;
}

size_t CalcSizeToUtf8(
    const std::wstring_view _utf16)
{
    size_t size = 0;
    for (const wchar_t wc: _utf16)
    {
        if (wc <= 0x7F)
        {
            size += 1;
        }
        else if (wc <= 0x7FF)
        {
            size += 2;
        }
        else
        {
            size += 3;
        }
    }
    return size;
}

size_t ToUtf16(
    const std::span<wchar_t> _outUtf16,
    const std::string_view   _utf8,
    const bool               _bTerminate)
{
    size_t       written = 0;
    const size_t cap     = _bTerminate ? _outUtf16.size() - 1 : _outUtf16.size();
    for (size_t i = 0; i < _utf8.size();)
    {
        uint32_t      codepoint = 0;
        size_t        bytes     = 0;
        unsigned char c         = static_cast<unsigned char>(_utf8[i]);
        if (c <= 0x7F)
        {
            codepoint = c;
            bytes     = 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            codepoint = c & 0x1F;
            bytes     = 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            codepoint = c & 0x0F;
            bytes     = 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            codepoint = c & 0x07;
            bytes     = 4;
        }
        else
        {
            // Invalid UTF-8 sequence
            break;
        }
        for (size_t j = 1; j < bytes; ++j)
        {
            if (i + j >= _utf8.size())
            {
                // Incomplete UTF-8 sequence
                break;
            }
            unsigned char next_c = static_cast<unsigned char>(_utf8[i + j]);
            if ((next_c & 0xC0) != 0x80)
            {
                // Invalid continuation byte
                break;
            }
            codepoint = (codepoint << 6) | (next_c & 0x3F);
        }
        if (written >= cap)
        {
            break;
        }
        _outUtf16[written++] = static_cast<wchar_t>(codepoint);
        i += bytes;
    }
    if (_bTerminate)
    {
        _outUtf16[written] = L'\0';
    }
    return written;
}

std::wstring ToUtf16(
    const std::string_view _utf8)
{
    const size_t size = CalcSizeToUtf16(_utf8);
    if (size == 0)
    {
        return {};
    }

    std::wstring utf16(size, L'\0');
    ToUtf16(utf16, _utf8, false);
    return utf16;
}

size_t CalcSizeToUtf16(
    const std::string_view _utf8)
{
    size_t size = 0;
    for (size_t i = 0; i < _utf8.size();)
    {
        unsigned char c = static_cast<unsigned char>(_utf8[i]);
        if (c <= 0x7F)
        {
            size += 1;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            size += 1;
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            size += 1;
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            size += 1;
            i += 4;
        }
        else
        {
            // Invalid UTF-8 sequence
            break;
        }
    }
    return size;
}

}   // namespace jug