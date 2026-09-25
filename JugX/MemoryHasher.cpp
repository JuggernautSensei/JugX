#include "pch.h"
#include "MemoryHasher.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "MemoryView.h"

namespace jug
{

namespace
{
    constexpr uint64_t k_murmur3C1 = 0x87c37b91114253d5ULL;
    constexpr uint64_t k_murmur3C2 = 0x4cf5ad432745937fULL;

    [[nodiscard]] uint64_t Rotl64_(
        const uint64_t x,
        const int8_t   r)
    {
        return (x << r) | (x >> (64 - r));
    }

    void MixBlock64_(
        uint64_t&      _hash,
        const uint64_t _k1)
    {
        uint64_t k1 = _k1;
        k1 *= k_murmur3C1;
        k1 = Rotl64_(k1, 31);
        k1 *= k_murmur3C2;

        _hash ^= k1;
        _hash = Rotl64_(_hash, 27);
        _hash = _hash * 5 + 0x52dce729;
    }
}   // namespace

Fnv1a::Fnv1a(
    const uint64_t _seed)
    : m_hash(_seed)
{
}

void Fnv1a::Mix(
    const MemoryView _mem)
{
    for (const std::byte b: _mem)
    {
        m_hash ^= static_cast<uint64_t>(b);
        m_hash *= 0x100000001b3;
    }
}

uint64_t Fnv1a::Finalize() const
{
    return m_hash;
}

Murmur3::Murmur3(
    const uint64_t _seed)
    : m_hash(_seed)
{
}

void Murmur3::Mix(
    const MemoryView _mem)
{
    const std::byte* pByte     = _mem.data();
    size_t           byteWidth = _mem.GetSize();

    m_length += byteWidth;
    while (byteWidth != 0 && (m_tailLen != 0 || byteWidth < 8))
    {
        m_tail[m_tailLen++] = static_cast<uint8_t>(*pByte++);
        --byteWidth;

        if (m_tailLen == 8)
        {
            uint64_t k1;
            std::memcpy(&k1, m_tail, sizeof(uint64_t));
            MixBlock64_(m_hash, k1);
            m_tailLen = 0;
        }
    }

    const uint32_t numBlocks = static_cast<uint32_t>(byteWidth / 8);
    for (uint32_t i = 0; i < numBlocks; ++i)
    {
        uint64_t k1;
        std::memcpy(&k1, pByte + static_cast<size_t>(i) * 8, sizeof(uint64_t));
        MixBlock64_(m_hash, k1);
    }

    pByte += static_cast<size_t>(numBlocks) * 8;
    byteWidth -= static_cast<size_t>(numBlocks) * 8;

    for (size_t i = 0; i < byteWidth; ++i)
    {
        m_tail[m_tailLen++] = static_cast<uint8_t>(pByte[i]);
    }
}

uint64_t Murmur3::Finalize() const
{
    uint64_t h1 = m_hash;
    uint64_t k1 = 0;
    switch (m_tailLen)   // NOLINT
    {
        case 7:
            k1 ^= static_cast<uint64_t>(m_tail[6]) << 48;
            [[fallthrough]];
        case 6:
            k1 ^= static_cast<uint64_t>(m_tail[5]) << 40;
            [[fallthrough]];
        case 5:
            k1 ^= static_cast<uint64_t>(m_tail[4]) << 32;
            [[fallthrough]];
        case 4:
            k1 ^= static_cast<uint64_t>(m_tail[3]) << 24;
            [[fallthrough]];
        case 3:
            k1 ^= static_cast<uint64_t>(m_tail[2]) << 16;
            [[fallthrough]];
        case 2:
            k1 ^= static_cast<uint64_t>(m_tail[1]) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= static_cast<uint64_t>(m_tail[0]);
            MixBlock64_(h1, k1);
    }

    h1 ^= m_length;
    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;
    return h1;
}

}   // namespace jug