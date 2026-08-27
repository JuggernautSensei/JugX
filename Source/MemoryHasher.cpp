#include "MemoryHasher.h"

namespace jug
{

XXH32::XXH32(
    const uint32_t _seed)
    : m_pState(XXH32_createState())
{
    XXH32_reset(m_pState, _seed);
}

XXH32::~XXH32()
{
    XXH32_freeState(m_pState);
}

XXH32::XXH32(
    XXH32&& _other) noexcept
    : m_pState(_other.m_pState)
{
    _other.m_pState = nullptr;
}

XXH32& XXH32::operator=(
    XXH32&& _other) noexcept
{
    if (this != &_other)
    {
        XXH32_freeState(m_pState);
        m_pState        = _other.m_pState;
        _other.m_pState = nullptr;
    }
    return *this;
}

void XXH32::Mix(   // NOLINT
    const MemoryView _mem)
{
    XXH32_update(m_pState, _mem.data(), _mem.size());
}

uint32_t XXH32::Finalize() const
{
    return XXH32_digest(m_pState);
}

uint32_t XXH32::GetHash() const
{
    return XXH32_digest(m_pState);
}

XXH64::XXH64(
    const uint64_t _seed)
    : m_pState(XXH64_createState())
{
    XXH64_reset(m_pState, _seed);
}

XXH64::~XXH64()
{
    XXH64_freeState(m_pState);
}

XXH64::XXH64(
    XXH64&& _other) noexcept
    : m_pState(_other.m_pState)
{
    _other.m_pState = nullptr;
}

XXH64& XXH64::operator=(
    XXH64&& _other) noexcept
{
    if (this != &_other)
    {
        XXH64_freeState(m_pState);
        m_pState        = _other.m_pState;
        _other.m_pState = nullptr;
    }
    return *this;
}

void XXH64::Mix(   // NOLINT
    const MemoryView _mem)
{
    XXH64_update(m_pState, _mem.data(), _mem.size());
}

uint64_t XXH64::Finalize() const
{
    return XXH64_digest(m_pState);
}

uint64_t XXH64::GetHash() const
{
    return XXH64_digest(m_pState);
}

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

Murmur3_32::Murmur3_32(
    const uint32_t _seed)
    : m_hash(_seed)
{
}

void Murmur3_32::Mix(
    const MemoryView _mem)
{
    constexpr uint32_t k_c1 = 0xcc9e2d51;
    constexpr uint32_t k_c2 = 0x1b873593;

    const auto*    pByte      = reinterpret_cast<const uint8_t*>(_mem.data());
    const size_t   byteWidth  = _mem.size();
    const uint32_t blockCount = static_cast<uint32_t>(byteWidth / 4);

    m_length += static_cast<uint32_t>(byteWidth);

    const auto* pBlock = reinterpret_cast<const uint32_t*>(pByte);
    for (uint32_t i = 0; i < blockCount; ++i)
    {
        uint32_t k1;
        std::memcpy(&k1, pBlock + i, sizeof(uint32_t));

        k1 *= k_c1;
        k1 = Rotl32_(k1, 15);
        k1 *= k_c2;

        m_hash ^= k1;
        m_hash = Rotl32_(m_hash, 13);
        m_hash = m_hash * 5 + 0xe6546b64;
    }

    const uint8_t* pTail = pByte + static_cast<size_t>(blockCount) * 4;
    uint32_t       k1    = 0;

    switch (byteWidth & 3)   // NOLINT
    {
        case 3:
            k1 ^= static_cast<uint32_t>(pTail[2]) << 16;
            [[fallthrough]];
        case 2:
            k1 ^= static_cast<uint32_t>(pTail[1]) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= static_cast<uint32_t>(pTail[0]);
            k1 *= k_c1;
            k1 = Rotl32_(k1, 15);
            k1 *= k_c2;
            m_hash ^= k1;
    }
}

uint32_t Murmur3_32::Finalize() const
{
    uint32_t h1 = m_hash;
    h1 ^= m_length;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
}

uint32_t Murmur3_32::GetHash() const
{
    return m_hash;
}

Murmur3_64::Murmur3_64(
    const uint64_t _seed)
    : m_hash(_seed)
{
}

void Murmur3_64::Mix(
    const MemoryView _mem)
{
    constexpr uint64_t k_c1 = 0x87c37b91114253d5ULL;
    constexpr uint64_t k_c2 = 0x4cf5ad432745937fULL;

    const auto*    pByte      = reinterpret_cast<const uint8_t*>(_mem.data());
    const size_t   byteWidth  = _mem.size();
    const uint32_t blockCount = static_cast<uint32_t>(byteWidth / 8);

    m_length += static_cast<uint32_t>(byteWidth);

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
    }
}

uint64_t Murmur3_64::Finalize() const
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

uint64_t Murmur3_64::GetHash() const
{
    return m_hash;
}

}   // namespace jug