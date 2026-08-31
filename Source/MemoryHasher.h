#pragma once
#include "MemoryView.h"

namespace jug
{

// ===========================================
//  Fnv1a
// ===========================================

class Fnv1a
{
public:
    explicit Fnv1a(uint64_t _seed = 0xcbf29ce484222325);
    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint64_t Finalize() const;

private:
    uint64_t m_hash = 0;
};

// ===========================================
//  Murmur3
// ===========================================

class Murmur3
{
public:
    explicit Murmur3(uint64_t _seed = 0x9747b28c);
    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint64_t Finalize() const;

private:
    uint64_t m_hash   = 0;
    size_t   m_length = 0;
};

// ===========================================
//  Utils
// ===========================================

template<typename T>
concept MemoryHasherT = requires(T _hasher, MemoryView _mem) {
    { _hasher.Mix(_mem) } -> std::same_as<void>;
    { _hasher.Finalize() } -> std::same_as<uint64_t>;
};

template<MemoryHasherT THasher>
[[nodiscard]] uint64_t Hash(
    const MemoryView _mem)
{
    THasher hasher {};
    hasher.Mix(_mem);
    return hasher.Finalize();
}

template<MemoryHasherT THasher>
[[nodiscard]] uint64_t Hash(
    const MemoryView _mem,
    const uint64_t   _seed)
{
    THasher hasher { _seed };
    hasher.Mix(_mem);
    return hasher.Finalize();
}

}   // namespace jug
