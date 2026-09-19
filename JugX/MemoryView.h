#pragma once

namespace jug
{

class MemoryView;

// ===========================================
//  Mutable Memory View
// ===========================================

class MutableMemoryView
{
public:
    using Iterator      = std::byte*;
    using ConstIterator = const std::byte*;

    MutableMemoryView() = default;
    MutableMemoryView(std::byte* _pValue, size_t _size);
    MutableMemoryView(void* _pValue, size_t _size);

    template<typename Container, typename V = typename Container::value_type>
        requires(std::ranges::contiguous_range<Container> && !std::is_const_v<V>)
    /* implicit */ MutableMemoryView(
        Container& _container)
        : m_pMem(reinterpret_cast<std::byte*>(std::data(_container)))
        , m_size(std::size(_container) * sizeof(V))
    {
    }

    template<typename Container, typename V = typename Container::value_type>
        requires(std::ranges::contiguous_range<Container> && !std::is_const_v<V>)
    /* implicit */ MutableMemoryView(
        Container&   _container,
        const size_t _count)
        : m_pMem(reinterpret_cast<std::byte*>(std::data(_container)))
        , m_size(_count * sizeof(V))
    {
        JUG_ASSERT(_count <= std::size(_container), "Count exceeds _container bounds.\n");
    }

    template<typename T, size_t kSize>
    /* implicit */ MutableMemoryView(
        T (&_data)[kSize])
        : m_pMem(reinterpret_cast<std::byte*>(_data))
        , m_size(kSize * sizeof(T))
    {
    }

    template<typename T, size_t kSize>
    /* implicit */ MutableMemoryView(
        T (&_data)[kSize],
        const size_t _count)
        : m_pMem(reinterpret_cast<std::byte*>(_data))
        , m_size(_count * sizeof(T))
    {
        JUG_ASSERT(_count <= kSize, "Count exceeds array bounds.\n");
    }

    template<typename T>
        requires(!std::ranges::contiguous_range<T> && !std::is_const_v<T>)
    /* implicit */ MutableMemoryView(
        T& _data)
        : m_pMem(reinterpret_cast<std::byte*>(&_data))
        , m_size(sizeof(T))
    {
    }

    [[nodiscard]] std::byte& operator[](size_t _index) const;
    [[nodiscard]] std::byte* GetPtr() const;
    [[nodiscard]] size_t     GetSize() const;
    [[nodiscard]] bool       IsEmpty() const;

    [[nodiscard]] MutableMemoryView SubView(size_t _offset, size_t _size) const;
    [[nodiscard]] MutableMemoryView SubView(size_t _offset) const;

    [[nodiscard]] Iterator      Begin();
    [[nodiscard]] Iterator      End();
    [[nodiscard]] ConstIterator Begin() const;
    [[nodiscard]] ConstIterator End() const;
    [[nodiscard]] ConstIterator CBegin() const;
    [[nodiscard]] ConstIterator CEnd() const;

    // ========================================
    //  STL Like
    // ========================================

    using value_type     = std::byte;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] value_type*       data();
    [[nodiscard]] const value_type* data() const;
    [[nodiscard]] iterator          begin();
    [[nodiscard]] iterator          end();
    [[nodiscard]] const_iterator    begin() const;
    [[nodiscard]] const_iterator    end() const;
    [[nodiscard]] const_iterator    cbegin() const;
    [[nodiscard]] const_iterator    cend() const;

private:
    std::byte* m_pMem = nullptr;
    size_t     m_size = 0;
};

// ===========================================
//  Memory View
// ===========================================

class MemoryView
{
public:
    using Iterator      = const std::byte*;
    using ConstIterator = const std::byte*;

    MemoryView() = default;
    MemoryView(const std::byte* _pValue, size_t _size);
    MemoryView(const void* _pValue, size_t _size);
    /* implicit */ MemoryView(MutableMemoryView _mem);

    template<typename Container, typename V = const typename Container::value_type>
        requires std::ranges::contiguous_range<Container>
    /* implicit */ MemoryView(
        Container& _container)
        : m_pMem(reinterpret_cast<const std::byte*>(std::data(_container)))
        , m_size(std::size(_container) * sizeof(V))
    {
    }

    template<typename Container, typename V = const typename Container::value_type>
        requires std::ranges::contiguous_range<Container>
    /* implicit */ MemoryView(
        Container&   _container,
        const size_t _count)
        : m_pMem(reinterpret_cast<const std::byte*>(std::data(_container)))
        , m_size(_count * sizeof(V))
    {
        JUG_ASSERT(_count <= std::size(_container), "Count exceeds _container bounds.\n");
    }

    template<typename T, size_t kSize>
    /* implicit */ MemoryView(
        T (&_data)[kSize])
        : m_pMem(reinterpret_cast<const std::byte*>(_data))
        , m_size(kSize * sizeof(T))
    {
    }

    template<typename T, size_t kSize>
    /* implicit */ MemoryView(
        T (&_data)[kSize],
        const size_t _count)
        : m_pMem(reinterpret_cast<const std::byte*>(_data))
        , m_size(_count * sizeof(T))
    {
        JUG_ASSERT(_count <= kSize, "Count exceeds array bounds.\n");
    }

    template<typename T>
        requires(!std::ranges::contiguous_range<T>)
    /* implicit */ MemoryView(
        T& _data)
        : m_pMem(reinterpret_cast<const std::byte*>(&_data))
        , m_size(sizeof(T))
    {
    }

    [[nodiscard]] const std::byte& operator[](size_t _index) const;
    [[nodiscard]] const std::byte* GetPtr() const;
    [[nodiscard]] size_t           GetSize() const;
    [[nodiscard]] bool             IsEmpty() const;

    [[nodiscard]] MemoryView SubView(size_t _offset, size_t _size) const;
    [[nodiscard]] MemoryView SubView(size_t _offset) const;

    [[nodiscard]] ConstIterator Begin() const;
    [[nodiscard]] ConstIterator End() const;
    [[nodiscard]] ConstIterator CBegin() const;
    [[nodiscard]] ConstIterator CEnd() const;

    // ========================================
    //  STL Like
    // ========================================

    using value_type     = const std::byte;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] const value_type* data() const;
    [[nodiscard]] const_iterator    begin() const;
    [[nodiscard]] const_iterator    end() const;
    [[nodiscard]] const_iterator    cbegin() const;
    [[nodiscard]] const_iterator    cend() const;

private:
    const std::byte* m_pMem = nullptr;
    size_t           m_size = 0;
};

}   // namespace jug
