#pragma once
#include <iterator>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "Assertion.h"
#include "Handle.h"
#include "Math.h"
#include "Typedef.h"

namespace jug
{

namespace handle_storage_detail
{
    constexpr size_t kNullIndex  = SIZE_MAX;
    constexpr size_t kGrowFactor = 2;
    constexpr size_t kMinSize    = 4;
}   // namespace handle_storage_detail

// ================================================
//  HandleStorageT
//   Handle을 key로 사용하여 값을 저장하는 연속적인 컨테이너.
//   핸들 배열과 값 배열을 분리 저장(SoA)하여
//   값만 순회할 때 최대 캐시 효율을 얻는다.
//   Sparse 배열을 동시에 운용하기에 랜덤 억세스도 O(1).
//   삽입, 삭제, 검색 모두 O(1).
//   객체의 포인터는 언제든지 무효화될 수 있기 때문에 따로 캐싱하지 말것.
//
//   만약 한 자료구조에서 핸들의 발급과 관리를 모두 하고 싶다면
//   ResourcePool을 쓸 것
// ================================================

template<HandleT H, typename V>
class HandleStorage
{
public:
    // ===========================================
    //  Iterator
    // ===========================================

    template<bool kbConst>
    class BaseIterator
    {
        template<bool>
        friend class BaseIterator;
        friend class HandleStorage;

        using ContainerT = std::conditional_t<kbConst, const HandleStorage, HandleStorage>;
        using ValueT     = std::conditional_t<kbConst, const V, V>;

    public:
        BaseIterator() = default;

        // ===========================================
        //  Access
        // ===========================================

        [[nodiscard]] H GetHandle() const
        {
            JUG_ASSERT(m_pStorage && m_index < m_pStorage->GetSize(), "Cannot get a handle from an out of range iterator.\n");
            return m_pStorage->m_handles[m_index];
        }

        [[nodiscard]] ValueT& operator*() const
        {
            JUG_ASSERT(m_pStorage && m_index < m_pStorage->GetSize(), "Cannot dereference an out of range iterator.\n");
            return m_pStorage->m_values[m_index];
        }

        [[nodiscard]] ValueT* operator->() const
        {
            JUG_ASSERT(m_pStorage && m_index < m_pStorage->GetSize(), "Cannot dereference an out of range iterator.\n");
            return std::addressof(m_pStorage->m_values[m_index]);
        }

        // ===========================================
        //  Increment/Decrement
        // ===========================================

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pStorage && m_index < m_pStorage->GetSize(), "Iterator is already at the end. Cannot increment further.\n");
            ++m_index;
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
            JUG_ASSERT(m_index > 0, "Iterator is already at the beginning. Cannot decrement further.\n");
            --m_index;
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
            m_index = static_cast<size_t>(static_cast<ptrdiff_t>(m_index) + _offset);
            JUG_ASSERT(m_pStorage && m_index <= m_pStorage->GetSize(), "Iterator is out of range.\n");
            return *this;
        }

        [[nodiscard]] BaseIterator operator+(
            const ptrdiff_t _offset) const
        {
            BaseIterator tmp = *this;
            return tmp += _offset;
        }

        BaseIterator& operator-=(
            const ptrdiff_t _offset)
        {
            return *this += -_offset;
        }

        [[nodiscard]] BaseIterator operator-(
            const ptrdiff_t _offset) const
        {
            BaseIterator tmp = *this;
            return tmp -= _offset;
        }

        [[nodiscard]] ptrdiff_t operator-(
            const BaseIterator& _other) const
        {
            JUG_ASSERT(m_pStorage == _other.m_pStorage, "Cannot compare iterators from different HandleStorages.\n");
            return static_cast<ptrdiff_t>(m_index) - static_cast<ptrdiff_t>(_other.m_index);
        }

        [[nodiscard]] ValueT& operator[](
            const ptrdiff_t _offset) const
        {
            return *(*this + _offset);
        }

        // ===========================================
        //  Comparison
        // ===========================================

        [[nodiscard]] bool operator==(const BaseIterator& _other) const  = default;
        [[nodiscard]] auto operator<=>(const BaseIterator& _other) const = default;

        // ===========================================
        //  Conversion
        // ===========================================

        [[nodiscard]] operator BaseIterator<true>() const
            requires(!kbConst)
        {
            return BaseIterator<true> { m_pStorage, m_index };
        }

    private:
        constexpr BaseIterator(
            ContainerT*  _pStorage,
            const size_t _index)
            : m_pStorage(_pStorage)
            , m_index(_index)
        {
            JUG_ASSERT(_pStorage, "HandleStorageT must not be null.\n");
        }

        ContainerT* m_pStorage = nullptr;
        size_t      m_index    = 0;
    };

    using Iterator      = BaseIterator<false>;
    using ConstIterator = BaseIterator<true>;

public:
    void Insert(
        const H  _handle,
        const V& _value)
    {
        Emplace(_handle, _value);
    }

    void Insert(
        const H _handle,
        V&&     _value)
    {
        Emplace(_handle, std::move(_value));
    }

    template<typename... Args>
    V& Emplace(
        const H _handle,
        Args&&... _args)
    {
        JUG_ASSERT(!_handle.IsNull(), "Cannot insert a null handle.\n");
        JUG_ASSERT(FindDenseOrNullIndex_(_handle) == handle_storage_detail::kNullIndex, "_handle already exists in HandleStorageT.\n");

        // make space
        const size_t index = _handle.GetIndex();
        if (index >= m_sparse.size())
        {
            const size_t size = Max(handle_storage_detail::kMinSize, index * handle_storage_detail::kGrowFactor);
            m_sparse.resize(size, handle_storage_detail::kNullIndex);
        }

        // insert
        m_sparse[index] = m_values.size();
        m_handles.emplace_back(_handle);
        return m_values.emplace_back(std::forward<Args>(_args)...);
    }

    V& InsertOrReplace(
        const H  _handle,
        const V& _value)
    {
        return EmplaceOrReplace(_handle, _value);
    }

    V& InsertOrReplace(
        const H _handle,
        V&&     _value)
    {
        return EmplaceOrReplace(_handle, std::move(_value));
    }

    template<typename... Args>
    V& EmplaceOrReplace(
        const H _handle,
        Args&&... _args)
    {
        JUG_ASSERT(!_handle.IsNull(), "Cannot insert a null handle.\n");

        const size_t dense = FindDenseOrNullIndex_(_handle);
        if (dense != handle_storage_detail::kNullIndex)
        {
            m_values[dense] = V { std::forward<Args>(_args)... };
            return m_values[dense];
        }

        return Emplace(_handle, std::forward<Args>(_args)...);
    }

    void Erase(
        const H _handle)
    {
        const size_t dense = FindDenseOrNullIndex_(_handle);
        JUG_ASSERT(dense != handle_storage_detail::kNullIndex, "_handle does not exist in HandleStorageT.\n");

        // swap & pop
        const size_t last = m_values.size() - 1;
        if (dense != last)
        {
            const size_t lastIndex = m_handles[last].GetIndex();
            m_handles[dense]       = m_handles[last];
            m_values[dense]        = std::move(m_values[last]);
            m_sparse[lastIndex]    = dense;
        }

        m_sparse[_handle.GetIndex()] = handle_storage_detail::kNullIndex;
        m_handles.pop_back();
        m_values.pop_back();
    }

    void Clear()
    {
        m_sparse.clear();
        m_handles.clear();
        m_values.clear();
    }

    [[nodiscard]] bool Contains(
        const H _handle) const
    {
        // null 도 동시에 잡음
        return FindDenseOrNullIndex_(_handle) != handle_storage_detail::kNullIndex;
    }

    [[nodiscard]] V& Get(
        const H _handle)
    {
        const size_t dense = FindDenseOrNullIndex_(_handle);
        JUG_ASSERT(dense != handle_storage_detail::kNullIndex, "_handle does not exist in HandleStorageT.\n");
        return m_values[dense];
    }

    [[nodiscard]] const V& Get(
        const H _handle) const
    {
        return const_cast<HandleStorage*>(this)->Get(_handle);
    }

    [[nodiscard]] V* GetOrNull(
        const H _handle)
    {
        const size_t dense = FindDenseOrNullIndex_(_handle);
        return dense != handle_storage_detail::kNullIndex ? &m_values[dense] : nullptr;
    }

    [[nodiscard]] const V* GetOrNull(
        const H _handle) const
    {
        return const_cast<HandleStorage*>(this)->GetOrNull(_handle);
    }

    [[nodiscard]] V& operator[](
        const H _handle)
    {
        return Get(_handle);
    }

    [[nodiscard]] const V& operator[](
        const H _handle) const
    {
        return Get(_handle);
    }

    [[nodiscard]] std::span<V> GetValues()
    {
        return { m_values };
    }

    [[nodiscard]] std::span<const V> GetValues() const
    {
        return { m_values };
    }

    [[nodiscard]] std::span<const H> GetHandles() const
    {
        return { m_handles };
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_values.size();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_values.empty();
    }

    [[nodiscard]] V* GetPtr()
    {
        return m_values.data();
    }

    [[nodiscard]] const V* GetPtr() const
    {
        return m_values.data();
    }

    // ===========================================
    //  Iterator
    // ===========================================

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
    [[nodiscard]] size_t FindDenseOrNullIndex_(
        const H _handle) const
    {
        const size_t index = _handle.GetIndex();
        if (index >= m_sparse.size())
        {
            return handle_storage_detail::kNullIndex;
        }

        const size_t dense = m_sparse[index];
        if (dense == handle_storage_detail::kNullIndex || m_handles[dense] != _handle)
        {
            return handle_storage_detail::kNullIndex;
        }

        return dense;
    }

    Vector<size_t> m_sparse  = {};   // dense index 저장
    Vector<H>      m_handles = {};   // 핸들을 연속적으로 저장
    Vector<V>      m_values  = {};   // 값을 연속적으로 저장
};

}   // namespace jug