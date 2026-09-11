
#pragma once
#include <ranges>

#include "Assertion.h"
#include "Config.h"

namespace jug
{

template<bool kbConst>
class BasicMemoryView;

template<typename Container>
concept MemoryViewConstructableContainerT = std::ranges::contiguous_range<Container>
                                         && requires(Container& _cont) {typename Container::value_type; _cont.data(); _cont.size(); };

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

    template<MemoryViewConstructableContainerT Container, typename V = typename Container::value_type>
        requires(kbConst || !std::is_const_v<V>)
    /* implicit */ BasicMemoryView(
        Container& _container)
        : m_pMem(reinterpret_cast<ValueT*>(_container.data()))
        , m_size(_container.size() * sizeof(V))
    {
    }

    template<MemoryViewConstructableContainerT Container, typename V = typename Container::value_type>
        requires(kbConst || !std::is_const_v<V>)
    /* implicit */ BasicMemoryView(
        Container&   _container,
        const size_t _count)
        : m_pMem(reinterpret_cast<ValueT*>(_container.data()))
        , m_size(_count * sizeof(V))
    {
        JUG_ASSERT(_count <= _container.size(), "Count exceeds _cont bounds.\n");
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
        requires(!MemoryViewConstructableContainerT<T> && (kbConst || !std::is_const_v<T>))
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

    [[nodiscard]] size_t size() const
    {
        return m_size;
    }

    [[nodiscard]] bool empty() const
    {
        return m_size == 0;
    }

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
