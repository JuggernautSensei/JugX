#pragma once
#include "Logger.h"

namespace jug
{

class ConsoleLogger : public Logger
{
private:
    void WriteImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void WriteImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;
    void Flush() override;
};

}   // namespace jug
