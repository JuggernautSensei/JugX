#include "pch.h"
#include "Alloc.h"

#include <bit>
#include <cstddef>
#include <cstdlib>

#if !defined(_WIN32)
#    if defined(__unix__) || defined(__APPLE__)
#        include <stdlib.h>
#    else
#        error "Unsupported platform for aligned allocation"
#    endif
#endif

#include "Assert.h"

namespace jug
{

void* Malloc(
    const size_t _size) noexcept
{
    return std::malloc(_size);
}

void Free(
    void* _ptr) noexcept
{
    std::free(_ptr);
}

void* Malloc(
    const size_t _size,
    const size_t _alignment) noexcept
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");

    // 기본 정렬 범위에서는 일반 malloc 사용
    if (_alignment <= alignof(std::max_align_t))
    {
        return std::malloc(_size);
    }

#if defined(_WIN32)

    return ::_aligned_malloc(_size, _alignment);

#else

    void* ptr = nullptr;
    if (::posix_memalign(&ptr, _alignment, _size) != 0)
    {
        return nullptr;
    }

    return ptr;

#endif
}

void Free(
    void*        _ptr,
    const size_t _alignment) noexcept
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");

#if defined(_WIN32)

    if (_alignment > alignof(std::max_align_t))
    {
        ::_aligned_free(_ptr);
        return;
    }

#endif

    std::free(_ptr);
}

}   // namespace jug