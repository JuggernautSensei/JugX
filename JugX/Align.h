#pragma once

namespace jug
{

template<std::unsigned_integral T>
[[nodiscard]] constexpr bool IsAligned(
    const T _value,
    const T _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return (_value & (_alignment - 1)) == 0;
}

template<std::unsigned_integral T>
[[nodiscard]] constexpr T AlignUp(
    const T _value,
    const T _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return (_value + _alignment - 1) & ~(_alignment - 1);
}

template<std::unsigned_integral T>
[[nodiscard]] constexpr T AlignDown(
    const T _value,
    const T _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return _value & ~(_alignment - 1);
}

[[nodiscard]] constexpr bool IsAligned(
    void* const  _ptr,
    const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return (std::bit_cast<uintptr_t>(_ptr) & (_alignment - 1)) == 0;
}

[[nodiscard]] constexpr void* AlignUp(
    void* const  _ptr,
    const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return std::bit_cast<void*>((std::bit_cast<uintptr_t>(_ptr) + _alignment - 1) & ~(_alignment - 1));
}

[[nodiscard]] constexpr void* AlignDown(
    void* const  _ptr,
    const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return std::bit_cast<void*>(std::bit_cast<uintptr_t>(_ptr) & ~(_alignment - 1));
}

[[nodiscard]] constexpr bool IsAligned(
    std::byte* const _ptr,
    const size_t     _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return (std::bit_cast<uintptr_t>(_ptr) & (_alignment - 1)) == 0;
}

[[nodiscard]] constexpr std::byte* AlignUp(
    std::byte* const _ptr,
    const size_t     _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return std::bit_cast<std::byte*>((std::bit_cast<uintptr_t>(_ptr) + _alignment - 1) & ~(_alignment - 1));
}

[[nodiscard]] constexpr std::byte* AlignDown(
    std::byte* const _ptr,
    const size_t     _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    return std::bit_cast<std::byte*>(std::bit_cast<uintptr_t>(_ptr) & ~(_alignment - 1));
}

}   // namespace jug