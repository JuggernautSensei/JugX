#pragma once
#include <cstdint>

namespace jug
{

// ===========================================
//  Mix Hash
// ===========================================

[[nodiscard]] constexpr uint32_t MixHash(
    const uint32_t _hash0,
    const uint32_t _hash1)
{
    return _hash0 ^ (_hash1 + 0x9e3779b9 + (_hash0 << 6) + (_hash0 >> 2));
}

[[nodiscard]] constexpr uint64_t MixHash(
    const uint64_t _hash0,
    const uint64_t _hash1)
{
    return _hash0 ^ (_hash1 + 0x9e3779b97f4a7c15 + (_hash0 << 12) + (_hash0 >> 4));
}

}   // namespace jug
