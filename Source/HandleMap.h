#pragma once
#include <iterator>
#include <span>
#include <type_traits>
#include <vector>

#include "Handle.h"

namespace jug
{

namespace handle_map_detail
{
    constexpr size_t kNullIndex  = SIZE_MAX;
    constexpr size_t kGrowFactor = 2;
    constexpr size_t kMinSize    = 4;
}   // namespace handle_map_detail

// ================================================
//  HandleMap
//   Handle를 key로 사용하여 값을 저장하는 연속적인 컨테이너.
//   리소스는 반드시 연속된 컨테이너에 저장되고,
//   Sparse 배열을 동시에 운용하기에 랜덤 억세스도 O(1)로 가능.
//   삽입, 삭제, 검색 모두 O(1). 순회에 있어서 굉장히 높은 퍼포먼스.
//   단, 객체의 포인터는 언제든지 무효화될 수 있기 떄문에
//   따로 캐싱하지 말것.
//
//   만약 한 자료구조에서 핸들의 발급과 관리를 모두 하고 싶다면
//   ResourcePool을 쓸 것
// ================================================

template<typename TTag, typename TValue>
class HandleMap
{
    using Handle = Handle<TTag>;

public:
    // ===========================================
    //  Iterator
    // ===========================================

    template<bool kbIsConst>
    class BaseIterator
    {
        template<bool>
        friend class BaseIterator;
        friend class HandleMap;

        using ContainerT = std::conditional_t<kbIsConst, const HandleMap, HandleMap>;
        using ValueT     = std::conditional_t<kbIsConst, const TValue, TValue>;

    public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = TValue;
        using difference_type   = ptrdiff_t;
        using pointer           = ValueT*;
        using reference         = ValueT&;

        BaseIterator() = default;

        [[nodiscard]] Handle GetHandle() const
        {
            JUG_ASSERT(m_pMap && m_index < m_pMap->GetSize(), "Cannot get a handle from an out of range iterator.\n");
            return m_pMap->m_handles[m_index];
        }

        [[nodiscard]] reference operator*() const
        {
            JUG_ASSERT(m_pMap && m_index < m_pMap->GetSize(), "Cannot dereference an out of range iterator.\n");
            return m_pMap->m_dense[m_index];
        }

        [[nodiscard]] pointer operator->() const
        {
            JUG_ASSERT(m_pMap && m_index < m_pMap->GetSize(), "Cannot dereference an out of range iterator.\n");
            return std::addressof(m_pMap->m_dense[m_index]);
        }

        [[nodiscard]] reference operator[](
            const difference_type _offset) const
        {
            return *(*this + _offset);
        }

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pMap && m_index < m_pMap->GetSize(), "Iterator is already at the end. Cannot increment further.\n");
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
            JUG_ASSERT(m_index > 0, "Iterator is already at the beginning. Cannot decrement further.\n");
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
            JUG_ASSERT(m_pMap && m_index <= m_pMap->GetSize(), "Iterator is out of range.\n");
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
            JUG_ASSERT(m_pMap == _other.m_pMap, "Cannot compare iterators from different HandleMaps.\n");
            return static_cast<difference_type>(m_index) - static_cast<difference_type>(_other.m_index);
        }

        [[nodiscard]] bool operator==(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pMap == _other.m_pMap, "Cannot compare iterators from different HandleMaps.\n");
            return m_index == _other.m_index;
        }

        [[nodiscard]] bool operator<(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pMap == _other.m_pMap, "Cannot compare iterators from different HandleMaps.\n");
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
            return BaseIterator<true> { m_pMap, m_index };
        }

    private:
        constexpr BaseIterator(
            ContainerT*  _pMap,
            const size_t _index)
            : m_pMap(_pMap)
            , m_index(_index)
        {
            JUG_ASSERT(_pMap, "HandleMap must not be null.\n");
        }

        ContainerT* m_pMap  = nullptr;
        size_t      m_index = 0;
    };

    using Iterator      = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    template<typename... TArgs>
    TValue& Emplace(
        const Handle _handle,
        TArgs&&... _args)
    {
        JUG_ASSERT(!_handle.IsNull(), "Cannot insert a null handle.\n");
        const size_t index = _handle.GetIndex();

        // realloc
        if (m_sparse.size() <= index)
        {
            const size_t size = std::max(handle_map_detail::kMinSize, index * handle_map_detail::kGrowFactor);
            m_sparse.resize(size, handle_map_detail::kNullIndex);
        }

        // insert
        JUG_ASSERT(m_sparse[index] == handle_map_detail::kNullIndex, "Handle is already occupied in HandleMap.\n");
        m_sparse[index] = m_dense.size();
        m_handles.push_back(_handle);
        return m_dense.emplace_back(std::forward<TArgs>(_args)...);
    }

    TValue& Insert(
        const Handle  _handle,
        const TValue& _value)
    {
        return Emplace(_handle, std::move(_value));
    }

    TValue& Insert(
        const Handle _handle,
        TValue&&     _value)
    {
        return Emplace(_handle, std::move(_value));
    }

    void Erase(
        const Handle _handle)
    {
        const size_t idx   = _handle.GetIndex();
        const size_t dense = m_sparse[idx];
        const size_t last  = m_dense.size() - 1;
        JUG_ASSERT(dense != handle_map_detail::kNullIndex, "Handle does not exist in HandleMap.\n");

        if (dense != last)
        {
            const Handle lastHandle         = m_handles[last];
            m_dense[dense]                  = std::move(m_dense[last]);
            m_handles[dense]                = lastHandle;
            m_sparse[lastHandle.GetIndex()] = dense;
        }

        m_sparse[idx] = handle_map_detail::kNullIndex;
        m_dense.pop_back();
        m_handles.pop_back();
    }

    [[nodiscard]] bool Contains(
        const Handle _handle) const
    {
        // null 도 동시에 잡음
        const size_t index = _handle.GetIndex();
        if (index >= m_sparse.size() || m_sparse[index] == handle_map_detail::kNullIndex)
        {
            return false;
        }

        return m_handles[m_sparse[index]] == _handle;
    }

    [[nodiscard]] TValue& Get(
        const Handle _handle)
    {
        JUG_ASSERT(Contains(_handle), "Handle does not exist in HandleMap.\n");
        return m_dense[m_sparse[_handle.GetIndex()]];
    }

    [[nodiscard]] const TValue& Get(
        const Handle _handle) const
    {
        return const_cast<HandleMap*>(this)->Get(_handle);
    }

    [[nodiscard]] TValue* GetOrNull(
        const Handle _handle)
    {
        if (!Contains(_handle))
        {
            return nullptr;
        }

        return &m_dense[m_sparse[_handle.GetIndex()]];
    }

    [[nodiscard]] const TValue* GetOrNull(
        const Handle _handle) const
    {
        return const_cast<HandleMap*>(this)->GetOrNull(_handle);
    }

    [[nodiscard]] TValue& operator[](
        const Handle _handle)
    {
        return Get(_handle);
    }

    [[nodiscard]] const TValue& operator[](
        const Handle _handle) const
    {
        return Get(_handle);
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_dense.size();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_dense.empty();
    }

    void Clear()
    {
        m_sparse.clear();
        m_handles.clear();
        m_dense.clear();
    }

    [[nodiscard]] Span<Handle> GetHandles()
    {
        return m_handles;
    }

    [[nodiscard]] Span<const Handle> GetHandles() const
    {
        return m_handles;
    }

    [[nodiscard]] Span<TValue> GetValues()
    {
        return m_dense;
    }

    [[nodiscard]] Span<const TValue> GetValues() const
    {
        return m_dense;
    }

    [[nodiscard]] TValue* GetPtr()
    {
        return m_dense.data();
    }

    [[nodiscard]] const TValue* GetPtr() const
    {
        return m_dense.data();
    }

    [[nodiscard]] Iterator Begin()
    {
        return Iterator { this, 0 };
    }

    [[nodiscard]] Iterator End()
    {
        return Iterator { this, GetSize() };
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return ConstIterator { this, 0 };
    }

    [[nodiscard]] ConstIterator End() const
    {
        return ConstIterator { this, GetSize() };
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return ConstIterator { this, 0 };
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return ConstIterator { this, GetSize() };
    }

    // ==========================================
    //  STL like
    // ==========================================

    using value_type     = TValue;
    using size_type      = size_t;
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

    [[nodiscard]] TValue* data()
    {
        return m_dense.data();
    }

    [[nodiscard]] const TValue* data() const
    {
        return m_dense.data();
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

protected:
    Vector<size_t> m_sparse  = {};   // dense index 저장
    Vector<TValue> m_dense   = {};   // 값을 연속적으로 저장
    Vector<Handle> m_handles = {};   // dense index에 대응하는 handle 저장
};

}   // namespace jug
