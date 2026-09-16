#pragma once
#include "CoreLogger.h"
#include "Result.h"
#include "SystemError.h"

namespace jug
{

template<std::integral T>
[[nodiscard]] Result<T> ParseAs(
    const StringView _str,
    const int        _base = 10)
{
    T value;
    const auto [_, err] = std::from_chars(_str.data(), _str.data() + _str.size(), value, _base);
    if (err)
    {
        JUG_CORE_LOG_ERROR("Failed to parse '{}' as integral type: {}", _str, err.message());
        return MakeSystemError(static_cast<std::errc>(err.value()));
    }
    return value;
}

template<std::floating_point T>
[[nodiscard]] Result<T> ParseAs(
    const StringView        _str,
    const std::chars_format _format = std::chars_format::general)
{
    T value;
    const auto [_, err] = std::from_chars(_str.data(), _str.data() + _str.size(), value, _format);
    if (err)
    {
        JUG_CORE_LOG_ERROR("Failed to parse '{}' as floating point type: {}", _str, err.message());
        return MakeSystemError(static_cast<std::errc>(err.value()));
    }
    return value;
}

}   // namespace jug