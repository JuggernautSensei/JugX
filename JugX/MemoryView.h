#pragma once

namespace jug
{

template<bool kbConst>
class BasicMemoryView;

template<bool kbConst>
class BasicMemoryView
{
    using ValueT = std::conditional_t<kbConst, const std::byte, std::byte>;

public:
    using Iterator      = ValueT*;
    using ConstIterator = const ValueT*;

    BasicMemoryView() = default;

    BasicMemoryView(
        ValueT*      _pValue,
        const size_t _size)
        : m_pMem(_pValue)
        , m_size(_size)
    {
    }

    template<typename Container, typename V = typename Container::value_type>
        requires(std::ranges::contiguous_range<Container> && (kbConst || !std::is_const_v<V>))
    /* implicit */ BasicMemoryView(
        Container& _container)
        : m_pMem(reinterpret_cast<ValueT*>(std::data(_container)))
        , m_size(std::size(_container) * sizeof(V))
    {
    }

    template<typename Container, typename V = typename Container::value_type>
        requires(std::ranges::contiguous_range<Container> && (kbConst || !std::is_const_v<V>))
    /* implicit */ BasicMemoryView(
        Container&   _container,
        const size_t _count)
        : m_pMem(reinterpret_cast<ValueT*>(std::data(_container)))
        , m_size(_count * sizeof(V))
    {
        JUG_ASSERT(_count <= std::size(_container), "Count exceeds _container bounds.\n");
    }

    template<typename T, size_t kSize>
        requires(kbConst || !std::is_const_v<T>)
    /* implicit */ BasicMemoryView(
        T (&_data)[kSize])
        : m_pMem(reinterpret_cast<ValueT*>(_data))
        , m_size(kSize * sizeof(T))
    {
    }

    template<typename T, size_t kSize>
        requires(kbConst || !std::is_const_v<T>)
    /* implicit */ BasicMemoryView(
        T (&_data)[kSize],
        const size_t _count)
        : m_pMem(reinterpret_cast<ValueT*>(_data))
        , m_size(_count * sizeof(T))
    {
        JUG_ASSERT(_count <= kSize, "Count exceeds array bounds.\n");
    }

    template<typename T>
        requires(!std::ranges::contiguous_range<T> && (kbConst || !std::is_const_v<T>))
    /* implicit */ BasicMemoryView(
        T& _data)
        : m_pMem(reinterpret_cast<ValueT*>(&_data))
        , m_size(sizeof(T))
    {
    }

    [[nodiscard]] ValueT& operator[](
        const size_t _index)
    {
        JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
        return m_pMem[_index];
    }

    [[nodiscard]] const ValueT& operator[](
        const size_t _index) const
    {
        JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
        return m_pMem[_index];
    }

    [[nodiscard]] ValueT* GetPtr() const
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

    [[nodiscard]] BasicMemoryView SubView(
        const size_t _offset,
        const size_t _size) const
    {
        JUG_ASSERT(_offset <= m_size && _size <= m_size - _offset, "SubView exceeds bounds.\n");
        return BasicMemoryView { m_pMem + _offset, _size };
    }

    [[nodiscard]] BasicMemoryView SubView(
        const size_t _offset) const
    {
        JUG_ASSERT(_offset <= m_size, "SubView exceeds bounds.\n");
        return BasicMemoryView { m_pMem + _offset, m_size - _offset };
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

    // ========================================
    //  STL Like
    // ========================================

    using value_type     = ValueT;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] value_type* data()
    {
        return m_pMem;
    }

    [[nodiscard]] value_type* data() const
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
    ValueT* m_pMem = nullptr;
    size_t  m_size = 0;
};

using MemoryView        = BasicMemoryView<true>;
using MutableMemoryView = BasicMemoryView<false>;

}   // namespace jug
