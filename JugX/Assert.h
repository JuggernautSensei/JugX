#pragma once
#include <source_location>

#include "Config.h"
#include "Macro.h"
#include "Typedef.h"

namespace jug
{

void ReportVAssert(const std::source_location& _loc, StringView _hint, StringView _fmt, std::format_args _args);

template<typename... Args>
void ReportAssert(
    const std::source_location&       _loc,
    const StringView                  _hint,
    const std::format_string<Args...> _fmt,
    Args&&... _args)
{
    ReportVAssert(_loc, _hint, _fmt.get(), std::make_format_args(_args...));
}

}   // namespace jug

#ifndef JUG_ENABLE_ASSERT
#    ifdef JUG_DEBUG
#        define JUG_ENABLE_ASSERT 1
#    else
#        define JUG_ENABLE_ASSERT 0
#    endif
#endif

#if JUG_ENABLE_ASSERT
#    define JUG_ASSERT(_cond, ...)                                                   \
        JUG_BEGIN_MACRO_BLOCK                                                        \
        if (!std::is_constant_evaluated() && !(_cond))                               \
        {                                                                            \
            jug::ReportAssert(std::source_location::current(), #_cond, __VA_ARGS__); \
            JUG_DEBUG_BREAK();                                                       \
        }                                                                            \
        JUG_END_MACRO_BLOCK
#else
#    define JUG_ASSERT(_cond, _msg) ((void)0)
#endif