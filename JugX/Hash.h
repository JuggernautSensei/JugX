#pragma once
#include "MemoryView.h"

namespace jug
{

[[nodiscard]] constexpr uint64_t MixHash(
    const uint64_t _hash0,
    const uint64_t _hash1)
{
    return _hash0 ^ (_hash1 + 0x9e3779b97f4a7c15 + (_hash0 << 12) + (_hash0 >> 4));
}

}   // namespace jug
