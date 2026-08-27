#pragma once
#include <vector>
#include <span>

#include "Handle.h"

#define DT handle_map_detail

namespace jug
{

namespace DT
{
    constexpr size_t kNullIndex  = SIZE_MAX;
    constexpr size_t kGrowFactor = 2;
    constexpr size_t kMinSize    = 4;
}   // namespace DT

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

template<
    typename TTag,
    typename TValue,
    template<typename...> class TVector = std::vector>
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

        using ValueT = std::conditional_t<kbIsConst, const TValue, TValue>;

    public:
        BaseIterator() = default;

        [[nodiscard]] ValueT& operator*() const
        {
            return *m_pValue;
        }

        [[nodiscard]] ValueT* operator->() const
        {
            return m_pValue;
        }

        [[nodiscard]] Handle GetHandle() const
        {
            const ptrdiff_t dense = m_pValue - m_pStorage->m_dense.data();
            return m_pStorage->m_handles[dense];
        }

        BaseIterator& operator++()
        {
            JUG_ASSERT(m_pStorage->m_dense.data() <= m_pValue && m_pValue < m_pStorage->m_dense.data() + m_pStorage->GetSize(), "Iterator is already at the end. Cannot increment further.\n");
            ++m_pValue;
            return *this;
        }

        BaseIterator operator++(
            const int)
        {
            BaseIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator!=(
            const BaseIterator& _other) const
        {
            return !(*this == _other);
        }

        bool operator==(
            const BaseIterator& _other) const
        {
            return m_pValue == _other.m_pValue;
        }

        [[nodiscard]] operator BaseIterator<true>() const
            requires(!kbIsConst)
        {
            return BaseIterator<true> { m_pStorage, m_pValue };
        }

    protected:
        constexpr BaseIterator(
            const HandleMap* _pSet,
            ValueT*          _pValue)
            : m_pStorage(_pSet)
            , m_pValue(_pValue)
        {
            JUG_ASSERT(_pSet, "HandleMap must not be null.\n");
        }

        const HandleMap* m_pStorage = nullptr;
        ValueT*          m_pValue   = nullptr;
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
            const size_t size = std::max(DT::kMinSize, index * DT::kGrowFactor);
            m_sparse.resize(size, DT::kNullIndex);
        }

        // insert
        JUG_ASSERT(m_sparse[index] == DT::kNullIndex, "Handle dense {} is already occupied.\n", idx);
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
        JUG_ASSERT(dense != DT::kNullIndex, "Handle {} does not exist in HandleMap.\n", _handle.GetValue());

        if (dense != last)
        {
            const Handle lastHandle         = m_handles[last];
            m_dense[dense]                  = std::move(m_dense[last]);
            m_handles[dense]                = lastHandle;
            m_sparse[lastHandle.GetIndex()] = dense;
        }

        m_sparse[idx] = DT::kNullIndex;
        m_dense.pop_back();
        m_handles.pop_back();
    }

    [[nodiscard]] bool Contains(
        const Handle _handle) const
    {
        // null 도 동시에 잡음
        const size_t index = _handle.GetIndex();
        if (index >= m_sparse.size() || m_sparse[index] == DT::kNullIndex)
        {
            return false;
        }

        return m_handles[m_sparse[index]] == _handle;
    }

    [[nodiscard]] TValue& Get(
        const Handle _handle)
    {
        JUG_ASSERT(Contains(_handle), "Handle {} does not exist in HandleMap.\n", _handle.GetValue());
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

    [[nodiscard]] std::span<Handle> GetHandles()
    {
        return m_handles;
    }

    [[nodiscard]] std::span<const Handle> GetHandles() const
    {
        return m_handles;
    }

    [[nodiscard]] std::span<TValue> GetValues()
    {
        return m_dense;
    }

    [[nodiscard]] std::span<const TValue> GetValues() const
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
        return Iterator { this, m_dense.data() };
    }

    [[nodiscard]] Iterator End()
    {
        return Iterator { this, m_dense.data() + GetSize() };
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return ConstIterator { this, m_dense.data() };
    }

    [[nodiscard]] ConstIterator End() const
    {
        return ConstIterator { this, m_dense.data() + GetSize() };
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return ConstIterator { this, m_dense.data() };
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return ConstIterator { this, m_dense.data() + GetSize() };
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
    TVector<size_t> m_sparse  = {};   // dense index 저장
    TVector<TValue> m_dense   = {};   // 값을 연속적으로 저장
    TVector<Handle> m_handles = {};   // dense index에 대응하는 handle 저장
};

}   // namespace jug

#undef DT