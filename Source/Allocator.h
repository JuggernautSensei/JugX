#pragma once
#include "Alloc.h"

namespace jug
{

template<typename T>
struct Allocator
{
    static_assert(!std::is_const_v<T>, "The C++ Standard forbids containers of const elements because allocator<const T> is ill-formed.");
    static_assert(!std::is_function_v<T>, "The C++ Standard forbids allocators for function elements because of [allocator.requirements].");
    static_assert(!std::is_reference_v<T>, "The C++ Standard forbids allocators for reference elements because of [allocator.requirements].");

public:
    constexpr Allocator() noexcept = default;

    template<class Other>
    /* implicit */ constexpr Allocator(
        const Allocator<Other>&) noexcept
    {
    }

    [[nodiscard]] T* Alloc(
        const size_t _n)
    {
        if constexpr (alignof(T) <= alignof(void*))
        {
            return static_cast<T*>(jug::Alloc(sizeof(T) * _n));
        }
        else
        {
            return static_cast<T*>(jug::Alloc(sizeof(T) * _n, alignof(T)));
        }
    }

    void Free(
        T* _p,
        const size_t)
    {
        if constexpr (alignof(T) <= alignof(void*))
        {
            jug::Free(_p);
        }
        else
        {
            jug::Free(_p, alignof(T));
        }
    }

    // ===========================================
    //  STL Like
    // ===========================================

    using value_type                             = T;
    using size_type                              = size_t;
    using difference_type                        = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal                        = std::true_type;

    [[nodiscard]] T* allocate(
        const size_t _n)
    {
        return Alloc(_n);
    }

    void deallocate(
        T*           _p,
        const size_t _n)
    {
        Free(_p, _n);
    }
};

}   // namespace jug