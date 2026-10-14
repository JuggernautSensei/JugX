#pragma once

#include <cstddef>

// ===================================================
//  Stack Alloc
// ===================================================

#if defined(_WIN32)
#    include <malloc.h>
#    define JUG_STACK_ALLOC(_size) _alloca(_size)
#elif defined(JUG_COMPILER_GCC) || defined(JUG_COMPILER_CLANG)
#    include <alloca.h>
#    define JUG_STACK_ALLOC(_size) alloca(_size)
#else
#    error "Unsupported compiler for stack allocation"
#endif

namespace jug
{

// ===================================================
//  Heap Alloc
// ===================================================

[[nodiscard]] void* Malloc(size_t _size) noexcept;
void                Free(void* _ptr) noexcept;

[[nodiscard]] void* Malloc(size_t _size, size_t _alignment) noexcept;
void                Free(void* _ptr, size_t _alignment) noexcept;

}   // namespace jug