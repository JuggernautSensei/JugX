#pragma once
#include "Logger.h"

namespace jug
{

class ConsoleLogger : public Logger
{
private:
    void LogImpl(eLogLevel _level, std::string_view _msg, bool _bEndLog) override;
    void VFormatImpl(eLogLevel _level, std::string_view _msg, std::format_args _args, bool _bEndLog) override;
    void Flush() override;
};

}   // namespace jug
