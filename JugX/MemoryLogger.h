#pragma once
#include "Logger.h"
#include "RingBuffer.h"

namespace jug
{

class MemoryLogger : public Logger
{
public:
    explicit MemoryLogger(size_t _capacity = 1024);
    void Flush() override;
    void Clear();
    void SetCapacity(size_t _capacity);

private:
    void LogImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void VLogImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;
    void PushPendding_();

    RingBuffer<String> m_logs;
    String             m_pendding = {};
};

}   // namespace jug
