#include "MemoryLogger.h"
#include <cstddef>
#include "Logger.h"
#include "Typedef.h"
#include <format>
#include <iterator>
#include "Assertion.h"
#include <utility>

namespace jug
{

MemoryLogger::MemoryLogger(
    const size_t _capacity)
    : m_cap(_capacity)
{
}

void MemoryLogger::Flush()
{
}

void MemoryLogger::Clear()
{
    m_logs.clear();
}

void MemoryLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const bool             _bEndLog)
{
    m_pendding += _msg;
    if (_bEndLog)
    {
        m_pendding += '\n';
        PushPendding_();
    }
}

void MemoryLogger::VWriteImpl(
    const eLogLevel,
    const StringView _msg,
    const std::format_args _args,
    const bool             _bEndLog)
{
    std::vformat_to(std::back_inserter(m_pendding), _msg, _args);
    if (_bEndLog)
    {
        m_pendding += '\n';
        PushPendding_();
    }
}

void MemoryLogger::PushPendding_()
{
    JUG_ASSERT(m_logs.size() <= m_cap, "MemoryLogger capacity exceeded.\n");

    if (m_logs.size() == m_cap)
    {
        m_logs.pop_front();
    }
    m_logs.push_back(std::move(m_pendding));
}

}   // namespace jug