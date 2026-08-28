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
    void LogImpl(eLogLevel _level, std::string_view _msg, bool _bEndLog) override;
    void VFormatImpl(eLogLevel _level, std::string_view _msg, std::format_args _args, bool _bEndLog) override;
    void PushPendding_();

    std::list<std::string> m_logs     = {};
    std::string            m_pendding = {};
    size_t                 m_cap      = 0;
};

}   // namespace jug
