#pragma once

// ===========================================
//  OS
// ===========================================

#if defined(_WIN64) || defined(_WIN32)
#    define JUG_OS_WINDOWS 1
#    define JUG_OS_NAME    "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
#    include <TargetConditionals.h>
#    if TARGET_OS_IPHONE
#        define JUG_OS_IOS  1
#        define JUG_OS_NAME "iOS"
#    else
#        define JUG_OS_MACOS 1
#        define JUG_OS_NAME  "macOS"
#    endif
#elif defined(__ANDROID__)
#    define JUG_OS_ANDROID 1
#    define JUG_OS_NAME    "Android"
#elif defined(__linux__)
#    define JUG_OS_LINUX 1
#    define JUG_OS_NAME  "Linux"
#elif defined(__FreeBSD__)
#    define JUG_OS_FREEBSD 1
#    define JUG_OS_NAME    "FreeBSD"
#elif defined(__unix__)
#    define JUG_OS_UNIX 1
#    define JUG_OS_NAME "Unix"
#else
#    define JUG_OS_UNKNOWN 1
#    define JUG_OS_NAME    "Unknown"
#endif

// ===========================================
//  CPP
// ===========================================

#if defined(_MSC_VER) && defined(_MSVC_LANG)
#    define JUG_CPP_STANDARD _MSVC_LANG
#else
#    define JUG_CPP_STANDARD __cplusplus
#endif

#if JUG_CPP_STANDARD >= 202302L
#    define JUG_CPP_VERSION 23
#elif JUG_CPP_STANDARD >= 202002L
#    define JUG_CPP_VERSION 20
#elif JUG_CPP_STANDARD >= 201703L
#    define JUG_CPP_VERSION 17
#elif JUG_CPP_STANDARD >= 201402L
#    define JUG_CPP_VERSION 14
#elif JUG_CPP_STANDARD >= 201103L
#    define JUG_CPP_VERSION 11
#else
#    define JUG_CPP_VERSION 3   // C++98/03
#endif

// ===========================================
//  Compiler
// ===========================================

#ifdef __clang__
#    define JUG_COMPILER_CLANG         1
#    define JUG_COMPILER_NAME          "Clang"
#    define JUG_COMPILER_VERSION_MAJOR __clang_major__
#    define JUG_COMPILER_VERSION_MINOR __clang_minor__
#    define JUG_COMPILER_VERSION_PATCH __clang_patchlevel__
#elif defined(__INTEL_COMPILER) || defined(__ICC) || defined(__ICL)
#    define JUG_COMPILER_INTEL         1
#    define JUG_COMPILER_NAME          "Intel C++ Compiler"
#    define JUG_COMPILER_VERSION_MAJOR (__INTEL_COMPILER / 100)
#    define JUG_COMPILER_VERSION_MINOR (__INTEL_COMPILER % 100)
#    define JUG_COMPILER_VERSION_PATCH 0
#elif defined(_MSC_VER)
#    define JUG_COMPILER_MSVC          1
#    define JUG_COMPILER_NAME          "MSVC"
#    define JUG_COMPILER_VERSION_MAJOR (_MSC_VER / 100)
#    define JUG_COMPILER_VERSION_MINOR (_MSC_VER % 100)
#    define JUG_COMPILER_VERSION_PATCH 0
#elif defined(__GNUC__)
#    define JUG_COMPILER_GCC           1
#    define JUG_COMPILER_NAME          "GCC"
#    define JUG_COMPILER_VERSION_MAJOR __GNUC__
#    define JUG_COMPILER_VERSION_MINOR __GNUC_MINOR__
#    define JUG_COMPILER_VERSION_PATCH __GNUC_PATCHLEVEL__
#else
#    define JUG_COMPILER_UNKNOWN       1
#    define JUG_COMPILER_NAME          "Unknown"
#    define JUG_COMPILER_VERSION_MAJOR 0
#    define JUG_COMPILER_VERSION_MINOR 0
#    define JUG_COMPILER_VERSION_PATCH 0
#endif

// ===========================================
//  Architecture
// ===========================================

#if defined(__x86_64__) || defined(_M_X64)
#    define JUG_ARCH_X64  1
#    define JUG_ARCH_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#    define JUG_ARCH_X86  1
#    define JUG_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#    define JUG_ARCH_ARM64 1
#    define JUG_ARCH_NAME  "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
#    define JUG_ARCH_ARM  1
#    define JUG_ARCH_NAME "ARM"
#elif defined(__riscv) && (__riscv_xlen == 64)
#    define JUG_ARCH_RISCV64 1
#    define JUG_ARCH_NAME    "RISC-V64"
#elif defined(__riscv) && (__riscv_xlen == 32)
#    define JUG_ARCH_RISCV32 1
#    define JUG_ARCH_NAME    "RISC-V32"
#elif defined(JUG_ERROR_ON_UNKNOWN_ARCH)
#    error "Unknown architecture"
#else
#    define JUG_ARCH_UNKNOWN 1
#    define JUG_ARCH_NAME    "Unknown"
#endif

// ===========================================
//  Endian
// ===========================================

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#    define JUG_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#    define JUG_LITTLE_ENDIAN 1
#else
#    if defined(_WIN32) || defined(_WIN64)
#        define JUG_LITTLE_ENDIAN 1
#    else
#        error "Unknown endianness"
#    endif
#endif