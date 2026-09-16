#pragma once
#include "Error.h"

namespace jug
{

enum class eSystemError
{
    Errno,
    OS,
};

struct ErrnoErrorCategory : public IErrorCategory
{
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeMessage(int _err) const override;
};

struct OsErrorCategory : public IErrorCategory
{
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeMessage(int _err) const override;
};

[[nodiscard]] Error MakeSystemError(int _err, eSystemError _type);
[[nodiscard]] Error MakeSystemError(std::errc _err);

}   // namespace jug