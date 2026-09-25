#pragma once

namespace jug
{

namespace lru_cache_detail
{
    constexpr size_t kDefaultCapacity = 8;
}

template<
    typename K,
    typename V,
    typename H     = Hasher<K>,
    typename E     = EqualTo<K>,
    typename Alloc = Allocator<std::pair<const K, V>>>
class LruCache
{
    using ValueT   = std::pair<const K, V>;
    using ListT    = List<ValueT, Alloc>;
    using HashMapT = HashMap<K, typename ListT::iterator, H, E, Alloc>;

public:
    using Iterator      = typename ListT::iterator;
    using ConstIterator = typename ListT::const_iterator;

    explicit LruCache(
        const size_t _capacity = lru_cache_detail::kDefaultCapacity,
        const H&     _hash     = H {},
        const E&     _equal    = E {},
        const Alloc& _alloc    = Alloc {})
        : m_hashMap(0, _hash, _equal, _alloc)
        , m_list(_alloc)
        , m_capacity(_capacity)
    {
        JUG_ASSERT(_capacity > 0, "LruCache capacity must be greater than 0.\n");
    }

    std::pair<Iterator, bool> Insert(
        const K& _key,
        const V& _value)
    {
        return Emplace(_key, _value);
    }

    std::pair<Iterator, bool> Insert(
        const K& _key,
        V&&      _value)
    {
        return Emplace(_key, std::move(_value));
    }

    template<typename... Args>
    std::pair<Iterator, bool> Emplace(
        const K& _key,
        Args&&... _args)
    {
        const auto hashIt = m_hashMap.find(_key);
        if (hashIt != m_hashMap.end())
        {
            Touch_(hashIt->second);
            return { hashIt->second, false };
        }

        MakeSpace_();

        // insert succeeded
        m_list.emplace_front(std::piecewise_construct, std::forward_as_tuple(_key), std::forward_as_tuple(std::forward<Args>(_args)...));
        const Iterator listIt = m_list.begin();
        m_hashMap.emplace(_key, listIt);
        return { listIt, true };
    }

    Iterator InsertOrReplace(
        const K& _key,
        const V& _value)
    {
        return EmplaceOrReplace(_key, _value);
    }

    Iterator InsertOrReplace(
        const K& _key,
        V&&      _value)
    {
        return EmplaceOrReplace(_key, std::move(_value));
    }

    template<typename... Args>
    Iterator EmplaceOrReplace(
        const K& _key,
        Args&&... _value)
    {
        // 이미 있을 경우 값을 덮어쓴다.
        const auto it = m_hashMap.find(_key);
        if (it != m_hashMap.end())
        {
            it->second->second = V(std::forward<Args>(_value)...);
            Touch_(it->second);
            return it->second;
        }

        MakeSpace_();
        m_list.emplace_front(std::piecewise_construct, std::forward_as_tuple(_key), std::forward_as_tuple(std::forward<Args>(_value)...));

        const Iterator listIt = m_list.begin();
        m_hashMap.insert_or_assign(_key, listIt);
        return listIt;
    }

    bool Erase(
        const K& _key)
    {
        const auto it = m_hashMap.find(_key);
        if (it == m_hashMap.end())
        {
            return false;
        }

        m_list.erase(it->second);
        m_hashMap.erase(it);
        return true;
    }

    void EraseOldest()
    {
        JUG_ASSERT(!IsEmpty(), "LruCache is empty, cannot erase oldest element.\n");
        const size_t numErased = m_hashMap.erase(m_list.back().first);
        JUG_ASSERT(numErased == 1, "Failed to erase oldest element from hash map.\n");
        m_list.pop_back();
    }

    void Clear()
    {
        m_hashMap.clear();
        m_list.clear();
    }

    void SetCapacity(
        const size_t _capacity)
    {
        JUG_ASSERT(_capacity > 0, "LruCache capacity must be greater than 0.\n");
        m_capacity = _capacity;
        Trim_();
    }

    // 승격만
    void Touch(
        const K& _key)
    {
        const auto it = m_hashMap.find(_key);
        JUG_ASSERT(it != m_hashMap.end(), "Key not found in LruCache.\n");
        Touch_(it->second);
    }

    // 조회 후 승격
    [[nodiscard]] Iterator Find(
        const K& _key)
    {
        const auto it = m_hashMap.find(_key);
        if (it == m_hashMap.end())
        {
            return m_list.end();
        }

        // 승격
        Touch_(it->second);
        return it->second;
    }

    // 승격 없이 조회만
    [[nodiscard]] ConstIterator Peek(
        const K& _key) const
    {
        const auto it = m_hashMap.find(_key);
        return (it == m_hashMap.end()) ? m_list.end() : ConstIterator(it->second);
    }

    [[nodiscard]] V& At(
        const K& _key)
    {
        const auto it = m_hashMap.find(_key);
        JUG_ASSERT(it != m_hashMap.end(), "Key not found in LruCache.\n");
        Touch_(it->second);
        return it->second->second;
    }

    // 존재하지 않을 경우 생성
    [[nodiscard]] V& operator[](
        const K& _key)
        requires(std::is_default_constructible_v<V>)
    {
        return Emplace(_key).first->second;
    }

    [[nodiscard]] bool Contains(
        const K& _key) const
    {
        return m_hashMap.find(_key) != m_hashMap.end();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_list.empty();
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_hashMap.size();
    }

    [[nodiscard]] size_t GetCapacity() const
    {
        return m_capacity;
    }

    // MRU -> LRU 순회
    [[nodiscard]] Iterator Begin()
    {
        return m_list.begin();
    }

    [[nodiscard]] Iterator End()
    {
        return m_list.end();
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return m_list.begin();
    }

    [[nodiscard]] ConstIterator End() const
    {
        return m_list.end();
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return m_list.cbegin();
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return m_list.cend();
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
    void Touch_(
        const Iterator _it)   // NOLINT
    {
        m_list.splice(m_list.begin(), m_list, _it);
    }

    void Trim_()
    {
        while (GetSize() > m_capacity)
        {
            EraseOldest();
        }
    }

    void MakeSpace_()
    {
        while (GetSize() >= m_capacity)
        {
            EraseOldest();
        }
    }

    HashMapT m_hashMap  = {};
    ListT    m_list     = {};
    size_t   m_capacity = lru_cache_detail::kDefaultCapacity;
};

}   // namespace jug