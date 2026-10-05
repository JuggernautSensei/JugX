#include "pch.h"
#include "TimeStamp.h"

#include <ctime>

#include "EnumRefl.h"
#include "Platform.h"

namespace jug
{

TimeStamp TimeStamp::Now()
{
    const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::tm tm;
#if JUG_OS_WINDOWS
    const bool bOK = ::localtime_s(&tm, &t) == 0;
#else
    const bool bOK = ::localtime_r(&t, &tm) != nullptr;
#endif
    JUG_ASSERT(bOK, "Failed to get local time.\n");

    Optional<eDayOfTheWeek> dayOfWeek = ParseAs<eDayOfTheWeek>(tm.tm_wday);
    JUG_ASSERT(dayOfWeek, "Failed to parse dayOfTheMonth of week.\n");

    TimeStamp ts     = {};
    ts.sec           = tm.tm_sec;
    ts.min           = tm.tm_min;
    ts.hour          = tm.tm_hour;
    ts.year          = tm.tm_year + 1900;
    ts.month         = tm.tm_mon + 1;
    ts.dayOfTheWeek  = *dayOfWeek;
    ts.dayOfTheMonth = tm.tm_mday;
    ts.dayOfTheYear  = tm.tm_yday + 1;
    return ts;
}

}   // namespace jug
