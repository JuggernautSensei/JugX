#pragma once
#include "Logger.h"

namespace jug
{

class DebugLogger : public Logger
{
public:
    void Flush() override;

private:
    void WriteImpl(eLogLevel _level, StringView _msg, bool _bEnd) override;
    void VWriteImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEnd) override;

    String  m_utf8Buf  = {};
    WString m_utf16Buf = {};
};

}   // namespace jug
