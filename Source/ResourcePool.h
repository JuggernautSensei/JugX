#pragma once
#include "HandleAllocator.h"
#include "HandleMap.h"

namespace jug
{

// ================================================
//  Resource Pool
//   Handle의 발급과 Resource 관리를 동시에 하는 컨테이너.
// ================================================

template<
    typename TTag,
    typename TResource,
    template<typename...> class TAllocatorVector = std::vector,
    template<typename...> class TStorageVector   = std::vector>
class ResourcePool
{
    using Handle          = Handle<TTag>;
    using HandleAllocator = HandleAllocator<TTag, TAllocatorVector>;
    using HandleMap       = HandleMap<TTag, TResource, TStorageVector>;

public:
    using Iterator      = typename HandleMap::Iterator;
    using ConstIterator = typename HandleMap::ConstIterator;

    ResourcePool() = default;

    [[nodiscard]] Handle Insert(
        const TResource& _resource)
    {
        return Emplace(_resource);
    }

    [[nodiscard]] Handle Insert(
        TResource&& _resource)
    {
        return Emplace(std::move(_resource));
    }

    template<typename... TArgs>
    [[nodiscard]] Handle Emplace(
        TArgs&&... _args)
    {
        const Handle handle = m_allocator.Alloc();
        m_storage.Emplace(handle, std::forward<TArgs>(_args)...);
        return handle;
    }

    void Erase(
        const Handle _handle)
    {
        m_storage.Erase(_handle);
        m_allocator.Free(_handle);
    }

    [[nodiscard]] bool IsValid(
        const Handle _handle) const
    {
        return m_allocator.IsValid(_handle);
    }

    void Clear()
    {
        m_storage.Clear();
        m_allocator.Clear();
    }

    [[nodiscard]] TResource& Get(
        const Handle _handle)
    {
        return m_storage.Get(_handle);
    }

    [[nodiscard]] const TResource& Get(
        const Handle _handle) const
    {
        return m_storage.Get(_handle);
    }

    [[nodiscard]] TResource* GetOrNull(
        const Handle _handle)
    {
        return m_storage.GetOrNull(_handle);
    }

    [[nodiscard]] const TResource* GetOrNull(
        const Handle _handle) const
    {
        return m_storage.GetOrNull(_handle);
    }

    [[nodiscard]] TResource& operator[](
        const Handle _handle)
    {
        return Get(_handle);
    }

    [[nodiscard]] const TResource& operator[](
        const Handle _handle) const
    {
        return Get(_handle);
    }

    [[nodiscard]] int GetSize() const
    {
        return m_storage.GetSize();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_storage.IsEmpty();
    }

    [[nodiscard]] std::span<Handle> GetHandles()
    {
        return m_storage.GetHandles();
    }

    [[nodiscard]] std::span<const Handle> GetHandles() const
    {
        return m_storage.GetHandles();
    }

    [[nodiscard]] std::span<TResource> GetValues()
    {
        return m_storage.GetValues();
    }

    [[nodiscard]] std::span<const TResource> GetValues() const
    {
        return m_storage.GetValues();
    }

    [[nodiscard]] TResource* GetPtr()
    {
        return m_storage.GetPtr();
    }

    [[nodiscard]] const TResource* GetPtr() const
    {
        return m_storage.GetPtr();
    }

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
    //  STL like
    // ==========================================

    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] size_t size() const
    {
        return m_storage.GetSize();
    }

    [[nodiscard]] bool empty() const
    {
        return m_storage.IsEmpty();
    }

    [[nodiscard]] TResource* data()
    {
        return m_storage.GetPtr();
    }

    [[nodiscard]] const TResource* data() const
    {
        return m_storage.GetPtr();
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

private:
    HandleAllocator m_allocator = {};
    HandleMap       m_storage   = {};
};

}   // namespace jug