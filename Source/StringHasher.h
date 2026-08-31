#pragma once
#include <cstdint>
#include "Typedef.h"

namespace jug
{

namespace string_hasher_detail
{
    constexpr uint64_t kFnv1a64Seed  = 0xcbf29ce484222325;
    constexpr uint64_t kFnv1a64Prime = 0x100000001b3;

    [[nodiscard]] constexpr uint64_t Fnv1a64(
        const char*  _str,
        const size_t _length)
    {
        uint64_t hash = kFnv1a64Seed;
        for (size_t i = 0; i < _length; ++i)
        {
            hash ^= static_cast<uint64_t>(_str[i]);
            hash *= kFnv1a64Prime;
        }
        return hash;
    }
}   // namespace string_hasher_detail

// ===========================================
//  Constexpr String Hasher
// ===========================================

[[nodiscard]] constexpr uint64_t HashString(
    const StringView _str)
{
    return string_hasher_detail::Fnv1a64(_str.data(), _str.size());
}

}   // namespace jug