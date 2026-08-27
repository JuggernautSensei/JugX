#pragma once
#include <memory>

namespace jug
{

// ===============================================================
//  Scoped
//   smart pointer like std::unique_ptr
//   zero-overhead, no virtual function, no RTTI, no exception
// ===============================================================

template<typename T, typename Delete = std::default_delete<T>>
class Scoped
{
    template<typename U, typename E>
    friend class Scoped;

public:
    using ValueT   = T;
    using DeleterT = Delete;

    Scoped() noexcept
        : m_pObj(nullptr)
        , m_delete()
    {
    }

    /* implicit */ Scoped(
        const nullptr_t)
        : Scoped()
    {
    }

    /* implicit */ Scoped(
        T* _pObj)
        : m_pObj(_pObj)
        , m_delete()
    {
    }

    /* implicit */ Scoped(
        T*            _pObj,
        const Delete& _deleter) noexcept
        : m_pObj(_pObj)
        , m_delete(_deleter)
    {
    }

    /* implicit */ Scoped(
        T*       _pObj,
        Delete&& _deleter) noexcept
        : m_pObj(_pObj)
        , m_delete(std::move(_deleter))
    {
    }

    Scoped(const Scoped&)            = delete;
    Scoped& operator=(const Scoped&) = delete;

    Scoped(
        Scoped&& _other) noexcept
        : m_pObj(_other.Release())
        , m_delete(std::move(_other.m_delete))
    {
    }

    template<typename U, typename E>
        requires(std::is_constructible_v<U*, T*> && std::is_constructible_v<E, Delete>)
    /* implicit */ Scoped(
        Scoped<U, E>&& _other) noexcept
        : m_pObj(static_cast<T*>(_other.Release()))
        , m_delete(std::forward<E>(_other.GetDeleter()))
    {
    }

    Scoped& operator=(
        Scoped&& _other) noexcept
    {
        if (this != &_other)
        {
            Reset(_other.Release());
            m_delete = std::move(_other.m_delete);
        }
        return *this;
    }

    template<typename U, typename E>
        requires(std::is_constructible_v<U*, T*> && std::is_constructible_v<E, Delete>)
    Scoped& operator=(
        Scoped<U, E>&& _other) noexcept
    {
        Reset(static_cast<T*>(_other.Release()));
        m_delete = std::forward<E>(_other.GetDeleter());
        return *this;
    }

    ~Scoped()
    {
        Reset();
    }

    Scoped& operator=(
        const nullptr_t)
    {
        Reset();
        return *this;
    }

    [[nodiscard]] T& operator*() const
    {
        return *m_pObj;
    }

    [[nodiscard]] T* operator->() const noexcept
    {
        return m_pObj;
    }

    explicit operator bool() const noexcept
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const noexcept
    {
        return m_pObj == nullptr;
    }

    [[nodiscard]] bool operator!=(
        const nullptr_t) const noexcept
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] bool operator==(
        const Scoped& _other) const noexcept
    {
        return m_pObj == _other.m_pObj;
    }

    [[nodiscard]] bool operator!=(
        const Scoped& _other) const noexcept
    {
        return m_pObj != _other.m_pObj;
    }

    [[nodiscard]] T* GetPtr() const noexcept
    {
        return m_pObj;
    }

    [[nodiscard]] T* Release() noexcept
    {
        T* ptr = m_pObj;
        m_pObj = nullptr;
        return ptr;
    }

    void Reset(
        T* _pObj = nullptr) noexcept
    {
        T* pOld = m_pObj;
        m_pObj  = _pObj;
        if (pOld)
        {
            m_delete(pOld);
        }
    }

    void Reset(
        T*            _pObj,
        const Delete& _deleter) noexcept
    {
        if (m_pObj)
        {
            m_delete(m_pObj);
        }

        m_pObj   = _pObj;
        m_delete = _deleter;
    }

    void Reset(
        T*       _pObj,
        Delete&& _deleter) noexcept
    {
        if (m_pObj)
        {
            m_delete(m_pObj);
        }

        m_pObj   = _pObj;
        m_delete = std::move(_deleter);
    }

    [[nodiscard]] Delete& GetDeleter() noexcept
    {
        return m_delete;
    }

    [[nodiscard]] const Delete& GetDeleter() const noexcept
    {
        return m_delete;
    }

    void Swap(
        Scoped& _other) noexcept
    {
        using std::swap;
        swap(m_pObj, _other.m_pObj);
        swap(m_delete, _other.m_delete);
    }

    // 다운 캐스팅. 최소한의 제약만 체크하기 때문에 위험.
    // 커스텀 Delete가 필요하면 매개변수로 직접 주입.
    // 캐스팅 후 기존 Scoped는 무효화 됨.
    template<typename U, typename E = std::default_delete<U>>
        requires std::is_base_of_v<T, U>
    [[nodiscard]] Scoped<U, E> As(E&& _deleter = E {})
    {
        return Scoped<U, E> { static_cast<U*>(Release()), std::forward<E>(_deleter) };
    }

private:
    T*                           m_pObj   = nullptr;
    [[no_unique_address]] Delete m_delete = Delete {};
};

template<typename T, typename Delete>
class Scoped<T[], Delete>
{
public:
    using ValueT   = T;
    using DeleterT = Delete;

    Scoped() noexcept
        : m_pObj(nullptr)
        , m_delete()
    {
    }

    ~Scoped()
    {
        Reset();
    }

    /* implicit */ Scoped(
        const nullptr_t)
        : Scoped()
    {
    }

    /* implicit */ Scoped(
        T* _pObj)
        : m_pObj(_pObj)
        , m_delete()
    {
    }

    /* implicit */ Scoped(
        T*            _pObj,
        const Delete& _deleter) noexcept
        : m_pObj(_pObj)
        , m_delete(_deleter)
    {
    }

    /* implicit */ Scoped(
        T*       _pObj,
        Delete&& _deleter) noexcept
        : m_pObj(_pObj)
        , m_delete(std::move(_deleter))
    {
    }

    Scoped(const Scoped&) = delete;

    Scoped(
        Scoped&& _other) noexcept
        : m_pObj(_other.Release())
        , m_delete(std::move(_other.m_delete))
    {
    }

    template<typename U, typename E>
        requires(std::is_constructible_v<U*, T*> && std::is_constructible_v<E, Delete>)
    /* implicit */ Scoped(
        Scoped<U[], E>&& _other) noexcept
        : m_pObj(static_cast<T*>(_other.Release()))
        , m_delete(std::forward<E>(_other.GetDeleter()))
    {
    }

    Scoped& operator=(const Scoped&) = delete;

    Scoped& operator=(
        Scoped&& _other) noexcept
    {
        if (this != &_other)
        {
            Reset(_other.Release());
            m_delete = std::move(_other.m_delete);
        }
        return *this;
    }

    template<typename U, typename E>
        requires(std::is_constructible_v<U*, T*> && std::is_constructible_v<E, Delete>)
    Scoped& operator=(
        Scoped<U[], E>&& _other) noexcept
    {
        Reset(static_cast<T*>(_other.Release()));
        m_delete = std::forward<E>(_other.GetDeleter());
        return *this;
    }

    Scoped& operator=(
        const nullptr_t)
    {
        Reset();
        return *this;
    }

    [[nodiscard]] T* GetPtr() const noexcept
    {
        return m_pObj;
    }

    [[nodiscard]] T* Release() noexcept
    {
        T* p   = m_pObj;
        m_pObj = nullptr;
        return p;
    }

    void Reset(T* _pObj = nullptr) noexcept
    {
        T* old = m_pObj;
        m_pObj = _pObj;
        if (old)
        {
            m_delete(old);
        }
    }

    void Reset(
        T*            _pObj,
        const Delete& _deleter) noexcept
    {
        m_delete = _deleter;
        Reset(_pObj);
    }

    void Reset(
        T*       _pObj,
        Delete&& _deleter) noexcept
    {
        m_delete = std::move(_deleter);
        Reset(_pObj);
    }

    [[nodiscard]] Delete& GetDeleter() noexcept
    {
        return m_delete;
    }

    [[nodiscard]] const Delete& GetDeleter() const noexcept
    {
        return m_delete;
    }

    void Swap(
        Scoped& _other) noexcept
    {
        using std::swap;
        swap(m_pObj, _other.m_pObj);
        swap(m_delete, _other.m_delete);
    }

    [[nodiscard]] T& operator[](
        const size_t _index) const
    {
        return m_pObj[_index];
    }

    explicit operator bool() const noexcept
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const noexcept
    {
        return m_pObj == nullptr;
    }

    [[nodiscard]] bool operator!=(
        const nullptr_t) const noexcept
    {
        return m_pObj != nullptr;
    }

    [[nodiscard]] bool operator==(
        const Scoped& _other) const noexcept
    {
        return m_pObj == _other.m_pObj;
    }

    [[nodiscard]] bool operator!=(
        const Scoped& _other) const noexcept
    {
        return m_pObj != _other.m_pObj;
    }

private:
    T*                           m_pObj   = nullptr;
    [[no_unique_address]] Delete m_delete = Delete {};
};

template<typename T, typename... Args>
[[nodiscard]] Scoped<T> MakeScoped(Args&&... args)
{
    return Scoped<T> { new T(std::forward<Args>(args)...) };
}

}   // namespace jug

template<typename T, typename Delete>
struct std::hash<jug::Scoped<T, Delete>>
{
    [[nodiscard]] size_t operator()(const jug::Scoped<T, Delete>& _str) const noexcept
    {
        return std::hash<size_t> {}(reinterpret_cast<size_t>(_str.GetPtr()));
    }
};