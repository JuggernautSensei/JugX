#pragma once
#include "Math.h"

namespace jug
{

namespace ring_buffer_detail
{
    constexpr size_t kDefaultCapacity = 8;
}

template<typename T, typename Alloc = Allocator<T>>
class RingBuffer
{
    static_assert(std::is_copy_constructible_v<T>, "RingBuffer requires copy constructible type.");
    static_assert(std::is_move_constructible_v<T>, "RingBuffer requires move constructible type.");

public:
    // ===========================================
    //  Iterator
    // ===========================================

    template<bool kbConst>
    class BaseIterator
    {
        template<bool>
        friend class BaseIterator;
        friend class RingBuffer;

        using ContainerT = std::conditional_t<kbConst, const RingBuffer, RingBuffer>;
        using ValueT     = std::conditional_t<kbConst, const T, T>;

    public:
        BaseIterator() = default;

        // ===========================================
        //  Access
        // ===========================================

        [[nodiscard]] ValueT& operator*() const
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            return m_pContainer->GetValue_(m_index);
        }

        [[nodiscard]] ValueT* operator->() const
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            return std::addressof(m_pContainer->GetValue_(m_index));
        }

        // ===========================================
        //  Increment/Decrement
        // ===========================================

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            m_index = Min(m_index + 1, m_pContainer->GetSize());
            return *this;
        }

        [[nodiscard]] BaseIterator operator++(
            int)
        {
            BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        BaseIterator& operator--()
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            m_index = (m_index > 0) ? m_index - 1 : 0;
            return *this;
        }

        [[nodiscard]] BaseIterator operator--(
            int)
        {
            BaseIterator tmp = *this;
            --(*this);
            return tmp;
        }

        // ===========================================
        //  Random Access
        // ===========================================

        BaseIterator& operator+=(
            const ptrdiff_t _offset)
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            m_index = static_cast<size_t>(Clamp<ptrdiff_t>(static_cast<ptrdiff_t>(m_index) + _offset, 0, static_cast<ptrdiff_t>(m_pContainer->GetSize())));
            return *this;
        }

        [[nodiscard]] BaseIterator operator+(
            const ptrdiff_t _offset) const
        {
            BaseIterator tmp = *this;
            tmp += _offset;
            return tmp;
        }

        BaseIterator& operator-=(
            const ptrdiff_t _offset)
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            m_index = static_cast<size_t>(Clamp<ptrdiff_t>(static_cast<ptrdiff_t>(m_index) - _offset, 0, static_cast<ptrdiff_t>(m_pContainer->GetSize())));
            return *this;
        }

        [[nodiscard]] BaseIterator operator-(
            const ptrdiff_t _offset) const
        {
            BaseIterator tmp = *this;
            tmp -= _offset;
            return tmp;
        }

        [[nodiscard]] ValueT& operator[](
            const ptrdiff_t _offset) const
        {
            JUG_ASSERT(m_pContainer, "RingBuffer must not be null.\n");
            return m_pContainer->GetValue_(static_cast<size_t>(Clamp<ptrdiff_t>(static_cast<ptrdiff_t>(m_index) + _offset, 0, static_cast<ptrdiff_t>(m_pContainer->GetSize()))));
        }

        // ===========================================
        //  Comparison
        // ===========================================

        [[nodiscard]] bool operator==(const BaseIterator& _other) const  = default;
        [[nodiscard]] auto operator<=>(const BaseIterator& _other) const = default;

        // ===========================================
        //  Conversion
        // ===========================================

        operator BaseIterator<true>() const
        {
            return BaseIterator<true>(m_pContainer, m_index);
        }

    private:
        BaseIterator(
            ContainerT*  _pContainer,
            const size_t _index)
            : m_pContainer(_pContainer)
            , m_index(_index)
        {
            JUG_ASSERT(_pContainer, "RingBuffer must not be null.\n");
        }

        ContainerT* m_pContainer = nullptr;
        size_t      m_index      = 0;
    };

    using Iterator      = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    explicit RingBuffer(
        const size_t _capacity = ring_buffer_detail::kDefaultCapacity,
        Alloc        _alloc    = Alloc {})
        : m_capacity(_capacity)
        , m_allocator(std::move(_alloc))
    {
        JUG_ASSERT(_capacity > 0, "RingBuffer capacity must be greater than 0.\n");
    }

    RingBuffer(
        const RingBuffer& _other)
        : m_capacity(_other.m_capacity)
        , m_allocator(_other.m_allocator)
    {
        CopyConstructFrom_(_other);
    }

    RingBuffer& operator=(
        const RingBuffer& _other)
    {
        if (this != &_other)
        {
            Destroy_();
            m_capacity  = _other.m_capacity;
            m_allocator = _other.m_allocator;
            CopyConstructFrom_(_other);
        }
        return *this;
    }

    RingBuffer(
        RingBuffer&& _other) noexcept(std::is_nothrow_move_constructible_v<Alloc>)
        : m_pData(std::exchange(_other.m_pData, nullptr))
        , m_head(std::exchange(_other.m_head, 0))
        , m_size(std::exchange(_other.m_size, 0))
        , m_capacity(std::exchange(_other.m_capacity, 0))
        , m_allocator(std::move(_other.m_allocator))
    {
    }

    RingBuffer& operator=(
        RingBuffer&& _other) noexcept(std::is_nothrow_move_assignable_v<Alloc>)
    {
        if (this != &_other)
        {
            Destroy_();
            m_pData     = std::exchange(_other.m_pData, nullptr);
            m_head      = std::exchange(_other.m_head, 0);
            m_size      = std::exchange(_other.m_size, 0);
            m_capacity  = std::exchange(_other.m_capacity, 0);
            m_allocator = std::move(_other.m_allocator);
        }
        return *this;
    }

    ~RingBuffer()
    {
        Destroy_();
    }

    void PushBack(
        const T& _value)
    {
        EmplaceBack(_value);
    }

    void PushBack(
        T&& _value)
    {
        EmplaceBack(std::move(_value));
    }

    template<typename... Args>
    void EmplaceBack(
        Args&&... _args)
    {
        Ensure_();

        if (IsFull())
        {
            PopFront();
        }

        std::construct_at(m_pData + ToRealIndex_(m_size), std::forward<Args>(_args)...);
        ++m_size;
    }

    void PushFront(
        const T& _value)
    {
        EmplaceFront(_value);
    }

    void PushFront(
        T&& _value)
    {
        EmplaceFront(std::move(_value));
    }

    template<typename... Args>
    void EmplaceFront(
        Args&&... _args)
    {
        Ensure_();

        if (IsFull())
        {
            PopBack();
        }

        m_head = (m_head > 0) ? m_head - 1 : m_capacity - 1;
        std::construct_at(m_pData + m_head, std::forward<Args>(_args)...);
        ++m_size;
    }

    void PopFront()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        std::destroy_at(m_pData + m_head);
        m_head = ToRealIndex_(1);
        --m_size;
    }

    void PopBack()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        std::destroy_at(m_pData + ToRealIndex_(m_size - 1));
        --m_size;
    }

    void SetCapacity(
        const size_t _capacity)
    {
        Reserve_(_capacity);
    }

    void Clear()
    {
        while (!IsEmpty())
        {
            PopFront();
        }
    }

    [[nodiscard]] T& Front()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        return m_pData[m_head];
    }

    [[nodiscard]] const T& Front() const
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        return m_pData[m_head];
    }

    [[nodiscard]] T& Back()
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        return GetValue_(m_size - 1);
    }

    [[nodiscard]] const T& Back() const
    {
        JUG_ASSERT(!IsEmpty(), "RingBuffer is empty.\n");
        return GetValue_(m_size - 1);
    }

    [[nodiscard]] T& operator[](
        const size_t _index)
    {
        return GetValue_(_index);
    }

    [[nodiscard]] const T& operator[](
        const size_t _index) const
    {
        return GetValue_(_index);
    }

    [[nodiscard]] T& At(
        const size_t _index)
    {
        JUG_ASSERT(_index < m_size, "RingBuffer index out of range.\n");
        return GetValue_(_index);
    }

    [[nodiscard]] const T& At(
        const size_t _index) const
    {
        JUG_ASSERT(_index < m_size, "RingBuffer index out of range.\n");
        return GetValue_(_index);
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_size;
    }

    [[nodiscard]] size_t GetCapacity() const
    {
        return m_capacity;
    }

    [[nodiscard]] size_t GetAvailSize() const
    {
        return GetMaxSize() - m_size;
    }

    [[nodiscard]] size_t GetMaxSize() const
    {
        return m_capacity - 1;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_size == 0;
    }

    [[nodiscard]] bool IsFull() const
    {
        return m_size + 1 == m_capacity;
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
        return Begin();
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return End();
    }

    // ===========================================
    //  STL Like
    // ===========================================

    using iterator       = Iterator;
    using const_iterator = ConstIterator;

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
    void Ensure_()   // 메모리를 보장
    {
        Reserve_(m_capacity);
    }

    void Reserve_(
        const size_t _capacity)
    {
        if (_capacity == m_capacity && m_pData)
        {
            return;
        }

        DoReserve_(_capacity);
    }

    void DoReserve_(
        const size_t _capacity)
    {
        JUG_ASSERT(_capacity > 0, "RingBuffer capacity must be greater than 0.\n");

        T*           pMem = m_allocator.allocate(_capacity);
        const size_t size = Min(m_size, _capacity - 1);   // 슬롯 하나는 항상 비워 둬야함.

        if (m_pData)
        {
            size_t i = 0;
            for (; i < size; ++i)
            {
                T* pSrc = m_pData + ToRealIndex_(i);
                std::construct_at(pMem + i, std::move(*pSrc));
                std::destroy_at(pSrc);
            }

            // 남은 원소는 파괴만
            for (; i < m_size; ++i)
            {
                std::destroy_at(m_pData + ToRealIndex_(i));
            }

            m_allocator.deallocate(m_pData, m_capacity);
        }

        m_pData    = pMem;
        m_head     = 0;
        m_capacity = _capacity;
        m_size     = size;
    }

    void Destroy_()
    {
        if (m_pData)
        {
            Clear();
            m_allocator.deallocate(m_pData, m_capacity);
        }

        m_pData = nullptr;
        m_head  = 0;
        m_size  = 0;
    }

    void CopyConstructFrom_(
        const RingBuffer& _other)
    {
        JUG_ASSERT(m_pData == nullptr, "RingBuffer must be empty before copying.\n");

        if (!_other.m_pData)
        {
            return;
        }

        m_pData = m_allocator.allocate(m_capacity);
        m_head  = 0;

        for (size_t i = 0; i < _other.m_size; ++i)
        {
            std::construct_at(m_pData + i, _other.GetValue_(i));
        }
        m_size = _other.m_size;
    }

    [[nodiscard]] size_t ToRealIndex_(
        const size_t _logicalIndex) const
    {
        const size_t index = m_head + _logicalIndex;
        return (index < m_capacity) ? index : (index - m_capacity);
    }

    [[nodiscard]] T& GetValue_(
        const size_t _logicalIndex)
    {
        return m_pData[ToRealIndex_(_logicalIndex)];
    }

    [[nodiscard]] const T& GetValue_(
        const size_t _logicalIndex) const
    {
        return m_pData[ToRealIndex_(_logicalIndex)];
    }

    T*                          m_pData     = nullptr;
    size_t                      m_head      = 0;
    size_t                      m_size      = 0;
    size_t                      m_capacity  = 0;
    [[no_unique_address]] Alloc m_allocator = {};
};

}   // namespace jug