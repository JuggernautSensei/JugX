#pragma once
#include <list>

#include "Logger.h"

namespace jug
{

class MemoryLogger : public Logger
{
public:
    explicit MemoryLogger(size_t _capacity = 1024);
    void Flush() override;
    void Clear();

private:
    void WriteImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void VWriteImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;
    void PushPendding_();

    std::list<String> m_logs     = {};
    String            m_pendding = {};
    size_t                 m_cap      = 0;
};

}   // namespace jug
