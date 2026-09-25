#pragma once

namespace jug
{

// ===================================================
//  Stack Alloc
// ===================================================

#if defined(JUG_COMPILER_MSVC)
#    include <malloc.h>
#    define JUG_STACK_ALLOC(_size) _alloca(_size)
#elif defined(JUG_COMPILER_GCC) || defined(JUG_COMPILER_CLANG)
#    include <alloca.h>
#    define JUG_STACK_ALLOC(_size) alloca(_size)
#else
#    error "Unsupported compiler for stack allocation"
#endif

// ===========================================
//  Aligned Alloca
// ===========================================

#if defined(JUG_COMPILER_MSVC)
#    include <stdlib.h>
#    define JUG_ALIGNED_ALLOC(_size, _alignment) _aligned_malloc(_size, _alignment)
#elif defined(JUG_COMPILER_GCC) || defined(JUG_COMPILER_CLANG)
#    include <stdlib.h>
#    define JUG_ALIGNED_ALLOC(_size, _alignment) aligned_alloc(_alignment, _size)
#else
#    error "Unsupported compiler for aligned stack allocation"
#endif

// ===================================================
//  Heap AllocMemory
// ===================================================

[[nodiscard]] void* Alloc(size_t _size);
void                Free(void* _ptr);

[[nodiscard]] void* Alloc(size_t _size, size_t _alignment);
void                Free(void* _ptr, size_t _alignment);

}   // namespace jug
