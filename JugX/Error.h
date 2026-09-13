#pragma once
#include <system_error>

#include "Macro.h"
#include "StringFormat.h"
#include "Typedef.h"

namespace jug
{

// ===========================================
//  Type Traits
// ===========================================

class IErrorCategory;

template<typename T>
struct ErrorCodeTraits;

template<typename T>
concept ErrorCodeT = requires(T _value) {
    { ErrorCodeTraits<T>::GetCategory() } -> std::same_as<IErrorCategory*>;
    { ErrorCodeTraits<T>::GetErrorCode(_value) } -> std::same_as<int>;
};

// ===========================================
//  Error Category
// ===========================================

class IErrorCategory
{
    JUG_CLASS(IErrorCategory, DEFAULT_COPY, DEFAULT_MOVE)

public:
    IErrorCategory()          = default;
    virtual ~IErrorCategory() = default;

    [[nodiscard]] virtual StringView GetName() const noexcept         = 0;
    [[nodiscard]] virtual String     MakeErrorMessage(int _err) const = 0;
};

// ===========================================
//  Error
// ===========================================

class Error
{
public:
    constexpr Error() = default;

    constexpr Error(
        const int       _errorCode,
        IErrorCategory& _category)
        : m_errorCode(_errorCode)
        , m_pCategory(&_category)
    {
    }

    template<ErrorCodeT T>
    /* implicit */ constexpr Error(T _errorCode)
        : m_errorCode(ErrorCodeTraits<T>::GetErrorCode(_errorCode))
        , m_pCategory(ErrorCodeTraits<T>::GetCategory())
    {
    }

    constexpr void Reset()
    {
        m_errorCode = 0;
        m_pCategory = nullptr;
    }

    [[nodiscard]] constexpr bool IsError() const
    {
        return m_errorCode != 0;
    }

    [[nodiscard]] constexpr bool IsOK() const
    {
        return !IsError();
    }

    [[nodiscard]] constexpr IErrorCategory* GetCategory() const
    {
        return m_pCategory;
    }

    [[nodiscard]] constexpr int GetErrorCode() const
    {
        return m_errorCode;
    }

    [[nodiscard]] constexpr String MakeErrorMessage() const
    {
        if (m_pCategory)
        {
            return m_pCategory->MakeErrorMessage(m_errorCode);
        }
        else if (m_errorCode != 0)
        {
            return Format("Unknown error code: {}", m_errorCode);
        }
        else
        {
            return "No error";
        }
    }

    constexpr explicit operator bool() const
    {
        return IsError();
    }

    [[nodiscard]] constexpr bool operator==(
        const Error _other) const
    {
        return m_errorCode == _other.m_errorCode && m_pCategory == _other.m_pCategory;
    }

private:
    int             m_errorCode = 0;
    IErrorCategory* m_pCategory = nullptr;
};

// ===========================================
//  Utils
// ===========================================

constexpr Error kOK = Error {};

template<typename T>
[[nodiscard]] T& GetErrorCategory()
{
    static T s_category = {};
    return s_category;
}

}   // namespace jug

#define JUG_DEFINE_ERROR_ENUM(_enum, _category)   \
    template<>                                    \
    struct jug::ErrorCodeTraits<_enum>            \
    {                                             \
        static int GetErrorCode(_enum _value)     \
        {                                         \
            return static_cast<int>(_value);      \
        }                                         \
        static jug::IErrorCategory* GetCategory() \
        {                                         \
            static _category s_category;          \
            return &s_category;                   \
        }                                         \
    }
