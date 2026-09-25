#pragma once
#include "Math.h"
#include "Tag.h"

namespace jug
{

template<typename T, typename Alloc = std::allocator<T>>
class Buffer
{
    JUG_CLASS(Buffer, NO_COPY)

public:
    using Iterator      = T*;
    using ConstIterator = const T*;

    Buffer() = default;

    explicit Buffer(
        const size_t _size,
        const Alloc& _alloc = Alloc {})
        requires(std::is_default_constructible_v<T>)
        : m_alloc(_alloc)
    {
        Resize(_size);
    }

    Buffer(
        const size_t _size,
        const T&     _value,
        const Alloc& _alloc = Alloc {})
        requires(std::is_copy_constructible_v<T>)
        : m_alloc(_alloc)
    {
        Resize(_size, _value);
    }

    // 메모리 미 초기화 모델. 미초기화된 상태로 소멸시 UB의 위험이 있으며, 호출자에게 책임이 있음.
    Buffer(
        const NoInitType,
        const size_t _size,
        const Alloc& _alloc = Alloc {})
        : m_alloc(_alloc)
    {
        ResizeUninitialized(_size);
    }

    Buffer(
        Buffer&& _other) noexcept
        : m_alloc(std::move(_other.m_alloc))
        , m_pData(std::exchange(_other.m_pData, nullptr))
        , m_size(std::exchange(_other.m_size, 0))
        , m_capacity(std::exchange(_other.m_capacity, 0))
    {
    }

    Buffer& operator=(
        Buffer&& _other) noexcept
    {
        if (this != &_other)
        {
            Reset();
            m_alloc    = std::move(_other.m_alloc);
            m_pData    = std::exchange(_other.m_pData, nullptr);
            m_size     = std::exchange(_other.m_size, 0);
            m_capacity = std::exchange(_other.m_capacity, 0);
        }
        return *this;
    }

    ~Buffer()
    {
        Reset();
    }

    void Resize(
        const size_t _size,
        const T&     _value = T {})
    {
        const size_t old = m_size;
        ResizeUninitialized(_size);
        if (_size > old)
        {
            std::uninitialized_fill_n(m_pData + old, _size - old, _value);
        }
    }

    // 메모리 미 초기화 모델. 미초기화된 상태로 소멸시 UB의 위험이 있으며, 호출자에게 책임이 있음.
    void ResizeUninitialized(
        const size_t _size)
    {
        if (_size < m_size)
        {
            std::destroy_n(m_pData + _size, m_size - _size);
            m_size = _size;
        }
        else if (_size > m_size)
        {
            if (m_capacity < _size)   // realloc
            {
                Grow_(_size);
            }
            m_size = _size;
        }
    }

    void Reset()
    {
        if (m_pData)
        {
            std::destroy_n(m_pData, m_size);
            m_alloc.deallocate(m_pData, m_capacity);
            m_pData    = nullptr;
            m_size     = 0;
            m_capacity = 0;
        }
    }

    T& operator[](
        const size_t _index)
    {
        JUG_ASSERT(_index < m_size, "Index out of bounds");
        return m_pData[_index];
    }

    const T& operator[](
        const size_t _index) const
    {
        JUG_ASSERT(_index < m_size, "Index out of bounds");
        return m_pData[_index];
    }

    [[nodiscard]] T* GetPtr()
    {
        return m_pData;
    }

    [[nodiscard]] const T* GetPtr() const
    {
        return m_pData;
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_size;
    }

    [[nodiscard]] size_t GetCapacity() const
    {
        return m_capacity;
    }

    [[nodiscard]] Iterator Begin()
    {
        return m_pData;
    }

    [[nodiscard]] Iterator End()
    {
        return m_pData + m_size;
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return m_pData;
    }

    [[nodiscard]] ConstIterator End() const
    {
        return m_pData + m_size;
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return m_pData;
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return m_pData + m_size;
    }

    // ===========================================
    //  STL like
    // ===========================================

    using value_type     = T;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] value_type* data()
    {
        return GetPtr();
    }

    [[nodiscard]] const value_type* data() const
    {
        return GetPtr();
    }

    [[nodiscard]] iterator begin()
    {
        return Begin();
    }

    [[nodiscard]] iterator end()
    {
        return End();
    }

    [[nodiscard]] const_iterator begin() const
    {
        return Begin();
    }

    [[nodiscard]] const_iterator end() const
    {
        return End();
    }

    [[nodiscard]] const_iterator cbegin() const
    {
        return CBegin();
    }

    [[nodiscard]] const_iterator cend() const
    {
        return CEnd();
    }

private:
    void Grow_(
        const size_t _newCapacity)
    {
        JUG_ASSERT(_newCapacity > m_capacity, "New capacity must be greater than current capacity");

        T* pMem = m_alloc.allocate(_newCapacity);
        if (m_pData)
        {
            std::uninitialized_move_n(m_pData, m_size, pMem);
            std::destroy_n(m_pData, m_size);
            m_alloc.deallocate(m_pData, m_capacity);
        }
        m_pData    = pMem;
        m_capacity = _newCapacity;
    }

    [[no_unique_address]] Alloc m_alloc    = {};
    T*                          m_pData    = nullptr;
    size_t                      m_size     = 0;
    size_t                      m_capacity = 0;
};

}   // namespace jug