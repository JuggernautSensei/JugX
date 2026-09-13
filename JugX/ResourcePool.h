#pragma once
#include "HandleAllocator.h"
#include "HandleStorage.h"
#include "Typedef.h"

namespace jug
{

// ================================================
//  Resource Pool
//   Handle의 발급과 Resource 관리를 동시에 하는 컨테이너.
// ================================================

template<HandleT H, typename V>
class ResourcePool
{
    using HandleAllocatorT = HandleAllocator<H>;
    using HandleStorageT   = HandleStorage<H, V>;

public:
    using Iterator      = typename HandleStorageT::Iterator;
    using ConstIterator = typename HandleStorageT::ConstIterator;

    [[nodiscard]] H Insert(
        const V& _resource)
    {
        return Emplace(_resource).first;
    }

    [[nodiscard]] H Insert(
        V&& _resource)
    {
        return Emplace(std::move(_resource)).first;
    }

    template<typename... Args>
    [[nodiscard]] std::pair<H, V&> Emplace(
        Args&&... _args)
    {
        const H handle   = m_handleAllocator.Alloc();
        V&      resource = m_handleStorage.Emplace(handle, std::forward<Args>(_args)...);
        return { handle, resource };
    }

    void Erase(
        const H _handle)
    {
        m_handleStorage.Erase(_handle);
        m_handleAllocator.Free(_handle);
    }

    [[nodiscard]] bool IsValid(
        const H _handle) const
    {
        return m_handleAllocator.IsValid(_handle);
    }

    void Clear()
    {
        m_handleStorage.Clear();
        m_handleAllocator.Clear();
    }

    [[nodiscard]] V& Get(
        const H _handle)
    {
        return m_handleStorage.Get(_handle);
    }

    [[nodiscard]] const V& Get(
        const H _handle) const
    {
        return m_handleStorage.Get(_handle);
    }

    [[nodiscard]] V* GetOrNull(
        const H _handle)
    {
        return m_handleStorage.GetOrNull(_handle);
    }

    [[nodiscard]] const V* GetOrNull(
        const H _handle) const
    {
        return m_handleStorage.GetOrNull(_handle);
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

    [[nodiscard]] Span<V> GetResources()
    {
        return m_handleStorage.GetValues();
    }

    [[nodiscard]] Span<const V> GetResources() const
    {
        return m_handleStorage.GetValues();
    }

    [[nodiscard]] Span<const H> GetHandles() const
    {
        return m_handleStorage.GetHandles();
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_handleStorage.GetSize();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_handleStorage.IsEmpty();
    }

    [[nodiscard]] V* GetPtr()
    {
        return m_handleStorage.GetPtr();
    }

    [[nodiscard]] const V* GetPtr() const
    {
        return m_handleStorage.GetPtr();
    }

    // ===========================================
    //  Iterator
    // ===========================================

    [[nodiscard]] Iterator Begin()
    {
        return m_handleStorage.Begin();
    }

    [[nodiscard]] Iterator End()
    {
        return m_handleStorage.End();
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return m_handleStorage.Begin();
    }

    [[nodiscard]] ConstIterator End() const
    {
        return m_handleStorage.End();
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return m_handleStorage.CBegin();
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return m_handleStorage.CEnd();
    }

    // ==========================================
    //  STL Like
    // ==========================================

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
    HandleAllocatorT m_handleAllocator = {};
    HandleStorageT   m_handleStorage   = {};
};

}   // namespace jug