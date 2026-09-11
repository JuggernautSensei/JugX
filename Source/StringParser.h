#pragma once
#include <charconv>
#include <optional>
#include <string>

#include "CoreLogger.h"
#include "TypeTraits.h"

namespace jug
{

template<std::integral T>
[[nodiscard]] std::optional<T> ParseAs(
    const StringView _str,
    const int              _base = 10)
{
    T value;
    const auto [_, ec] = std::from_chars(_str.data(), _str.data() + _str.size(), value, _base);
    if (ec != std::errc {})
    {
        JUG_CORE_LOG_ERROR("Failed to parse '{}' as integral type: {}", _str, std::make_error_code(ec).message());
        return std::nullopt;
    }
    return value;
}

template<std::floating_point T>
[[nodiscard]] std::optional<T> ParseAs(
    const StringView  _str,
    const std::chars_format _format = std::chars_format::general)
{
    T value;
    const auto [_, ec] = std::from_chars(_str.data(), _str.data() + _str.size(), value, _format);
    if (ec != std::errc {})
    {
        JUG_CORE_LOG_ERROR("Failed to parse '{}' as floating point type: {}", _str, std::make_error_code(ec).message());
        return std::nullopt;
    }
    return value;
}

}   // namespace jug