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
    [[nodiscard]] uint32_t Rotl32_(
        const uint32_t x,
        const int8_t   r)
    {
        return (x << r) | (x >> (32 - r));
    }

    [[nodiscard]] uint64_t Rotl64_(
        const uint64_t x,
        const int8_t   r)
    {
        return (x << r) | (x >> (64 - r));
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
    constexpr uint64_t k_c1 = 0x87c37b91114253d5ULL;
    constexpr uint64_t k_c2 = 0x4cf5ad432745937fULL;

    const auto*    pByte      = reinterpret_cast<const uint8_t*>(_mem.data());
    const size_t   byteWidth  = _mem.GetSize();
    const uint32_t blockCount = static_cast<uint32_t>(byteWidth / 8);

    m_length += byteWidth;

    const auto* pBlock = reinterpret_cast<const uint64_t*>(pByte);
    for (uint32_t i = 0; i < blockCount; ++i)
    {
        uint64_t k1;
        std::memcpy(&k1, pBlock + i, sizeof(uint64_t));

        k1 *= k_c1;
        k1 = Rotl64_(k1, 31);
        k1 *= k_c2;

        m_hash ^= k1;
        m_hash = Rotl64_(m_hash, 27);
        m_hash = m_hash * 5 + 0x52dce729;
    }

    const uint8_t* pTail = pByte + static_cast<size_t>(blockCount) * 8;
    uint64_t       k1    = 0;

    switch (byteWidth & 7)   // NOLINT
    {
        case 7:
            k1 ^= static_cast<uint64_t>(pTail[6]) << 48;
            [[fallthrough]];
        case 6:
            k1 ^= static_cast<uint64_t>(pTail[5]) << 40;
            [[fallthrough]];
        case 5:
            k1 ^= static_cast<uint64_t>(pTail[4]) << 32;
            [[fallthrough]];
        case 4:
            k1 ^= static_cast<uint64_t>(pTail[3]) << 24;
            [[fallthrough]];
        case 3:
            k1 ^= static_cast<uint64_t>(pTail[2]) << 16;
            [[fallthrough]];
        case 2:
            k1 ^= static_cast<uint64_t>(pTail[1]) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= static_cast<uint64_t>(pTail[0]);
            k1 *= k_c1;
            k1 = Rotl64_(k1, 31);
            k1 *= k_c2;
            m_hash ^= k1;

            // [AI] 블록 경로와 똑같이 해시를 전진시킨다. 정통 Murmur3 는 tail 을 맨 끝에 한 번만
            //      돌리므로 전진이 필요 없지만, 이 클래스는 Mix 를 연달아 부르는 스트리밍이라
            //      tail 이 중간에 반복된다. 전진이 없으면 8바이트 미만 값들이 순수 XOR 로 합쳐져
            //      교환법칙이 성립한다 -> Mix(a),Mix(b) 와 Mix(b),Mix(a) 가 같은 해시가 된다.
            m_hash = Rotl64_(m_hash, 27);
            m_hash = m_hash * 5 + 0x52dce729;
    }
}

uint64_t Murmur3::Finalize() const
{
    uint64_t h1 = m_hash;
    h1 ^= m_length;
    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;
    return h1;
}

}   // namespace jug