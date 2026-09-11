#pragma once
#include <source_location>
#include <type_traits>

#include "Config.h"
#include "Macro.h"
#include "Typedef.h"

namespace jug
{

bool CheckAssertion(bool _bCondition, StringView _cond, StringView _msg, std::source_location _loc);

}

#if !defined(JUG_ENABLE_ASSERT) && defined(JUG_DEBUG)
#    define JUG_ENABLE_ASSERT 1
#endif

#if JUG_ENABLE_ASSERT == 1
#    define JUG_ASSERT(_cond, _msg)                                                                                           \
        JUG_BEGIN_MACRO_BLOCK                                                                                                 \
        if (!std::is_constant_evaluated() && !jug::CheckAssertion((bool)(_cond), #_cond, _msg, std::source_location::current())) \
        {                                                                                                                     \
            JUG_DEBUG_BREAK();                                                                                                \
        }                                                                                                                     \
        JUG_END_MACRO_BLOCK
#else
#    define JUG_ASSERT(_cond, _msg) ((void)0)
#endif