#pragma once
#include "Error.h"

namespace jug
{

namespace result_detail
{
    enum class eState : uint8_t
    {
        Invalid,
        Error,
        Value
    };
}   // namespace result_detail

template<typename T>
class Result
{
    static_assert(!std::is_same_v<T, Error>, "Result<T> cannot be used with Error type. Use Result<void> instead.");
    static_assert(std::is_trivially_copyable_v<Error>, "Result<T> requires Error to be trivially copyable.");

public:
    // 소멸자
    ~Result()
    {
        Reset();
    }

    // 복사 생성자
    Result(
        const Result& _other)
        requires std::is_copy_constructible_v<T>
        : m_state(_other.m_state)
    {
        if (m_state == result_detail::eState::Value)
        {
            std::construct_at(std::addressof(m_value), _other.m_value);
        }
        else if (m_state == result_detail::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _other.m_error);
        }
    }

    // 이동 생성자
    Result(
        Result&& _other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_move_constructible_v<T>
        : m_state(_other.m_state)
    {
        if (m_state == result_detail::eState::Value)
        {
            std::construct_at(std::addressof(m_value), std::move(_other.m_value));
        }
        else if (m_state == result_detail::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _other.m_error);
        }
    }

    // 복사 대입 연산자
    Result& operator=(
        const Result& _other)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>)
    {
        if (this != &_other)
        {
            if (m_state == result_detail::eState::Value && _other.m_state == result_detail::eState::Value)
            {
                m_value = _other.m_value;
            }
            else
            {
                Reset();
                m_state = _other.m_state;
                if (m_state == result_detail::eState::Value)
                {
                    std::construct_at(std::addressof(m_value), _other.m_value);
                }
                else if (m_state == result_detail::eState::Error)
                {
                    std::construct_at(std::addressof(m_error), _other.m_error);
                }
            }
        }
        return *this;
    }

    // 이동 대입 연산자
    Result& operator=(
        Result&& _other) noexcept
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
    {
        if (this != &_other)
        {
            if (m_state == result_detail::eState::Value && _other.m_state == result_detail::eState::Value)
            {
                m_value = std::move(_other.m_value);
            }
            else
            {
                Reset();
                m_state = _other.m_state;
                if (m_state == result_detail::eState::Value)
                {
                    std::construct_at(std::addressof(m_value), std::move(_other.m_value));
                }
                else if (m_state == result_detail::eState::Error)
                {
                    std::construct_at(std::addressof(m_error), _other.m_error);
                }
            }
        }
        return *this;
    }

    // =====================================
    //  GetValue
    // =====================================

    /* implicit */ Result(   // NOLINT
        const T& _value)
        : m_state(result_detail::eState::Value)
    {
        std::construct_at(std::addressof(m_value), _value);
    }

    /* implicit */ Result(   // NOLINT
        T&& _value)
        : m_state(result_detail::eState::Value)
    {
        std::construct_at(std::addressof(m_value), std::move(_value));
    }

    template<typename... Args>
    explicit Result(   // NOLINT
        const std::in_place_t,
        Args&&... _args)
        : m_state(result_detail::eState::Value)
    {
        std::construct_at(std::addressof(m_value), std::forward<Args>(_args)...);
    }

    Result& operator=(
        const T& _value)
    {
        Reset();
        m_state = result_detail::eState::Value;
        std::construct_at(std::addressof(m_value), _value);
        return *this;
    }

    Result& operator=(
        T&& _value)
    {
        Reset();
        m_state = result_detail::eState::Value;
        std::construct_at(std::addressof(m_value), std::move(_value));
        return *this;
    }

    // =====================================
    //  Error
    // =====================================

    /* implicit */ Result(
        const Error _error)
        : m_state(result_detail::eState::Error)
    {
        std::construct_at(std::addressof(m_error), _error);
    }

    template<ErrorCodeT E>
    /* implicit */ Result(
        const E _enum)
        : m_state(result_detail::eState::Error)
    {
        std::construct_at(std::addressof(m_error), Error(_enum));
    }

    Result& operator=(
        const Error _error)
    {
        Reset();
        m_state = result_detail::eState::Error;
        std::construct_at(std::addressof(m_error), _error);
        return *this;
    }

    template<ErrorCodeT E>
    Result& operator=(
        const E _enum)
    {
        Reset();
        m_state = result_detail::eState::Error;
        std::construct_at(std::addressof(m_error), Error(_enum));
        return *this;
    }

    // ====================================
    //  Access
    // ====================================

    [[nodiscard]] bool HasValue() const
    {
        return m_state == result_detail::eState::Value;
    }

    [[nodiscard]] T& GetValue()
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value.");
        return m_value;
    }

    [[nodiscard]] const T& GetValue() const
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value.");
        return m_value;
    }

    [[nodiscard]] T Take()
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value.");
        T value = std::move(m_value);
        m_state = result_detail::eState::Invalid;
        std::destroy_at(std::addressof(m_value));
        return value;
    }

    [[nodiscard]] T& operator*()
    {
        return GetValue();
    }

    [[nodiscard]] const T& operator*() const
    {
        return GetValue();
    }

    [[nodiscard]] T* operator->()
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] const T* operator->() const
    {
        return std::addressof(GetValue());
    }

    [[nodiscard]] bool HasError() const
    {
        return m_state == result_detail::eState::Error;
    }

    [[nodiscard]] Error GetError() const
    {
        JUG_ASSERT(m_state == result_detail::eState::Error, "Result does not contain an error.");
        return m_error;
    }

    explicit operator bool() const
    {
        return HasValue();
    }

    void Reset()
    {
        if (m_state == result_detail::eState::Value)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                std::destroy_at(std::addressof(m_value));
            }
        }
        m_state = result_detail::eState::Invalid;
    }

private:
    union
    {
        T     m_value;
        Error m_error;
    };
    result_detail::eState m_state;
};

// =====================================
//  Result<void> (pure error)
// =====================================

template<>
class Result<void>
{
public:
    // =====================================
    //  Error
    // =====================================

    /* implicit */ Result(
        const Error _error)
        : m_error(_error)
    {
    }

    template<ErrorCodeT E>
    /* implicit */ Result(
        const E _enum)
        : m_error(_enum)
    {
    }

    // ====================================
    //  Access
    // ====================================

    [[nodiscard]] bool IsOK() const
    {
        return !HasError();
    }

    [[nodiscard]] bool HasError() const
    {
        return m_error.IsError();
    }

    [[nodiscard]] Error GetError() const
    {
        return m_error;
    }

    explicit operator bool() const
    {
        return IsOK();
    }

    void Reset()
    {
        m_error.Reset();
    }

private:
    Error m_error = kOK;
};

// ===========================================
//  Inplace Factory
// ===========================================

template<typename T, typename... Args>
[[nodiscard]] Result<T> MakeResult(
    Args&&... _args)
{
    return Result<T> { std::in_place, std::forward<Args>(_args)... };
}

// ===========================================
//  Utils
// ===========================================

namespace result_detail
{

    [[nodiscard]] bool  IsError(Error _error);
    [[nodiscard]] Error GetError(Error _error);

    template<typename T>
    [[nodiscard]] bool IsError(
        const Result<T>& _result)
    {
        return _result.HasError();
    }

    template<typename T>
    [[nodiscard]] Error GetError(
        const Result<T>& _result)
    {
        return _result.GetError();
    }

}   // namespace result_detail

}   // namespace jug

#define JUG_RETURN_IF_ERROR(_expr)                                       \
    JUG_BEGIN_MACRO_BLOCK                                                \
    if (const auto& _ret = (_expr); ::jug::result_detail::IsError(_ret)) \
    {                                                                    \
        return ::jug::result_detail::GetError(_ret);                     \
    }                                                                    \
    JUG_END_MACRO_BLOCK
