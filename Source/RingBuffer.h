#pragma once
#include <iterator>
#include <type_traits>

#include "Alloc.h"
#include "Allocator.h"
#include "Config.h"
#include "Math.h"

namespace jug
{

namespace ring_buffer_detail
{
    constexpr size_t kMinCapacity = 8;
    static_assert(IsPowerOf2(kMinCapacity), "kMinCapacity must be a power of 2.");
}   // namespace ring_buffer_detail

template<typename T>
class RingBuffer
{
public:
    // ===========================================
    //  Iterator
    // ===========================================

    template<bool kbIsConst>
    class BaseIterator
    {
        template<bool>
        friend class BaseIterator;
        friend class RingBuffer;

        using ContainerT = std::conditional_t<kbIsConst, const RingBuffer, RingBuffer>;
        using ValueT     = std::conditional_t<kbIsConst, const T, T>;

    public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = ValueT*;
        using reference         = ValueT&;

        BaseIterator() = default;

        [[nodiscard]] reference operator*() const
        {
            JUG_ASSERT(m_pBuffer && m_index < m_pBuffer->GetSize(), "Cannot dereference an out of range iterator.");
            return *GetPtr_(m_index);
        }

        [[nodiscard]] pointer operator->() const
        {
            JUG_ASSERT(m_pBuffer && m_index < m_pBuffer->GetSize(), "Cannot dereference an out of range iterator.");
            return GetPtr_(m_index);
        }

        [[nodiscard]] reference operator[](
            const difference_type _offset) const
        {
            return *(*this + _offset);
        }

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pBuffer && m_index < m_pBuffer->GetSize(), "Iterator is already at the end. Cannot increment further.");
            ++m_index;
            return *this;
        }

        BaseIterator operator++(
            const int)
        {
            BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        BaseIterator& operator--()
        {
            JUG_ASSERT(m_index > 0, "Iterator is already at the begin. Cannot decrement further.");
            --m_index;
            return *this;
        }

        BaseIterator operator--(
            const int)
        {
            BaseIterator tmp = *this;
            --(*this);
            return tmp;
        }

        BaseIterator& operator+=(
            const difference_type _offset)
        {
            m_index = static_cast<size_t>(static_cast<difference_type>(m_index) + _offset);
            JUG_ASSERT(m_pBuffer && m_index <= m_pBuffer->GetSize(), "Iterator is out of range.");
            return *this;
        }

        BaseIterator& operator-=(
            const difference_type _offset)
        {
            return *this += -_offset;
        }

        [[nodiscard]] BaseIterator operator+(
            const difference_type _offset) const
        {
            BaseIterator tmp = *this;
            return tmp += _offset;
        }

        [[nodiscard]] friend BaseIterator operator+(
            const difference_type _offset,
            const BaseIterator&   _it)
        {
            return _it + _offset;
        }

        [[nodiscard]] BaseIterator operator-(
            const difference_type _offset) const
        {
            BaseIterator tmp = *this;
            return tmp -= _offset;
        }

        [[nodiscard]] difference_type operator-(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pBuffer == _other.m_pBuffer, "Cannot compare iterators from different RingBuffers.");
            return static_cast<difference_type>(m_index) - static_cast<difference_type>(_other.m_index);
        }

        [[nodiscard]] bool operator==(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pBuffer == _other.m_pBuffer, "Cannot compare iterators from different RingBuffers.");
            return m_index == _other.m_index;
        }

        [[nodiscard]] bool operator<(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pBuffer == _other.m_pBuffer, "Cannot compare iterators from different RingBuffers.");
            return m_index < _other.m_index;
        }

        [[nodiscard]] bool operator>(
            const BaseIterator& _other) const
        {
            return _other < *this;
        }

        [[nodiscard]] bool operator<=(
            const BaseIterator& _other) const
        {
            return !(_other < *this);
        }

        [[nodiscard]] bool operator>=(
            const BaseIterator& _other) const
        {
            return !(*this < _other);
        }

        [[nodiscard]] operator BaseIterator<true>() const
            requires(!kbIsConst)
        {
            return BaseIterator<true> { m_pBuffer, m_index };
        }

    private:
        constexpr BaseIterator(
            ContainerT*  _pBuffer,
            const size_t _index)
            : m_pBuffer(_pBuffer)
            , m_index(_index)
        {
            JUG_ASSERT(_pBuffer, "RingBuffer must not be null.");
        }

        [[nodiscard]] pointer GetPtr_(
            const size_t _index) const
        {
            return m_pBuffer->m_pBegin + ((m_pBuffer->m_front + _index) & m_pBuffer->Mask_());
        }

        ContainerT* m_pBuffer = nullptr;
        size_t      m_index   = 0;
    };

    using Iterator      = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    RingBuffer()
    {
        Reserve(ring_buffer_detail::kMinCapacity);
    }

    explicit RingBuffer(
        const size_t _initCap)
    {
        Reserve(_initCap);
    }

    RingBuffer(const RingBuffer& _other)
    {
        Reserve(_other.m_capacity);
        CopyFrom_(_other);
    }

    RingBuffer& operator=(
        const RingBuffer& _other)
    {
        if (this != &_other)
        {
            Clear();
            Reserve(_other.m_capacity);
            CopyFrom_(_other);
        }
        return *this;
    }

    RingBuffer(RingBuffer&& _other) noexcept
        : m_pBegin(std::exchange(_other.m_pBegin, nullptr))
        , m_capacity(std::exchange(_other.m_capacity, 0))
        , m_front(std::exchange(_other.m_front, 0))
        , m_size(std::exchange(_other.m_size, 0))
    {
    }

    RingBuffer& operator=(
        RingBuffer&& _other) noexcept
    {
        if (this != &_other)
        {
            DestroyAll_();
            m_allocator.Free(m_pBegin, m_capacity);

            m_pBegin   = std::exchange(_other.m_pBegin, nullptr);
            m_capacity = std::exchange(_other.m_capacity, 0);
            m_front    = std::exchange(_other.m_front, 0);
            m_size     = std::exchange(_other.m_size, 0);
        }
        return *this;
    }

    ~RingBuffer()
    {
        DestroyAll_();
        m_allocator.Free(m_pBegin, m_capacity);
    }

    template<typename... TArgs>
        requires std::is_constructible_v<T, TArgs...>
    T& Emplace(
        TArgs&&... _args)
    {
        if (m_size == m_capacity)
        {
            if (m_capacity == 0)
            {
                Reserve(ring_buffer_detail::kMinCapacity);
            }
            else
            {
                Pop();
            }
        }

        T* pObj = std::construct_at(GetValue_(m_size), std::forward<TArgs>(_args)...);
        ++m_size;
        return *pObj;
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

    void Pop()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.");
        std::destroy_at(m_pBegin + m_front);
        m_front = (m_front + 1) & Mask_();
        --m_size;
    }

    void Clear()
    {
        DestroyAll_();
        m_front = 0;
        m_size  = 0;
    }

    [[nodiscard]] T& Front()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.");
        return m_pBegin[m_front];
    }

    [[nodiscard]] const T& Front() const
    {
        return const_cast<RingBuffer*>(this)->Front();
    }

    [[nodiscard]] T& Back()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.");
        return *GetValue_(m_size - 1);
    }

    [[nodiscard]] const T& Back() const
    {
        return const_cast<RingBuffer*>(this)->Back();
    }

    [[nodiscard]] T& operator[](
        const size_t _index)
    {
        JUG_ASSERT(_index < m_size, "RingBuffer index out of range.");
        return *GetValue_(_index);
    }

    [[nodiscard]] const T& operator[](
        const size_t _index) const
    {
        return const_cast<RingBuffer*>(this)->operator[](_index);
    }

    [[nodiscard]] T* GetPtr()
    {
        return m_pBegin;
    }

    [[nodiscard]] const T* GetPtr() const
    {
        return m_pBegin;
    }

    void Reserve(
        const size_t _cap)
    {
        if (_cap <= m_capacity)
        {
            return;
        }

        Reallocate_(ClacIdealCapacity_(_cap));
    }

    void SetCapacity(
        const size_t _cap)
    {
        const size_t cap = ClacIdealCapacity_(_cap);
        if (cap == m_capacity)
        {
            return;
        }

        Reallocate_(cap);
    }

    void Resize(
        const size_t _size)
        requires std::is_default_constructible_v<T>
    {
        JUG_ASSERT(_size <= m_capacity, "Resize does not change the capacity. Call Reserve or SetCapacity first.");

        while (m_size > _size)
        {
            Pop();
        }

        while (m_size < _size)
        {
            std::construct_at(GetValue_(m_size));
            ++m_size;
        }
    }

    void Resize(
        const size_t _size,
        const T&     _value)
        requires std::is_copy_constructible_v<T>
    {
        JUG_ASSERT(_size <= m_capacity, "Resize does not change the capacity. Call Reserve or SetCapacity first.");

        while (m_size > _size)
        {
            Pop();
        }

        while (m_size < _size)
        {
            std::construct_at(GetValue_(m_size), _value);
            ++m_size;
        }
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_size;
    }

    [[nodiscard]] size_t GetCapacity() const
    {
        return m_capacity;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_size == 0;
    }

    [[nodiscard]] bool IsFull() const
    {
        return m_size == m_capacity;
    }

    [[nodiscard]] Iterator Begin()
    {
        return Iterator { this, 0 };
    }

    [[nodiscard]] Iterator End()
    {
        return Iterator { this, m_size };
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return ConstIterator { this, 0 };
    }

    [[nodiscard]] ConstIterator End() const
    {
        return ConstIterator { this, m_size };
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return ConstIterator { this, 0 };
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return ConstIterator { this, m_size };
    }

    // ==========================================
    //  STL like
    // ==========================================

    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] size_t size() const
    {
        return GetSize();
    }

    [[nodiscard]] size_t capacity() const
    {
        return GetCapacity();
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
    [[nodiscard]] size_t Mask_() const
    {
        JUG_ASSERT(m_capacity > 0, "RingBuffer has no storage.");
        return m_capacity - 1;
    }

    [[nodiscard]] T* GetValue_(
        const size_t _index) const
    {
        return m_pBegin + ((m_front + _index) & Mask_());
    }

    [[nodiscard]] static size_t ClacIdealCapacity_(
        const size_t _cap)
    {
        return _cap == 0 ? 0 : CeilPowerOf2(Max(_cap, ring_buffer_detail::kMinCapacity));
    }

    void DestroyAll_()
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_t i = 0; i < m_size; ++i)
            {
                std::destroy_at(GetValue_(i));
            }
        }
    }

    void CopyFrom_(
        const RingBuffer& _other)
    {
        JUG_ASSERT(IsEmpty(), "Destination must be empty before copying.");
        JUG_ASSERT(_other.m_size <= m_capacity, "Destination buffer is not large enough for source data.");

        for (size_t i = 0; i < _other.m_size; ++i)
        {
            std::construct_at(m_pBegin + i, _other[i]);
        }

        m_front = 0;
        m_size  = _other.m_size;
    }

    void Reallocate_(
        const size_t _cap)
    {
        JUG_ASSERT(_cap == 0 || IsPowerOf2(_cap), "capacity must be a power of 2.");

        T* pMem = _cap > 0 ? m_allocator.Alloc(_cap) : nullptr;

        const size_t numDropped = m_size > _cap ? m_size - _cap : 0;
        const size_t numMoved   = m_size - numDropped;
        for (size_t i = 0; i < numMoved; ++i)
        {
            std::construct_at(pMem + i, std::move(*GetValue_(numDropped + i)));
        }

        DestroyAll_();
        m_allocator.Free(m_pBegin, m_capacity);

        m_pBegin   = pMem;
        m_capacity = _cap;
        m_front    = 0;
        m_size     = numMoved;
    }

    T*     m_pBegin   = nullptr;
    size_t m_capacity = 0;   // 항상 2의 거듭제곱 또는 0
    size_t m_front    = 0;
    size_t m_size     = 0;

    [[no_unique_address]] Allocator<T> m_allocator = {};
};

}   // namespace jug
