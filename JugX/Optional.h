#pragma once
#include "Tag.h"

namespace jug
{

// ===========================================
//  Nullopt
// ===========================================

JUG_DEFINE_TAG(NulloptType, kNullopt);

// ===========================================
//  Basic Optional
// ===========================================

template<typename T>
class Optional
{
    static_assert(
        !std::is_reference_v<T> && !std::is_array_v<T> && std::is_object_v<T> && std::is_destructible_v<T>,
        "Optional supports only destructible non-array object types.");

public:
    constexpr Optional()
        : m_dummy()
        , m_bHasValue(false)
    {
    }

    /* implicit */ constexpr Optional(
        const NulloptType)
        : m_dummy()
        , m_bHasValue(false)
    {
    }

    /* implicit */ constexpr Optional(
        const T& _value)
        : m_value(_value)
        , m_bHasValue(true)
    {
    }

    /* implicit */ constexpr Optional(
        T&& _value)
        : m_value(std::move(_value))
        , m_bHasValue(true)
    {
    }

    template<typename... Args>
        requires std::is_constructible_v<T, Args...>
    explicit constexpr Optional(
        const std::in_place_t,
        Args&&... _args)
        : m_value(std::forward<Args>(_args)...)
        , m_bHasValue(true)
    {
    }

    constexpr ~Optional()
    {
        Reset();
    }

    // 복사 생성자
    constexpr Optional(
        const Optional& _other)
        requires std::is_copy_constructible_v<T>
        : m_dummy()
        , m_bHasValue(false)
    {
        if (_other.m_bHasValue)
        {
            std::construct_at(&m_value, _other.m_value);
            m_bHasValue = true;
        }
    }

    // 이동 생성자
    constexpr Optional(
        Optional&& _other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_move_constructible_v<T>
        : m_dummy()
        , m_bHasValue(false)
    {
        if (_other.m_bHasValue)
        {
            std::construct_at(&m_value, std::move(_other.m_value));
            m_bHasValue = true;
        }
    }

    // 복사 대입 연산자
    constexpr Optional& operator=(
        const Optional& _other)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>)
    {
        if (this != &_other)
        {
            if (m_bHasValue && _other.m_bHasValue)
            {
                m_value = _other.m_value;
            }
            else if (_other.m_bHasValue)
            {
                std::construct_at(&m_value, _other.m_value);
                m_bHasValue = true;
            }
            else if (m_bHasValue)
            {
                Reset();
            }
        }
        return *this;
    }

    // 이동 대입 연산자
    constexpr Optional& operator=(
        Optional&& _other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
    {
        if (this != &_other)
        {
            if (m_bHasValue && _other.m_bHasValue)
            {
                m_value = std::move(_other.m_value);
            }
            else if (_other.m_bHasValue)
            {
                std::construct_at(&m_value, std::move(_other.m_value));
                m_bHasValue = true;
            }
            else if (m_bHasValue)
            {
                Reset();
            }
        }
        return *this;
    }

    [[nodiscard]] constexpr bool HasValue() const
    {
        return m_bHasValue;
    }

    constexpr explicit operator bool() const
    {
        return m_bHasValue;
    }

    [[nodiscard]] constexpr T& GetValue()
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        return m_value;
    }

    [[nodiscard]] constexpr const T& GetValue() const
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        return m_value;
    }

    [[nodiscard]] constexpr T& operator*()
    {
        return GetValue();
    }

    [[nodiscard]] constexpr const T& operator*() const
    {
        return GetValue();
    }

    [[nodiscard]] constexpr T* operator->()
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] constexpr const T* operator->() const
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] constexpr T GetValueOr(
        const T& _fallback) const
    {
        return m_bHasValue ? m_value : _fallback;
    }

    [[nodiscard]] constexpr T Take()
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        T value = std::move(m_value);
        Reset();
        return value;
    }

    template<typename... Args>
    constexpr T& Emplace(
        Args&&... _args)
    {
        Reset();
        std::construct_at(&m_value, std::forward<Args>(_args)...);
        m_bHasValue = true;
        return m_value;
    }

    constexpr void Reset()
    {
        if (m_bHasValue)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                m_value.~T();
            }
            m_bHasValue = false;
        }
    }

private:
    union
    {
        char m_dummy;
        T    m_value;
    };
    bool m_bHasValue;
};

// ===========================================
//  Trivial Optional
// ===========================================

template<typename T>
    requires std::is_trivially_copyable_v<T>
class Optional<T>
{
    static_assert(
        !std::is_reference_v<T> && !std::is_array_v<T> && std::is_object_v<T> && std::is_destructible_v<T>,
        "Optional supports only destructible non-array object types.");

public:
    constexpr Optional()
        : m_dummy()
        , m_bHasValue(false)
    {
    }

    /* implicit */ constexpr Optional(
        const NulloptType)
        : m_dummy()
        , m_bHasValue(false)
    {
    }

    /* implicit */ constexpr Optional(
        const T& _value)
        : m_value(_value)
        , m_bHasValue(true)
    {
    }

    template<typename... Args>
        requires std::is_constructible_v<T, Args...>
    explicit constexpr Optional(
        const std::in_place_t,
        Args&&... _args)
        : m_value(std::forward<Args>(_args)...)
        , m_bHasValue(true)
    {
    }

    [[nodiscard]] constexpr bool HasValue() const
    {
        return m_bHasValue;
    }

    constexpr explicit operator bool() const
    {
        return m_bHasValue;
    }

    [[nodiscard]] constexpr T& GetValue()
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        return m_value;
    }

    [[nodiscard]] constexpr const T& GetValue() const
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        return m_value;
    }

    [[nodiscard]] constexpr T& operator*()
    {
        return GetValue();
    }

    [[nodiscard]] constexpr const T& operator*() const
    {
        return GetValue();
    }

    [[nodiscard]] constexpr T* operator->()
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] constexpr const T* operator->() const
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] constexpr T GetValueOr(
        const T& _fallback) const
    {
        return m_bHasValue ? m_value : _fallback;
    }

    [[nodiscard]] constexpr T Take()
    {
        JUG_ASSERT(m_bHasValue, "Optional has no value.\n");
        T value     = m_value;
        m_bHasValue = false;
        return value;
    }

    template<typename... Args>
    constexpr T& Emplace(
        Args&&... _args)
    {
        std::construct_at(&m_value, std::forward<Args>(_args)...);
        m_bHasValue = true;
        return m_value;
    }

    constexpr void Reset()
    {
        m_bHasValue = false;
    }

private:
    union
    {
        char m_dummy;
        T    m_value;
    };
    bool m_bHasValue;
};

// ===========================================
//  Ref Optional
// ===========================================

template<typename T>
class Optional<T&>
{
    static_assert(std::is_object_v<T>, "Optional<T&> supports only object types.");

public:
    constexpr Optional()
        : m_pValue(nullptr)
    {
    }

    /* implicit */ constexpr Optional(
        const NulloptType)
        : m_pValue(nullptr)
    {
    }

    /* implicit */ constexpr Optional(
        T& _value)
        : m_pValue(std::addressof(_value))
    {
    }

    Optional(T&&) = delete;   // NOLINT

    constexpr explicit operator bool() const
    {
        return m_pValue != nullptr;
    }

    [[nodiscard]] constexpr bool HasValue() const
    {
        return static_cast<bool>(*this);
    }

    [[nodiscard]] constexpr T& GetValue() const
    {
        JUG_ASSERT(*this, "Optional has no value.\n");
        return *m_pValue;
    }

    [[nodiscard]] constexpr T& operator*() const
    {
        return GetValue();
    }

    [[nodiscard]] constexpr T* operator->() const
    {
        return std::addressof(GetValue());
    }

    constexpr T& Emplace(
        T& _value)
    {
        m_pValue = std::addressof(_value);
        return _value;
    }

    constexpr void Reset()
    {
        m_pValue = nullptr;
    }

private:
    T* m_pValue;
};

// ===========================================
//  Comparison
// ===========================================

template<typename T>
[[nodiscard]] constexpr bool operator==(
    const Optional<T>& _lhs,
    const Optional<T>& _rhs)
{
    if (_lhs && _rhs)
    {
        return _lhs.GetValue() == _rhs.GetValue();
    }
    return !_lhs && !_rhs;
}

template<typename T>
[[nodiscard]] constexpr bool operator==(
    const Optional<T>& _lhs,
    const NulloptType)
{
    return !_lhs;
}

template<typename T>
[[nodiscard]] constexpr bool operator==(
    const Optional<T>&             _lhs,
    const std::type_identity_t<T>& _rhs)
{
    return _lhs && _lhs.GetValue() == _rhs;
}

// ===========================================
//  MakeOptional
// ===========================================

template<typename T, typename... Args>
[[nodiscard]] constexpr Optional<T> MakeOptional(
    Args&&... _args)
{
    return Optional<T> { std::in_place, std::forward<Args>(_args)... };
}

}   // namespace jug
