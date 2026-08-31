#include "DebugLogger.h"

#include "StringEncoder.h"
#include "StringFormatter.h"

#include <Windows.h>

namespace jug
{

void DebugLogger::Flush()
{
}

void DebugLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const bool             _bEnd)
{
    AppendUtf8(m_utf16Buf, _msg);
    if (_bEnd)
    {
        m_utf16Buf += L'\n';
        ::OutputDebugStringW(m_utf16Buf.c_str());
        m_utf16Buf.clear();
    }
}

void DebugLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const std::format_args _args,
    const bool             _bEnd)
{
    std::vformat_to(std::back_inserter(m_utf8Buf), _msg, _args);
    if (_bEnd)
    {
        AppendUtf8(m_utf16Buf, m_utf8Buf);
        m_utf16Buf += L'\n';
        ::OutputDebugStringW(m_utf16Buf.c_str());
        m_utf8Buf.clear();
        m_utf16Buf.clear();
    }
}

}   // namespace jug