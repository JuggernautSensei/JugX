#include "pch.h"
#include "Assert.h"

#include "CoreLogger.h"
#include "StringFormat.h"
#include "TimeStamp.h"
#include "Typedef.h"

namespace jug
{

void ReportVAssert(
    const std::source_location& _loc,
    const StringView            _hint,
    const StringView            _fmt,
    const std::format_args      _args)
{
    const TimeStamp ts = TimeStamp::Now();
    JUG_CORE_LOG_FATAL("\n"
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
                       _hint,
                       VFormat(_fmt, _args),
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

    // 비동기 로깅시 로깅전에 프로그램이 멈출 수 있기 때문에 잠시 슬립
    std::this_thread::sleep_for(100ms);
}

}   // namespace jug