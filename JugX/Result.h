// ReSharper disable CppPossiblyUninitializedMember
#pragma once
#include <memory>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>

#include "Assertion.h"
#include "Config.h"   // NOLINT
#include "Error.h"

namespace jug
{

namespace result_detail
{
    enum class eState
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

public:
    Result(
        const Result& _value)
        : m_state(_value.m_state)
    {
        if (m_state == result_detail::eState::Value)
        {
            std::construct_at(std::addressof(m_value), _value.m_value);
        }
        else if (m_state == result_detail::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
    }

    Result(
        Result&& _value) noexcept
        : m_state(_value.m_state)
    {
        if (m_state == result_detail::eState::Value)
        {
            std::construct_at(std::addressof(m_value), std::move(_value.m_value));
        }
        else if (m_state == result_detail::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
        _value.Reset();
    }

    Result& operator=(
        const Result& _value)
    {
        if (this != &_value)
        {
            Reset();
            m_state = _value.m_state;
            if (m_state == result_detail::eState::Value)
            {
                std::construct_at(std::addressof(m_value), _value.m_value);
            }
            else if (m_state == result_detail::eState::Error)
            {
                std::construct_at(std::addressof(m_error), _value.m_error);
            }
        }
        return *this;
    }

    Result& operator=(
        Result&& _value) noexcept
    {
        if (this != &_value)
        {
            Reset();
            m_state = _value.m_state;
            if (m_state == result_detail::eState::Value)
            {
                std::construct_at(std::addressof(m_value), std::move(_value.m_value));
            }
            else if (m_state == result_detail::eState::Error)
            {
                std::construct_at(std::addressof(m_error), _value.m_error);
            }
            _value.Reset();
        }
        return *this;
    }

    ~Result()
    {
        Reset();
    }

    // =====================================
    //  Value
    // =====================================

    template<typename... Args>
        requires std::is_constructible_v<T, Args...>
    /* implicit */ Result(
        Args&&... _args)
        : m_state(result_detail::eState::Value)
    {
        std::construct_at(std::addressof(m_value), std::forward<Args>(_args)...);
    }

    template<typename... Args>
        requires std::is_constructible_v<T, Args...>
    Result& operator=(
        Args&&... _args)
    {
        Reset();
        m_state = result_detail::eState::Value;
        std::construct_at(std::addressof(m_value), std::forward<Args>(_args)...);
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
    //  Utils
    // ====================================

    [[nodiscard]] bool HasValue() const
    {
        return m_state == result_detail::eState::Value;
    }

    [[nodiscard]] T& GetValue()
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value");
        return m_value;
    }

    [[nodiscard]] const T& GetValue() const
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value");
        return m_value;
    }

    [[nodiscard]] T Take()
    {
        JUG_ASSERT(m_state == result_detail::eState::Value, "Result does not contain a value");
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
        JUG_ASSERT(m_state == result_detail::eState::Error, "Result does not contain an error");
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
            std::destroy_at(std::addressof(m_value));
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
//  Result<void>
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
        : m_error(make_error_code(_enum))
    {
    }

    // ====================================
    //  Utils
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

#define JUG_RETURN_IF_ERROR(_expr)                                \
    if (const auto& _ret = (_expr); result_detail::IsError(_ret)) \
    return result_detail::GetError(_ret)\
