#include "pch.h"
#include "Alloc.h"

#include <bit>
#include <cstddef>
#include <new>

#include "Assert.h"

namespace jug
{

void* Alloc(
    const size_t _size)
{
    return ::operator new(_size);
}

void Free(
    void* _ptr)
{
    ::operator delete(_ptr);
}

void* Alloc(
    const size_t _size,
    const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");

    // alignedxxx 는 성능이 낮으므로, alignof(std::max_align_t) 이하의 정렬은 일반 new를 사용
    if (_alignment <= alignof(std::max_align_t))
    {
        return ::operator new(_size);
    }
    else
    {
        return ::operator new(_size, static_cast<std::align_val_t>(_alignment));
    }
}

void Free(
    void*        _ptr,
    const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");

    // alignedxxx 는 성능이 낮으므로, alignof(std::max_align_t) 이하의 정렬은 일반 delete를 사용
    if (_alignment <= alignof(std::max_align_t))
    {
        ::operator delete(_ptr);
    }
    else
    {
        ::operator delete(_ptr, static_cast<std::align_val_t>(_alignment));
    }
}

}   // namespace jug