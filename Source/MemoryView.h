
#pragma once
#include <ranges>
#include "Config.h"

namespace jug
{

template<bool kbIsConst>
class BasicMemoryView;

template<typename TContainer>
concept MemoryViewConstructableContainerT = std::ranges::contiguous_range<TContainer>
                                         && requires(TContainer& _cont) {typename TContainer::value_type; _cont.data(); _cont.size(); };

template<bool kbIsConst>
class BasicMemoryView
{
public:
    using ByteT         = std::conditional_t<kbIsConst, const std::byte, std::byte>;
    using Iterator      = ByteT*;
    using ConstIterator = const ByteT*;

    BasicMemoryView() = default;

    BasicMemoryView(
        ByteT*       _pValue,
        const size_t _size)
        : m_pMem(_pValue)
        , m_size(_size)
    {
    }

    template<MemoryViewConstructableContainerT TContainer, typename TValue = typename TContainer::value_type>
        requires(kbIsConst || !std::is_const_v<TValue>)
    /* implicit */ BasicMemoryView(
        TContainer& _container)
        : m_pMem(reinterpret_cast<ByteT*>(_container.data()))
        , m_size(_container.size() * sizeof(TValue))
    {
    }

    template<MemoryViewConstructableContainerT TContainer, typename TValue = typename TContainer::value_type>
        requires(kbIsConst || !std::is_const_v<TValue>)
    /* implicit */ BasicMemoryView(
        TContainer&  _container,
        const size_t _count)
        : m_pMem(reinterpret_cast<ByteT*>(_container.data()))
        , m_size(_count * sizeof(TValue))
    {
        JUG_ASSERT(_count <= _container.size(), "Count exceeds _cont bounds.\n");
    }

    template<typename T, size_t N>
        requires(kbIsConst || !std::is_const_v<T>)
    /* implicit */ BasicMemoryView(
        T (&_data)[N])
        : m_pMem(reinterpret_cast<ByteT*>(_data))
        , m_size(N * sizeof(T))
    {
    }

    template<typename T, size_t N>
        requires(kbIsConst || !std::is_const_v<T>)
    /* implicit */ BasicMemoryView(
        T (&_data)[N],
        const size_t _count)
        : m_pMem(reinterpret_cast<ByteT*>(_data))
        , m_size(_count * sizeof(T))
    {
        JUG_ASSERT(_count <= N, "Count exceeds array bounds.\n");
    }

    template<typename T>
        requires(!MemoryViewConstructableContainerT<T> && (kbIsConst || !std::is_const_v<T>))
    /* implicit */ BasicMemoryView(
        T& _data)
        : m_pMem(reinterpret_cast<ByteT*>(&_data))
        , m_size(sizeof(T))
    {
    }

    [[nodiscard]] ByteT& operator[](
        const size_t _index)
    {
        JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
        return m_pMem[_index];
    }

    [[nodiscard]] const ByteT& operator[](
        const size_t _index) const
    {
        JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
        return m_pMem[_index];
    }

    [[nodiscard]] ByteT* GetPtr() const
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

    [[nodiscard]] ByteT* data()
    {
        return m_pMem;
    }

    [[nodiscard]] ByteT* data() const
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
    ByteT* m_pMem = nullptr;
    size_t m_size = 0;
};

using MemoryView        = BasicMemoryView<true>;
using MutableMemoryView = BasicMemoryView<false>;

}   // namespace jug
