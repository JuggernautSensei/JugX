#pragma once
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>

#include "MemoryView.h"

namespace jug
{

template<typename Delete = std::default_delete<std::byte[]>>
class BaseMemory
{
    JUG_CLASS(BaseMemory, NO_COPY)

public:
    using Iterator      = std::byte*;
    using ConstIterator = const std::byte*;

    BaseMemory() = default;

    BaseMemory(
        std::byte*   _pMem,
        const size_t _size)
        : m_pMem(_pMem)
        , m_size(_size)
    {
    }

    BaseMemory(
        std::byte*    _pMem,
        const size_t  _size,
        const Delete& _deletor)
        : m_pMem(_pMem)
        , m_size(_size)
        , m_deletor(_deletor)
    {
    }

    BaseMemory(
        std::byte*   _pMem,
        const size_t _size,
        Delete&&     _deletor)
        : m_pMem(_pMem)
        , m_size(_size)
        , m_deletor(std::move(_deletor))
    {
    }

    BaseMemory(
        BaseMemory&& _other) noexcept
        : m_pMem(std::exchange(_other.m_pMem, nullptr))
        , m_size(std::exchange(_other.m_size, 0))
        , m_deletor(std::move(_other.m_deletor))
    {
    }

    BaseMemory& operator=(
        BaseMemory&& _other) noexcept
    {
        if (this != &_other)
        {
            Reset();
            m_pMem    = std::exchange(_other.m_pMem, nullptr);
            m_size    = std::exchange(_other.m_size, 0);
            m_deletor = std::move(_other.m_deletor);
        }
        return *this;
    }

    ~BaseMemory()
    {
        Reset();
    }

    [[nodiscard]] std::byte* GetPtr()
    {
        return m_pMem;
    }

    [[nodiscard]] const std::byte* GetPtr() const
    {
        return m_pMem;
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_size;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_size == 0;
    }

    void Reset()
    {
        if (m_pMem)
        {
            m_deletor(m_pMem);
            m_pMem = nullptr;
            m_size = 0;
        }
    }

    [[nodiscard]] Iterator Begin()
    {
        return m_pMem;
    }

    [[nodiscard]] Iterator End()
    {
        return m_pMem + m_size;
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return m_pMem;
    }

    [[nodiscard]] ConstIterator End() const
    {
        return m_pMem + m_size;
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return m_pMem;
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return m_pMem + m_size;
    }

    explicit operator bool() const
    {
        return m_pMem;
    }

    // ======================================
    //  STL Like
    // ======================================

    using value_type     = std::byte;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] std::byte* data()
    {
        return m_pMem;
    }

    [[nodiscard]] const std::byte* data() const
    {
        return m_pMem;
    }

    [[nodiscard]] iterator begin()
    {
        return m_pMem;
    }

    [[nodiscard]] iterator end()
    {
        return m_pMem + m_size;
    }

    [[nodiscard]] const_iterator begin() const
    {
        return m_pMem;
    }

    [[nodiscard]] const_iterator end() const
    {
        return m_pMem + m_size;
    }

    [[nodiscard]] const_iterator cbegin() const
    {
        return m_pMem;
    }

    [[nodiscard]] const_iterator cend() const
    {
        return m_pMem + m_size;
    }

private:
    std::byte* m_pMem = nullptr;
    size_t     m_size = 0;

    [[no_unique_address]] Delete m_deletor = Delete {};
};

using Memory = BaseMemory<>;

[[nodiscard]] Memory AllocMemory(size_t _size);

}   // namespace jug