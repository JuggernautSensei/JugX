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

#if defined(__GNUC__) || defined(__clang__)
#    define JUG_FUNCSIG __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#    define JUG_FUNCSIG __FUNCSIG__
#else
#    error "Unsupported compiler for JUG_FUNCSIG"
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
    while (false)

#define JUG_UNREACHABLE_RETURN(T) \
    return T {}
#define JUG_DISCARD_RETURN(_fn) (void)(_fn)
#define JUG_ALWAYS_FALSE(T)     (sizeof(T) == 0)

#define JUG_CONCAT_IMPL(_a, _b) _a##_b
#define JUG_CONCAT(_a, _b)      JUG_CONCAT_IMPL(_a, _b)

#define JUG_FOURCC(_a, _b, _c, _d) ((static_cast<uint32_t>(_a) | (static_cast<uint32_t>(_b) << 8) | (static_cast<uint32_t>(_c) << 16) | (static_cast<uint32_t>(_d) << 24)))

// ===========================================
//  Class
// ===========================================

#define JUG_CLASS_NO_COPY(_class)              \
    _class(const _class&)            = delete; \
    _class& operator=(const _class&) = delete;

#define JUG_CLASS_NO_MOVE(_class)         \
    _class(_class&&)            = delete; \
    _class& operator=(_class&&) = delete;

#define JUG_CLASS_DEFAULT_COPY(_class)          \
    _class(const _class&)            = default; \
    _class& operator=(const _class&) = default;

#define JUG_CLASS_DEFAULT_MOVE(_class)     \
    _class(_class&&)            = default; \
    _class& operator=(_class&&) = default;

#define JUG_CLASS_EX_1(_class, _op)      JUG_CLASS_##_op(_class)
#define JUG_CLASS_EX_2(_class, _op, ...) JUG_CLASS_##_op(_class) JUG_CLASS_EX_1(_class, __VA_ARGS__)
#define JUG_CLASS_EX_3(_class, _op, ...) JUG_CLASS_##_op(_class) JUG_CLASS_EX_2(_class, __VA_ARGS__)
#define JUG_CLASS_EX_4(_class, _op, ...) JUG_CLASS_##_op(_class) JUG_CLASS_EX_3(_class, __VA_ARGS__)

#define JUG_CLASS_IMPL(_1, _2, _3, _4, NAME, ...) NAME

#define JUG_CLASS(_class, ...) \
public:                        \
JUG_CLASS_IMPL(__VA_ARGS__, JUG_CLASS_EX_4, JUG_CLASS_EX_3, JUG_CLASS_EX_2, JUG_CLASS_EX_1)(_class, __VA_ARGS__) private:
