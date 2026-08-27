#pragma once
#include "Macros.h"

// ===================================================
//  Heap Alloc
// ===================================================

namespace jug
{

[[nodiscard]] void* Alloc(size_t _size);
void                Free(void* _ptr);

[[nodiscard]] void* AlignedAlloc(size_t _size, size_t _alignment);
void                AlignedFree(void* _ptr, size_t _alignment);

}   // namespace jug

// ===================================================
//  Stack Alloc
// ===================================================

#if defined(_MSC_VER)
#    include <malloc.h>
#    define JUG_STACK_ALLOC(_size) _alloca(_size)
#elif defined(__GNUC__) || defined(__clang__)
#    include <alloca.h>
#    define JUG_STACK_ALLOC(_size) alloca(_size)
#else
#    error "Unsupported compiler for stack allocation"
#endif

#define JUG_STACK_ALGINED_ALLOC(_size, _align)                   \
    JUG_BEGIN_MACRO_BLOCK                                        \
    JUG_ASSERT(IsPowerOf2(_align), "_align must be power of 2"); \
    JUG_STACK_ALLOC((_size + (_align - 1)) & ~(_align - 1))      \
    JUG_END_MACRO_BLOCK
