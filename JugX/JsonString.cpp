#include "pch.h"
#include "JsonString.h"

namespace jug
{

JsonString::JsonString(
    char*        _pStr,
    const size_t _len)
    : m_pStr(_pStr)
    , m_len(_len)
{
}

JsonString::JsonString(
    JsonString&& _other) noexcept
    : m_pStr(std::exchange(_other.m_pStr, nullptr))
    , m_len(std::exchange(_other.m_len, 0))
{
}

JsonString& JsonString::operator=(
    JsonString&& _other) noexcept
{
    if (this != &_other)
    {
        Reset();
        m_pStr = std::exchange(_other.m_pStr, nullptr);
        m_len  = std::exchange(_other.m_len, 0);
    }
    return *this;
}

JsonString::~JsonString()
{
    Reset();
}

void JsonString::Reset()
{
    if (m_pStr)
    {
        std::free(m_pStr);
        m_pStr = nullptr;
        m_len  = 0;
    }
}

size_t JsonString::GetSize() const
{
    return m_len;
}

const char* JsonString::GetPtr() const
{
    return m_pStr;
}

char* JsonString::GetPtr()
{
    return m_pStr;
}

JsonString::operator std::string_view() const
{
    return StringView { m_pStr, m_len };
}

JsonString::operator std::string() const
{
    return String { m_pStr, m_len };
}

JsonString::Iterator JsonString::Begin()
{
    return m_pStr;
}

JsonString::Iterator JsonString::End()
{
    return m_pStr + m_len;
}

JsonString::ConstIterator JsonString::Begin() const
{
    return m_pStr;
}

JsonString::ConstIterator JsonString::End() const
{
    return m_pStr + m_len;
}

JsonString::ConstIterator JsonString::CBegin() const
{
    return m_pStr;
}

JsonString::ConstIterator JsonString::CEnd() const
{
    return m_pStr + m_len;
}

JsonString::value_type* JsonString::data()
{
    return m_pStr;
}

const JsonString::value_type* JsonString::data() const
{
    return m_pStr;
}

JsonString::iterator JsonString::begin()
{
    return m_pStr;
}

JsonString::iterator JsonString::end()
{
    return m_pStr + m_len;
}

JsonString::const_iterator JsonString::begin() const
{
    return m_pStr;
}

JsonString::const_iterator JsonString::end() const
{
    return m_pStr + m_len;
}

JsonString::const_iterator JsonString::cbegin() const
{
    return m_pStr;
}

JsonString::const_iterator JsonString::cend() const
{
    return m_pStr + m_len;
}

}   // namespace jug