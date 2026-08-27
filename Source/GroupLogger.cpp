#include "GroupLogger.h"

#include <algorithm>

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

void GroupLogger::LogImpl(
    const eLogLevel        _level,
    const std::string_view _message,
    const bool             _bNewLine)
{
    for (Logger* pLogger: m_loggers)
    {
        pLogger->LogImpl(_level, _message, _bNewLine);
    }
}

void GroupLogger::FormatImpl(
    const eLogLevel        _level,
    const std::string_view _message,
    const std::format_args _args,
    const bool              _bNewLine)
{
    for (Logger* pLogger: m_loggers)
    {
        pLogger->FormatImpl(_level, _message, _args, _bNewLine);
    }
}

}   // namespace jug
