#pragma once
#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "Config.h"
#include "Align.h"
#include "RefCount.h"

namespace jug
{

// ==========================================================
//  Ref
//   reference counting smart pointer
//   different from std::shared_ptr, it does not use control block.
//   it works like boost::intrusive_ptr or Microsoft::WRL::ComPtr
//   object must inherit from RefCounted to use this class
// ==========================================================

// requires std::is_base_of_v<RefCounted, T> 제약을 안두는 이유
// 구체적인 타입을 알아야만 Ref<T>를 사용할 수 있기 때문에, 전방선언을 적극적으로 활용할 수 없음.

template<typename T>
class Ref
{
    template<typename U>
    friend class Ref;

public:
    Ref()
        : m_pObj(nullptr)
    {
    }

    /* implicit */ Ref(
        const nullptr_t)
        : Ref()
    {
    }

    /* implicit */ Ref(
        T* _ptr)
        : m_pObj(_ptr)

    {
        if (m_pObj)
        {
            m_pObj->Retain();
        }
    }

    /* implicit */ Ref(
        const Ref& _other)
        : m_pObj(_other.m_pObj)
    {
        if (m_pObj)
        {
            m_pObj->Retain();
        }
    }

    Ref(
        Ref&& _other) noexcept
        : m_pObj(_other.m_pObj)
    {
        _other.m_pObj = nullptr;
    }

    template<typename U>
        requires std::is_constructible_v<U*, T*>
    /* implicit */ Ref(
        Scoped<U>&& _other)
        : m_pObj(static_cast<T*>(_other.Release()))
    {
        if (m_pObj)
        {
            m_pObj->Retain();
        }
    }

    template<typename U>
        requires std::is_base_of_v<RefCounted, U> && std::is_constructible_v<U*, T*>
    /* implicit */ Ref(
        const Ref<U>& _other)
        : m_pObj(static_cast<T*>(_other.m_pObj))
    {
        if (m_pObj)
        {
            m_pObj->Retain();
        }
    }

    template<typename U>
        requires std::is_base_of_v<RefCounted, U> && std::is_constructible_v<U*, T*>
    /* implicit */ Ref(
        Ref<U>&& _other) noexcept
        : m_pObj(static_cast<T*>(_other.m_pObj))
    {
        _other.m_pObj = nullptr;
    }

    ~Ref()
    {
        Release_();
    }

    Ref& operator=(
        const Ref& _other)
    {
        if (this != &_other)
        {
            if (_other.m_pObj)
            {
                _other.m_pObj->Retain();
            }
            Release_();
            m_pObj = _other.m_pObj;
        }
        return *this;
    }

    Ref& operator=(
        Ref&& _other) noexcept
    {
        if (this != &_other)
        {
            Release_();
            m_pObj        = _other.m_pObj;
            _other.m_pObj = nullptr;
        }
        return *this;
    }

    Ref& operator=(
        T* _ptr)
    {
        if (m_pObj != _ptr)
        {
            if (_ptr)
            {
                _ptr->Retain();
            }
            Release_();
            m_pObj = _ptr;
        }
        return *this;
    }

    Ref& operator=(
        const nullptr_t)
    {
        Release_();
        return *this;
    }

    template<typename U>
        requires std::is_base_of_v<RefCounted, U> && std::is_constructible_v<U*, T*>
    Ref& operator=(
        Scoped<U>&& _other)
    {
        if (m_pObj != static_cast<T*>(_other.GetPtr()))
        {
            T* pOther = static_cast<T*>(_other.Release());
            if (pOther)
            {
                pOther->Retain();
            }
            Release_();
            m_pObj = pOther;
        }
        return *this;
    }

    template<typename U>
        requires std::is_base_of_v<RefCounted, U> && std::is_constructible_v<U*, T*>
    Ref& operator=(
        const Ref<U>& _other)
    {
        T* pOther = static_cast<T*>(_other.m_pObj);
        if (m_pObj != pOther)
        {
            if (pOther)
            {
                pOther->Retain();
            }
            Release_();
            m_pObj = pOther;
        }
        return *this;
    }

    template<typename U>
        requires std::is_base_of_v<RefCounted, U> && std::is_constructible_v<U*, T*>
    Ref& operator=(
        Ref<U>&& _other) noexcept
    {
        T* pOther = static_cast<T*>(_other.m_pObj);
        if (m_pObj != pOther)
        {
            Release_();
            m_pObj = pOther;
        }
        _other.m_pObj = nullptr;
        return *this;
    }

    [[nodiscard]] T* operator->() const
    {
        return m_pObj;
    }

    [[nodiscard]] T& operator*() const
    {
        return *m_pObj;
    }

    [[nodiscard]] bool operator==(
        const T* _ptr) const
    {
        return m_pObj == _ptr;
    }

    [[nodiscard]] bool operator!=(
        const T* _ptr) const
    {
        return m_pObj != _ptr;
    }

    [[nodiscard]] bool operator==(
        const Ref& _other) const
    {
        return m_pObj == _other.m_pObj;
    }

    [[nodiscard]] bool operator!=(
        const Ref& _other) const
    {
        return m_pObj != _other.m_pObj;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const
    {
        return m_pObj == nullptr;
    }

    [[nodiscard]] bool operator!=(
        const nullptr_t) const
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] bool operator<(
        const Ref& _other) const
    {
        return m_pObj < _other.m_pObj;
    }

    [[nodiscard]] bool operator>(
        const Ref& _other) const
    {
        return m_pObj > _other.m_pObj;
    }

    [[nodiscard]] bool operator<=(
        const Ref& _other) const
    {
        return m_pObj <= _other.m_pObj;
    }

    [[nodiscard]] bool operator>=(
        const Ref& _other) const
    {
        return m_pObj >= _other.m_pObj;
    }

    explicit operator bool() const
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] T* GetPtr() const
    {
        return m_pObj;
    }

    void Reset(
        T* _ptr = nullptr)
    {
        if (m_pObj != _ptr)
        {
            if (_ptr)
            {
                _ptr->Retain();
            }
            Release_();
            m_pObj = _ptr;
        }
    }

    template<typename U>
        requires(std::is_base_of_v<RefCounted, U> && std::is_base_of_v<T, U>)
    [[nodiscard]] Ref<U> As() const
    {
        // 다운 캐스팅. 최소한의 제약만 체크하기 때문에 위험
        return Ref<U> { static_cast<U*>(m_pObj) };
    }

private:
    void Release_()
    {
        if (m_pObj)
        {
            if (m_pObj->Release() == 0)
            {
                delete m_pObj;
            }
            m_pObj = nullptr;
        }
    }

    T* m_pObj = nullptr;
};

using AnyRef = Ref<RefCounted>;

template<typename T, typename... Args>
[[nodiscard]] Ref<T> MakeRef(Args&&... args)
{
    return Ref<T>(new T(std::forward<Args>(args)...));
}

}   // namespace jug

template<typename T>
struct std::hash<jug::Ref<T>>
{
    [[nodiscard]] size_t operator()(const jug::Ref<T>& _ptr) const noexcept
    {
        return std::hash<size_t> {}(reinterpret_cast<size_t>(_ptr.GetPtr()));
    }
};