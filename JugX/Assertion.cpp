#include "Assertion.h"

#include <format>
#include <iostream>
#include <source_location>

#include "CoreLogger.h"
#include "StringFormat.h"
#include "TimeStamp.h"
#include "Typedef.h"

namespace jug
{

bool CheckAssertion(
    const bool                 _bCondition,
    const StringView           _cond,
    const StringView           _msg,
    const std::source_location _loc)
{
    if (!_bCondition)
    {
        const TimeStamp ts = TimeStamp::Now();

        const String msg = Format(
            "\n"
            "=========================================================\n"
            "  [ASSERTION FAILED]\n"
            "---------------------------------------------------------\n"
            "  Condition : {}\n"
            "  Message   : {}\n"
            "---------------------------------------------------------\n"
            "  Function  : {}\n"
            "  Location  : {}({}:{})\n"
            "  Time      : {:04}-{:02}-{:02} {:02}:{:02}:{:02}\n"
            "=========================================================\n",
            _cond,
            _msg,
            _loc.function_name(),
            _loc.file_name(),
            _loc.line(),
            _loc.column(),
            ts.year,
            ts.month,
            ts.dayOfTheMonth,
            ts.hour,
            ts.min,
            ts.sec);

        if (GetCoreLoggerOrNull())
        {
            JUG_CORE_LOG_FATAL(msg);
        }
        else
        {
            std::cout << msg;
        }

        return false;
    }
    return true;
}

}   // namespace jug