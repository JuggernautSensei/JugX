#pragma once
#include "Error.h"
#include "Typedef.h"

namespace jug
{

enum class eSystemError
{
    Errno,
    Win32,
    HRESULT,
};

struct ErrnoErrorCategory : public IErrorCategory
{
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeErrorMessage(int _err) const override;
};

struct Win32ErrorCategory : public IErrorCategory
{
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeErrorMessage(int _err) const override;
};

struct HResultErrorCategory : public IErrorCategory
{
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeErrorMessage(int _err) const override;
};

[[nodiscard]] Error MakeSystemError(int _err, eSystemError _type);
[[nodiscard]] Error MakeSystemError(std::errc _err);

}   // namespace jug