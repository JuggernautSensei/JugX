#pragma once

namespace jug
{

namespace string_hasher_detail
{
    constexpr uint64_t kFnv1a64Seed  = 0xcbf29ce484222325;
    constexpr uint64_t kFnv1a64Prime = 0x100000001b3;
}   // namespace string_hasher_detail

// ===========================================
//  Constexpr String Hasher
// ===========================================

[[nodiscard]] constexpr uint64_t HashString(
    const StringView _str)
{
    uint64_t hash = string_hasher_detail::kFnv1a64Seed;
    for (const char c: _str)
    {
        hash ^= static_cast<uint64_t>(c);
        hash *= string_hasher_detail::kFnv1a64Prime;
    }
    return hash;
}

}   // namespace jug