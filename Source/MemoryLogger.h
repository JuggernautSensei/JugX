#pragma once
#include <list>

#include "Logger.h"

namespace jug
{

class MemoryLogger : public Logger
{
public:


private:
    std::list<std::string> m_logs;
};

}   // namespace jug
