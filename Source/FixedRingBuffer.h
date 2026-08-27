// 고정 크기 링 버퍼. 복사/이동/initializer_list/iterator를 모두 지원
#pragma once
#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>

#include "Config.h"

namespace jug
{

template<typename T, size_t N>
    requires(N > 0)
class FixedRingBuffer
{
    // !std::is_default_constructible_v<T>를 만족하지 않는 타입을 위해 union을 사용
    union ITEM
    {
        T value;
    };

public:
    // ===========================================
    //  Iterator
    // ===========================================

    template<bool kbIsConst>
    class BaseIterator
    {
        template<bool>
        friend class BaseIterator;
        friend class FixedRingBuffer;

        using ValueT     = std::conditional_t<kbIsConst, const T, T>;
        using ContainerT = std::conditional_t<kbIsConst, const FixedRingBuffer, FixedRingBuffer>;

    public:
        BaseIterator() = default;

        [[nodiscard]] ValueT& operator*() const
        {
            return m_pContainer->m_items[m_index].value;
        }

        [[nodiscard]] ValueT* operator->() const
        {
            return std::addressof(m_pContainer->m_items[m_index].value);
        }

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pContainer, "Iterator is not initialized.");
            m_index = (m_index + 1) % N;
            return *this;
        }

        BaseIterator operator++(
            const int)
        {
            BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(
            const BaseIterator& _other) const
        {
            return m_pContainer == _other.m_pContainer && m_index == _other.m_index;
        }

        bool operator!=(
            const BaseIterator& _other) const
        {
            return !(*this == _other);
        }

        [[nodiscard]] operator BaseIterator<true>() const
            requires(!kbIsConst)
        {
            return BaseIterator<true> { m_pContainer, m_index };
        }

    protected:
        constexpr BaseIterator(
            ContainerT*  _pOwner,
            const size_t _index)
            : m_pContainer(_pOwner)
            , m_index(_index)
        {
            JUG_ASSERT(_pOwner, "FixedRingBuffer must not be null.");
        }

        ContainerT* m_pContainer = nullptr;
        size_t      m_index      = 0;
    };

    using Iterator      = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    FixedRingBuffer() = default;

    FixedRingBuffer(
        std::initializer_list<T> _list)
    {
        JUG_ASSERT(_list.size() <= GetMaxSize(), "initializer_list size exceeds capacity.");
        for (const T& value: _list)
        {
            Emplace(value);
        }
    }

    FixedRingBuffer(
        const FixedRingBuffer& _other)
        : m_front(_other.m_front)
        , m_back(_other.m_back)
    {
        for (size_t i = 0; i < _other.GetSize(); ++i)
        {
            const size_t index = (m_front + i) % N;
            std::construct_at(std::addressof(m_items[index].value), _other.m_items[index].value);
        }
    }

    FixedRingBuffer(
        FixedRingBuffer&& _other) noexcept
        : m_front(_other.m_front)
        , m_back(_other.m_back)
    {
        for (size_t i = 0; i < _other.GetSize(); ++i)
        {
            const size_t index = (m_front + i) % N;
            std::construct_at(std::addressof(m_items[index].value), std::move(_other.m_items[index].value));
            std::destroy_at(std::addressof(_other.m_items[index].value));
        }
        _other.m_front = 0;
        _other.m_back  = 0;
    }

    ~FixedRingBuffer()
    {
        Clear();
    }

    FixedRingBuffer& operator=(
        std::initializer_list<T> _list)
    {
        JUG_ASSERT(_list.size() <= GetMaxSize(), "initializer_list size exceeds capacity.");

        Clear();
        for (const T& value: _list)
        {
            Emplace(value);
        }

        return *this;
    }

    FixedRingBuffer& operator=(
        const FixedRingBuffer& _other)
    {
        if (this != &_other)
        {
            Clear();

            m_front = _other.m_front;
            m_back  = _other.m_back;

            for (size_t i = 0; i < _other.GetSize(); ++i)
            {
                const size_t index = (m_front + i) % N;
                std::construct_at(std::addressof(m_items[index].value), _other.m_items[index].value);
            }
        }

        return *this;
    }

    FixedRingBuffer& operator=(
        FixedRingBuffer&& _other) noexcept
    {
        if (this != &_other)
        {

            Clear();

            m_front = std::exchange(_other.m_front, 0);
            m_back  = std::exchange(_other.m_back, 0);

            for (size_t i = 0; i < _other.GetSize(); ++i)
            {
                const size_t index = (m_front + i) % N;
                std::construct_at(std::addressof(m_items[index].value), std::move(_other.m_items[index].value));
                std::destroy_at(std::addressof(_other.m_items[index].value));
            }
        }

        return *this;
    }

    void Push(
        const T& _value)
    {
        Emplace(_value);
    }

    void Push(
        T&& _value)
    {
        Emplace(std::move(_value));
    }

    template<typename... TArgs>
    T& Emplace(TArgs&&... _args)
    {
        JUG_ASSERT(!IsFull(), "FixedRingBuffer is full.");
        ITEM& item = m_items[m_back];
        std::construct_at(std::addressof(item.value), std::forward<TArgs>(_args)...);
        m_back = (m_back + 1) % N;
        return item.value;
    }

    [[nodiscard]] T& Front()
    {
        JUG_ASSERT(!IsEmpty(), "FixedRingBuffer is empty.");
        return m_items[m_front].value;
    }

    [[nodiscard]] const T& Front() const
    {
        JUG_ASSERT(!IsEmpty(), "FixedRingBuffer is empty.");
        return m_items[m_front].value;
    }

    void Pop()
    {
        JUG_ASSERT(!IsEmpty(), "FixedRingBuffer is empty.");
        std::destroy_at(std::addressof(m_items[m_front].value));
        m_front = (m_front + 1) % N;
    }

    void Clear()
    {
        for (size_t i = 0; i < GetSize(); ++i)
        {
            const size_t index = (m_front + i) % N;
            std::destroy_at(std::addressof(m_items[index].value));
        }
        m_front = 0;
        m_back  = 0;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_front == m_back;
    }

    [[nodiscard]] bool IsFull() const
    {
        return (m_back + 1) % N == m_front;
    }

    [[nodiscard]] size_t GetSize() const
    {
        return (m_back + N - m_front) % N;
    }

    [[nodiscard]] size_t GetMaxSize() const
    {
        return N - 1;
    }

    [[nodiscard]] Iterator Begin()
    {
        return Iterator { this, m_front };
    }

    [[nodiscard]] Iterator End()
    {
        return Iterator { this, m_back };
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return ConstIterator { this, m_front };
    }

    [[nodiscard]] ConstIterator End() const
    {
        return ConstIterator { this, m_back };
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return ConstIterator { this, m_front };
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return ConstIterator { this, m_back };
    }

    // ===========================================
    //  STL Like
    // ===========================================

    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] size_t size() const
    {
        return GetSize();
    }

    [[nodiscard]] bool empty() const
    {
        return IsEmpty();
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
    ITEM   m_items[N];
    size_t m_front = 0;
    size_t m_back  = 0;
};

}   // namespace jug