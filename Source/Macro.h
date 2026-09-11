#pragma once

#ifdef _MSC_VER
#    define JUG_DISABLE_ANON_WARNING_BEGIN \
        __pragma(warning(push))            \
            __pragma(warning(disable : 4201))
#    define JUG_DISABLE_ANON_WARNING_END \
        __pragma(warning(pop))
#elif defined(__clang__)
#    define JUG_DISABLE_ANON_WARNING_BEGIN                                 \
        _Pragma("clang diagnostic push")                                   \
            _Pragma("clang diagnostic ignored \"-Wgnu-anonymous-struct\"") \
                _Pragma("clang diagnostic ignored \"-Wnested-anon-types\"")
#    define JUG_DISABLE_ANON_WARNING_END \
        _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#    define JUG_DISABLE_ANON_WARNING_BEGIN \
        _Pragma("GCC diagnostic push")     \
            _Pragma("GCC diagnostic ignored \"-Wpedantic\"")
#    define JUG_DISABLE_ANON_WARNING_END \
        _Pragma("GCC diagnostic pop")
#else
#    define JUG_DISABLE_ANON_WARNING_BEGIN
#    define JUG_DISABLE_ANON_WARNING_END
#endif

#ifdef _MSC_VER
#    define JUG_FORCEINLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#    define JUG_FORCEINLINE inline __attribute__((always_inline))
#else
#    define JUG_FORCEINLINE inline
#endif

#ifndef JUG_PRETTY_FUNCTION
#    if defined(__GNUC__) || defined(__clang__)
#        define JUG_PRETTY_FUNCTION __PRETTY_FUNCTION__
#    elif defined(_MSC_VER)
#        define JUG_PRETTY_FUNCTION __FUNCSIG__
#    else
#        error "Unsupported compiler for JUG_PRETTY_FUNCTION"
#    endif
#endif

#if defined(_MSC_VER)
#    include <intrin.h>
#    define JUG_DEBUG_BREAK() __debugbreak()

#elif defined(__clang__) || defined(__GNUC__)
#    if defined(__has_builtin)
#        if __has_builtin(__builtin_debugtrap)
#            define JUG_DEBUG_BREAK() __builtin_debugtrap()
#        endif
#    endif

#    if !defined(JUG_DEBUG_BREAK)
#        if defined(__i386__) || defined(__x86_64__)
#            define JUG_DEBUG_BREAK() __asm__ volatile("int $0x03")
#        elif defined(__aarch64__)
#            define JUG_DEBUG_BREAK() __asm__ volatile(".inst 0xd4200000")
#        elif defined(__arm__)
#            define JUG_DEBUG_BREAK() __asm__ volatile(".inst 0xe7f001f0")
#        else
#            include <csignal>
#            define JUG_DEBUG_BREAK() raise(SIGTRAP)
#        endif
#    endif

#else
#    include <csignal>
#    define JUG_DEBUG_BREAK() raise(SIGTRAP)
#endif

#define JUG_NOOP (void)0

#define JUG_BEGIN_MACRO_BLOCK \
    do                        \
    {

#define JUG_END_MACRO_BLOCK \
    }                       \
    while (0)

#define JUG_UNREACHABLE_RETURN(T) return std::declval<T>()
#define JUG_DISCARD_RETURN(_fn)   (void)(_fn)
#define JUG_ALWAYS_FALSE(T)       (sizeof(T) == 0)

#define JUG_CONCAT_IMPL(_a, _b) _a##_b
#define JUG_CONCAT(_a, _b)      JUG_CONCAT_IMPL(_a, _b)