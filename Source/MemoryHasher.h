#pragma once
#include "Vendor/xxHash/xxhash.h"
#include "MemoryView.h"

namespace jug
{

class XXH32
{
public:
    explicit XXH32(uint32_t _seed = 0);
    ~XXH32();

    XXH32(const XXH32&)            = delete;
    XXH32& operator=(const XXH32&) = delete;
    XXH32(XXH32&& _other) noexcept;
    XXH32& operator=(XXH32&& _other) noexcept;

    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint32_t Finalize() const;
    [[nodiscard]] uint32_t GetHash() const;

private:
    XXH32_state_t* m_pState = nullptr;
};

class XXH64
{
public:
    explicit XXH64(uint64_t _seed = 0);
    ~XXH64();

    XXH64(const XXH64&)            = delete;
    XXH64& operator=(const XXH64&) = delete;
    XXH64(XXH64&& _other) noexcept;
    XXH64& operator=(XXH64&& _other) noexcept;

    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint64_t Finalize() const;
    [[nodiscard]] uint64_t GetHash() const;

private:
    XXH64_state_t* m_pState = nullptr;
};

class Murmur3_32
{
public:
    explicit Murmur3_32(uint32_t _seed = 0x9747b28c);
    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint32_t Finalize() const;
    [[nodiscard]] uint32_t GetHash() const;

private:
    uint32_t m_hash   = 0;
    uint32_t m_length = 0;
};

class Murmur3_64
{
public:
    explicit Murmur3_64(uint64_t _seed = 0x9747b28c);
    void                   Mix(MemoryView _mem);
    [[nodiscard]] uint64_t Finalize() const;
    [[nodiscard]] uint64_t GetHash() const;

private:
    uint64_t m_hash   = 0;
    uint32_t m_length = 0;
};

}   // namespace jug