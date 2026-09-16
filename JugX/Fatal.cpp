#include "pch.h"
#include "Fatal.h"

#include "CoreLogger.h"
#include "FileWriter.h"
#include "StringFormat.h"
#include "TimeStamp.h"

namespace jug
{

void ReportVFatal(
    const std::source_location& _loc,
    const StringView            _fmt,
    const std::format_args      _args)
{
    const TimeStamp ts  = TimeStamp::Now();
    const String    msg = Format(
        "\n"
        "=========================================================\n"
        "  [FATAL ERROR]\n"
        "---------------------------------------------------------\n"
        "  Message   : {}\n"
        "---------------------------------------------------------\n"
        "  Function  : {}\n"
        "  Location  : {}({}:{})\n"
        "  Time      : {:04}-{:02}-{:02} {:02}:{:02}:{:02}\n"
        "=========================================================\n",
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

    JUG_CORE_LOG_FATAL(msg);

    fs::create_directories("CrashDumps");
    const String filename = Format("CrashDumps/{:04}{:02}{:02}_{:02}{:02}{:02}.log", ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec);
    if (Result<FileWriter> writer = FileWriter::Open(filename))
    {
        writer->Write(msg);
    }
}

}   // namespace jug