#pragma once
#include "StringFormat.h"
#include "Tag.h"

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

    [[nodiscard]] virtual StringView GetName() const noexcept    = 0;
    [[nodiscard]] virtual String     MakeMessage(int _err) const = 0;
};

// ===========================================
//  Error
// ===========================================

JUG_DEFINE_TAG(OkType, kOK);

class Error
{
public:
    Error() = default;
    /* implicit */ Error(OkType);
    Error(int _errorCode, IErrorCategory& _category);

    template<ErrorCodeT T>
    /* implicit */ Error(T _errorCode)
        : m_errorCode(ErrorCodeTraits<T>::GetErrorCode(_errorCode))
        , m_pCategory(ErrorCodeTraits<T>::GetCategory())
    {
    }

    void Reset();

    [[nodiscard]] bool            IsError() const;
    [[nodiscard]] bool            IsOK() const;
    [[nodiscard]] IErrorCategory* GetCategory() const;
    [[nodiscard]] int             GetErrorCode() const;
    [[nodiscard]] String          MakeMessage() const;

    explicit           operator bool() const;
    [[nodiscard]] bool operator==(Error _other) const;

private:
    int             m_errorCode = 0;
    IErrorCategory* m_pCategory = nullptr;
};

// ===========================================
//  Utils
// ===========================================

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
                                                  \
        static jug::IErrorCategory* GetCategory() \
        {                                         \
            static _category s_category;          \
            return &s_category;                   \
        }                                         \
    }
