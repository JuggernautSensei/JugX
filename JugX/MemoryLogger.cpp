#include "pch.h"
#include "MemoryLogger.h"

#include <cstddef>
#include <format>
#include <iterator>
#include <utility>

#include "Assert.h"
#include "Logger.h"
#include "Typedef.h"

namespace jug
{

MemoryLogger::MemoryLogger(
    const size_t _capacity)
    : m_logs(_capacity)
{
}

void MemoryLogger::Flush()
{
}

void MemoryLogger::Clear()
{
    m_logs.Clear();
}

void MemoryLogger::SetCapacity(
    const size_t _capacity)
{
    m_logs.SetCapacity(_capacity);
}

void MemoryLogger::LogImpl(
    const eLogLevel,
    const StringView _msg,
    const bool       _bEndLog)
{
    m_pendding += _msg;
    if (_bEndLog)
    {
        m_pendding += '\n';
        PushPendding_();
    }
}

void MemoryLogger::VLogImpl(
    const eLogLevel,
    const StringView       _msg,
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
    m_logs.PushBack(std::move(m_pendding));
}

}   // namespace jug