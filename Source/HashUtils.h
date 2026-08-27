#pragma once
#include <cstdint>

namespace jug
{

[[nodiscard]] constexpr uint32_t CombineHash(
    const uint32_t hash0,
    const uint32_t hash1)
{
    return hash0 ^ (hash1 + 0x9e3779b9 + (hash0 << 6) + (hash0 >> 2));
}

[[nodiscard]] constexpr uint64_t CombineHash(
    const uint64_t hash0,
    const uint64_t hash1)
{
    return hash0 ^ (hash1 + 0x9e3779b97f4a7c15 + (hash0 << 12) + (hash0 >> 4));
}

}   // namespace jug
