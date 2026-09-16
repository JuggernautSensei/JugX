#pragma once
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
    void LogImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void VLogImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;

    Vector<Logger*> m_loggers = {};
};

}   // namespace jug
