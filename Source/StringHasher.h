#pragma once
#include <cstdint>
#include <string_view>

namespace jug
{

class Fnv1a32
{
public:
    explicit constexpr Fnv1a32(
        const uint32_t _seed = 0x811c9dc5)
        : m_hash(_seed)
    {
    }

    constexpr void Mix(
        const std::string_view _str)
    {
        for (const char c: _str)
        {
            m_hash ^= static_cast<uint32_t>(c);
            m_hash *= 0x01000193;
        }
    }

    [[nodiscard]] constexpr uint32_t GetHash() const
    {
        return m_hash;
    }

private:
    uint32_t m_hash;
};

class Fnv1a64
{
public:
    explicit constexpr Fnv1a64(
        const uint64_t _seed = 0xcbf29ce484222325)
        : m_hash(_seed)
    {
    }

    constexpr void Mix(
        const std::string_view _str)
    {
        for (const char c: _str)
        {
            m_hash ^= static_cast<uint64_t>(c);
            m_hash *= 0x100000001b3;
        }
    }

    [[nodiscard]] constexpr uint64_t GetHash() const
    {
        return m_hash;
    }

private:
    uint64_t m_hash;
};

}   // namespace jug