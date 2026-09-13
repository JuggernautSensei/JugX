#include "GroupLogger.h"

#include <algorithm>
#include <format>

#include "Assertion.h"
#include "Logger.h"

namespace jug
{

void GroupLogger::Flush()
{
    for (Logger* pLogger: m_loggers)
    {
        pLogger->Flush();
    }
}

void GroupLogger::AddLogger(
    Logger* _pLogger)
{
    JUG_ASSERT(_pLogger, "Logger pointer is null.\n");
    JUG_ASSERT(_pLogger != this, "Cannot add self as a logger.\n");
    JUG_ASSERT(std::ranges::find(m_loggers, _pLogger) == m_loggers.end(), "Logger already added.\n");
    m_loggers.push_back(_pLogger);
}

void GroupLogger::RemoveLogger(
    Logger* _pLogger)
{
    JUG_ASSERT(_pLogger, "Logger pointer is null.\n");
    JUG_ASSERT(_pLogger != this, "Cannot add self as a logger.\n");
    if (std::erase(m_loggers, _pLogger) == 0)
    {
        JUG_ASSERT(false, "Logger not found.\n");
    }
}

void GroupLogger::WriteImpl(
    const eLogLevel        _level,
    const StringView _msg,
    const bool             _bEndLog)
{
    for (Logger* pLogger: m_loggers)
    {
        pLogger->WriteImpl(_level, _msg, _bEndLog);
    }
}

void GroupLogger::VWriteImpl(
    const eLogLevel        _level,
    const StringView _msg,
    const std::format_args _args,
    const bool              _bEndLog)
{
    for (Logger* pLogger: m_loggers)
    {
        pLogger->VWriteImpl(_level, _msg, _args, _bEndLog);
    }
}

}   // namespace jug
