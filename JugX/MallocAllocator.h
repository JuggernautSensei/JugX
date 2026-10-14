#pragma once
#include "Alloc.h"
#include "Assert.h"
#include "Math.h"

namespace jug
{

template<typename T>
class MallocAllocator
{
public:
    using value_type      = T;
    using is_always_equal = std::true_type;

    MallocAllocator() noexcept = default;

    template<typename U>
    explicit MallocAllocator(
        const MallocAllocator<U>&) noexcept
    {
    }

    [[nodiscard]] T* allocate(
        const size_t n)
    {
        JUG_ASSERT(n <= max_size(), "Allocation size overflow");

        void* ptr = jug::Malloc(n * sizeof(T), alignof(T));
        if (!ptr)
        {
            throw std::bad_alloc {};
        }

        return static_cast<T*>(ptr);
    }

    void deallocate(
        T* ptr,
        size_t) noexcept
    {
        jug::Free(ptr, alignof(T));
    }

    constexpr size_t max_size() const noexcept
    {
        return Max<size_t>() / sizeof(T);
    }
};

template<typename T, typename U>
constexpr bool operator==(
    const MallocAllocator<T>&,
    const MallocAllocator<U>&) noexcept
{
    return true;
}

template<typename T, typename U>
constexpr bool operator!=(
    const MallocAllocator<T>&,
    const MallocAllocator<U>&) noexcept
{
    return false;
}

}   // namespace jug