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
    void LogImpl(eLogLevel _level, std::string_view _msg, bool _bEndLog) override;
    void VFormatImpl(eLogLevel _level, std::string_view _msg, std::format_args _args, bool _bEndLog) override;

    std::vector<Logger*> m_loggers = {};
};

}   // namespace jug
