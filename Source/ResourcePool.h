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

template<typename T, typename V>
class ResourcePool
{
    using HandleT          = Handle<T>;
    using HandleAllocatorT = HandleAllocator<T>;
    using HandleStorageT   = HandleStorage<T, V>;

public:
    using Iterator      = typename HandleStorageT::Iterator;
    using ConstIterator = typename HandleStorageT::ConstIterator;

    [[nodiscard]] HandleT Insert(
        const V& _resource)
    {
        return Emplace(_resource).first;
    }

    [[nodiscard]] HandleT Insert(
        V&& _resource)
    {
        return Emplace(std::move(_resource)).first;
    }

    template<typename... Args>
    [[nodiscard]] std::pair<HandleT, V&> Emplace(
        Args&&... _args)
    {
        const HandleT handle   = m_handleAllocator.Alloc();
        V&            resource = m_storage.Emplace(handle, std::forward<Args>(_args)...);
        return { handle, resource };
    }

    void Erase(
        const HandleT _handle)
    {
        m_storage.Erase(_handle);
        m_handleAllocator.Free(_handle);
    }

    [[nodiscard]] bool IsValid(
        const HandleT _handle) const
    {
        return m_handleAllocator.IsValid(_handle);
    }

    void Clear()
    {
        m_storage.Clear();
        m_handleAllocator.Clear();
    }

    [[nodiscard]] V& Get(
        const HandleT _handle)
    {
        return m_storage.Get(_handle);
    }

    [[nodiscard]] const V& Get(
        const HandleT _handle) const
    {
        return m_storage.Get(_handle);
    }

    [[nodiscard]] V* GetOrNull(
        const HandleT _handle)
    {
        return m_storage.GetOrNull(_handle);
    }

    [[nodiscard]] const V* GetOrNull(
        const HandleT _handle) const
    {
        return m_storage.GetOrNull(_handle);
    }

    [[nodiscard]] V& operator[](
        const HandleT _handle)
    {
        return Get(_handle);
    }

    [[nodiscard]] const V& operator[](
        const HandleT _handle) const
    {
        return Get(_handle);
    }

    [[nodiscard]] Span<V> GetResources()
    {
        return m_storage.GetValues();
    }

    [[nodiscard]] Span<const V> GetResources() const
    {
        return m_storage.GetValues();
    }

    [[nodiscard]] Span<const HandleT> GetHandles() const
    {
        return m_storage.GetHandles();
    }

    [[nodiscard]] size_t GetSize() const
    {
        return m_storage.GetSize();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_storage.IsEmpty();
    }

    [[nodiscard]] V* GetPtr()
    {
        return m_storage.GetPtr();
    }

    [[nodiscard]] const V* GetPtr() const
    {
        return m_storage.GetPtr();
    }

    // ===========================================
    //  Iterator
    // ===========================================

    [[nodiscard]] Iterator Begin()
    {
        return m_storage.Begin();
    }

    [[nodiscard]] Iterator End()
    {
        return m_storage.End();
    }

    [[nodiscard]] ConstIterator Begin() const
    {
        return m_storage.Begin();
    }

    [[nodiscard]] ConstIterator End() const
    {
        return m_storage.End();
    }

    [[nodiscard]] ConstIterator CBegin() const
    {
        return m_storage.CBegin();
    }

    [[nodiscard]] ConstIterator CEnd() const
    {
        return m_storage.CEnd();
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
    HandleStorageT   m_storage         = {};
};

}   // namespace jug