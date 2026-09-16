#include "pch.h"
#include "Error.h"

namespace jug
{

Error::Error(
    const OkType)
    : m_errorCode(0)
    , m_pCategory(nullptr)
{
}

Error::Error(
    const int       _errorCode,
    IErrorCategory& _category)
    : m_errorCode(_errorCode)
    , m_pCategory(&_category)
{
}

void Error::Reset()
{
    m_errorCode = 0;
    m_pCategory = nullptr;
}

bool Error::IsError() const
{
    return m_errorCode != 0;
}

bool Error::IsOK() const
{
    return !IsError();
}

IErrorCategory* Error::GetCategory() const
{
    return m_pCategory;
}

int Error::GetErrorCode() const
{
    return m_errorCode;
}

String Error::MakeMessage() const
{
    if (m_pCategory)
    {
        return m_pCategory->MakeMessage(m_errorCode);
    }
    else if (m_errorCode != 0)
    {
        return Format("Unknown error code: {}", m_errorCode);
    }
    else
    {
        return "No error";
    }
}

Error::operator bool() const
{
    return IsError();
}

bool Error::operator==(
    const Error _other) const
{
    return m_errorCode == _other.m_errorCode && m_pCategory == _other.m_pCategory;
}

}   // namespace jug