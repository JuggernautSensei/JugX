#pragma once
#include "Error.h"

namespace jug
{

enum class eSystemError
{
    Errno,
    OS,
};

class ErrnoErrorCategory : public IErrorCategory
{
public:
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeMessage(int _err) const override;
};

class OsErrorCategory : public IErrorCategory
{
public:
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeMessage(int _err) const override;
};

[[nodiscard]] Error MakeSystemError(int _err, eSystemError _type);
[[nodiscard]] Error MakeSystemError(std::errc _err);

}   // namespace jug