#pragma once
#include "Logger.h"

namespace jug
{

class ConsoleLogger : public Logger
{
private:
    void LogImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void VLogImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;
    void Flush() override;
};

}   // namespace jug
