// ReSharper disable CppPossiblyUninitializedMember
#pragma once
#include <memory>
#include <type_traits>
#include <utility>

#include "Config.h"   // NOLINT

#define DT expect_detail

namespace jug
{

namespace DT
{
    enum class eState
    {
        Invalid,
        Error,
        Value
    };
}

template<typename E>
struct Failed
{
    template<typename... TArgs>
    /* implicit */ Failed(
        std::in_place_t,
        TArgs&&... _args)
        : error(std::forward<TArgs>(_args)...)
    {
    }

    /* implicit */ Failed(
        const E& _error)
        : error(_error)
    {
    }

    /* implicit */ Failed(
        E&& _error)
        : error(std::move(_error))
    {
    }

    E error;
};

// =====================================
//  Result
// =====================================

template<typename T, typename E>
class Result
{
    template<typename U, typename F>
    friend class Result;

public:
    Result(
        const Result& _value)
        : m_state(_value.m_state)
    {
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), _value.m_value);
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
    }

    Result(
        Result&& _value) noexcept
        : m_state(_value.m_state)
    {
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), std::move(_value.m_value));
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
        }
        _value.Reset();
    }

    template<typename U, typename F>
        requires !std::is_same_v<Result, Result<U, F>> && std::is_constructible_v<T, U> && std::is_constructible_v<E, F>
    /* implicit */ Result(
        const Result<U, F>& _value)
        : m_state(_value.m_state)
    {
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), _value.m_value);
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
    }

    template<typename U, typename F>
        requires !std::is_same_v<Result, Result<U, F>> && std::is_constructible_v<T, U> && std::is_constructible_v<E, F>
    /* implicit */ Result(
        Result<U, F> && _value)
        : m_state(_value.m_state)
    {
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), std::move(_value.m_value));
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
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
            if (m_state == DT::eState::Value)
            {
                std::construct_at(std::addressof(m_value), _value.m_value);
            }
            else if (m_state == DT::eState::Error)
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
            if (m_state == DT::eState::Value)
            {
                std::construct_at(std::addressof(m_value), std::move(_value.m_value));
            }
            else if (m_state == DT::eState::Error)
            {
                std::construct_at(std::addressof(m_error), std::move(_value.m_error));
            }
            _value.Reset();
        }
        return *this;
    }

    template<typename U, typename F>
        requires(!std::is_same_v<Result, Result<U, F>> && std::is_assignable_v<T&, U> && std::is_assignable_v<E&, F>)
    Result& operator=(
        const Result<U, F>& _value)
    {
        Reset();
        m_state = _value.m_state;
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), _value.m_value);
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
        return *this;
    }

    template<typename U, typename F>
        requires(!std::is_same_v<Result, Result<U, F>> && std::is_assignable_v<T&, U> && std::is_assignable_v<E&, F>)
    Result& operator=(
        Result<U, F>&& _value)
    {
        Reset();
        m_state = _value.m_state;
        if (m_state == DT::eState::Value)
        {
            std::construct_at(std::addressof(m_value), std::move(_value.m_value));
        }
        else if (m_state == DT::eState::Error)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
        }
        _value.Reset();
        return *this;
    }

    ~Result()
    {
        Reset();
    }

    // =====================================
    //  Value
    // =====================================

    /* implicit */ Result(
        const T& _value)
        : m_state(DT::eState::Value)
    {
        std::construct_at(std::addressof(m_value), _value);
    }

    /* implicit */ Result(
        T&& _value)
        : m_state(DT::eState::Value)
    {
        std::construct_at(std::addressof(m_value), std::move(_value));
    }

    template<typename... TArgs>
    /* implicit */ Result(
        const std::in_place_t,
        TArgs&&... _args)
        : m_state(DT::eState::Value)
    {
        std::construct_at(std::addressof(m_value), std::forward<TArgs>(_args)...);
    }

    Result& operator=(
        const T& _value)
    {
        Reset();
        m_state = DT::eState::Value;
        std::construct_at(std::addressof(m_value), _value);
        return *this;
    }

    Result& operator=(
        T&& _value)
    {
        Reset();
        m_state = DT::eState::Value;
        std::construct_at(std::addressof(m_value), std::move(_value));
        return *this;
    }

    // =====================================
    //  Error
    // =====================================

    /* implicit */ Result(
        const Failed<E>& _failed)
        : m_state(DT::eState::Error)
    {
        std::construct_at(std::addressof(m_error), _failed.error);
    }

    /* implicit */ Result(
        Failed<E>&& _failed)
        : m_state(DT::eState::Error)
    {
        std::construct_at(std::addressof(m_error), std::move(_failed.error));
    }

    // ====================================
    //  Utils
    // ====================================

    [[nodiscard]] bool HasValue() const
    {
        return m_state == DT::eState::Value;
    }

    [[nodiscard]] T& GetValue()
    {
        JUG_ASSERT(m_state == DT::eState::Value, "Result does not contain a value");
        return m_value;
    }

    [[nodiscard]] const T& GetValue() const
    {
        JUG_ASSERT(m_state == DT::eState::Value, "Result does not contain a value");
        return m_value;
    }

    [[nodiscard]] T Take()
    {
        JUG_ASSERT(m_state == DT::eState::Value, "Result does not contain a value");
        T value = std::move(m_value);
        m_state = DT::eState::Invalid;
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
        return m_state == DT::eState::Error;
    }

    [[nodiscard]] E& GetError()
    {
        JUG_ASSERT(m_state == DT::eState::Error, "Result does not contain an error");
        return m_error;
    }

    [[nodiscard]] const E& GetError() const
    {
        JUG_ASSERT(m_state == DT::eState::Error, "Result does not contain an error");
        return m_error;
    }

    explicit operator bool() const
    {
        return HasValue();
    }

    void Reset()
    {
        if (m_state == DT::eState::Value)
        {
            std::destroy_at(std::addressof(m_value));
        }
        else if (m_state == DT::eState::Error)
        {
            std::destroy_at(std::addressof(m_error));
        }
        m_state = DT::eState::Invalid;
    }

private:
    union
    {
        T m_value;
        E m_error;
    };
    DT::eState m_state;
};

// =====================================
//  Result (Only Error)
// =====================================

template<typename E>
class Result<void, E>
{
public:
    Result()
        : m_bHasError(false)
    {
    }

    ~Result()
    {
        Reset();
    }

    Result(
        const Result& _value)
        : m_bHasError(_value.m_bHasError)
    {
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
    }

    Result(
        Result&& _value) noexcept
        : m_bHasError(_value.m_bHasError)
    {
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
        }
        _value.Reset();
    }

    template<typename F>
        requires !std::is_same_v<Result, Result<void, F>> && std::is_constructible_v<E, F>
    /* implicit */ Result(
        const Result<void, F>& _value)
        : m_bHasError(_value.m_bHasError)
    {
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
    }

    template<typename F>
        requires !std::is_same_v<Result, Result<void, F>> && std::is_constructible_v<E, F>
    /* implicit */ Result(
        Result<void, F> && _value)
        : m_bHasError(_value.m_bHasError)
    {
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
        }
        _value.Reset();
    }

    Result& operator=(
        const Result& _value)
    {
        if (this != &_value)
        {
            Reset();
            m_bHasError = _value.m_bHasError;
            if (m_bHasError)
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
            m_bHasError = _value.m_bHasError;
            if (m_bHasError)
            {
                std::construct_at(std::addressof(m_error), std::move(_value.m_error));
            }
            _value.Reset();
        }
        return *this;
    }

    template<typename F>
        requires(!std::is_same_v<Result, Result<void, F>> && std::is_assignable_v<E&, F>)
    Result& operator=(
        const Result<void, F>& _value)
    {
        Reset();
        m_bHasError = _value.m_bHasError;
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), _value.m_error);
        }
        return *this;
    }

    template<typename F>
        requires(!std::is_same_v<Result, Result<void, F>> && std::is_assignable_v<E&, F>)
    Result& operator=(
        Result<void, F>&& _value)
    {
        Reset();
        m_bHasError = _value.m_bHasError;
        if (m_bHasError)
        {
            std::construct_at(std::addressof(m_error), std::move(_value.m_error));
        }
        _value.Reset();
        return *this;
    }

    // =====================================
    //  Error
    // =====================================

    /* implicit */ Result(
        const Failed<E>& _failed)
        : m_bHasError(true)
    {
        std::construct_at(std::addressof(m_error), _failed.error);
    }

    /* implicit */ Result(
        Failed<E>&& _failed)
        : m_bHasError(true)
    {
        std::construct_at(std::addressof(m_error), std::move(_failed.error));
    }

    // ====================================
    //  Utils
    // ====================================

    [[nodiscard]] bool Succeeded() const
    {
        return !m_bHasError;
    }

    [[nodiscard]] bool HasError() const
    {
        return m_bHasError;
    }

    [[nodiscard]] E& GetError()
    {
        JUG_ASSERT(m_bHasError, "Result does not contain an error");
        return m_error;
    }

    [[nodiscard]] const E& GetError() const
    {
        JUG_ASSERT(m_bHasError, "Result does not contain an error");
        return m_error;
    }

    explicit operator bool() const
    {
        return !m_bHasError;
    }

    void Reset()
    {
        if (m_bHasError)
        {
            std::destroy_at(std::addressof(m_error));
            m_bHasError = false;
        }
    }

private:
    union
    {
        std::monostate m_value;
        E              m_error;
    };
    bool m_bHasError = false;
};

// ===========================================
//  Shortcut
// ===========================================

#define JUG_DISPATCH_FAILED(_result)            \
    JUG_BEGIN_MACRO_BLOCK                       \
    if (!(_result))                             \
    {                                           \
        return Failed { (_result).GetError() }; \
    }                                           \
    JUG_END_MACRO_BLOCK

#define JUG_DISPATCH_FAILED_AS(_result, _error) \
    JUG_BEGIN_MACRO_BLOCK                       \
    if (!(_result))                             \
    {                                           \
        return Failed { (_error) };             \
    }                                           \
    JUG_END_MACRO_BLOCK

}   // namespace jug

#undef DT