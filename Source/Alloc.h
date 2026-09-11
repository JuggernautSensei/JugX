#pragma once
#include "Macro.h"
#include <memory>

namespace jug
{

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

// ===================================================
//  Heap Alloc
// ===================================================

[[nodiscard]] void* Alloc(size_t _size);
void                Free(void* _ptr);

[[nodiscard]] void* Alloc(size_t _size, size_t _alignment);
void                Free(void* _ptr, size_t _alignment);

}   // namespace jug
