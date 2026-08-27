#include "Alloc.h"

#include "Align.h"
#include "Config.h"

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

void* AlignedAlloc(
    const size_t _size,
    const size_t _alignment)
{
    JUG_ASSERT(IsPowerOf2(_alignment), "_alignment must be power of 2");
    return ::operator new(_size, static_cast<std::align_val_t>(_alignment));
}

void AlignedFree(
    void*        _ptr,
    const size_t _alignment)
{
    JUG_ASSERT(IsPowerOf2(_alignment), "_alignment must be power of 2");
    ::operator delete(_ptr, static_cast<std::align_val_t>(_alignment));
}

}   // namespace jug