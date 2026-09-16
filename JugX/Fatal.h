#pragma once
#include <source_location>

#include "Typedef.h"

namespace jug
{

void ReportVFatal(const std::source_location& _loc, StringView _fmt, std::format_args _args);

template<typename... Args>
void ReportFatal(
    const std::source_location&       _loc,
    const std::format_string<Args...> _fmt,
    Args&&... _args)
{
    ReportVFatal(_loc, _fmt.get(), std::make_format_args(_args...));
}

}   // namespace jug

#define JUG_FATAL(...)                                              \
    JUG_BEGIN_MACRO_BLOCK                                           \
    jug::ReportFatal(std::source_location::current(), __VA_ARGS__); \
    std::terminate();                                               \
    JUG_END_MACRO_BLOCK
