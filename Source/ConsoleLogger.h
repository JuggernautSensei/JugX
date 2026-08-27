#pragma once
#include "Logger.h"

namespace jug
{

class ConsoleLogger : public Logger
{
private:
    void LogImpl(eLogLevel _level, std::string_view _message, bool _bNewLine) override;
    void FormatImpl(eLogLevel _level, std::string_view _message, std::format_args _args, bool _bNewLine) override;

    void Flush() override;
};

}   // namespace jug
