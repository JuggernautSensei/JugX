#pragma once
#include <vector>

#include "Logger.h"

namespace jug
{

class GroupLogger : public Logger
{
public:
    void Flush() override;

    void AddLogger(Logger* _pLogger);
    void RemoveLogger(Logger* _pLogger);

private:
    void LogImpl(eLogLevel _level, std::string_view _message, bool _bNewLine) override;
    void FormatImpl(eLogLevel _level, std::string_view _message, std::format_args _args, bool _bNewLine) override;

    std::vector<Logger*> m_loggers = {};
};

}   // namespace jug
