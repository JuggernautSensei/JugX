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
//  Shortcut
// ===========================================

template<typename T>
concept MemoryHasherT = requires(T _hasher, MemoryView _mem) {
    { _hasher.Mix(_mem) } -> std::same_as<void>;
    { _hasher.Finalize() } -> std::same_as<uint64_t>;
};

template<MemoryHasherT H>
[[nodiscard]] uint64_t Hash(
    const MemoryView _mem)
{
    H hasher {};
    hasher.Mix(_mem);
    return hasher.Finalize();
}

template<MemoryHasherT H>
[[nodiscard]] uint64_t Hash(
    const MemoryView _mem,
    const uint64_t   _seed)
{
    H hasher { _seed };
    hasher.Mix(_mem);
    return hasher.Finalize();
}

}   // namespace jug
