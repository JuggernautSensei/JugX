#pragma once
#include <format>

#include "StringEncoder.h"
#include "Typedef.h"

// ===========================================
//  Wide String Formatter Specialization
// ===========================================

template<>
struct std::formatter<jug::WString, char>
{
    constexpr auto parse(   // NOLINT
        std::format_parse_context& _ctx)
    {
        auto it = _ctx.begin();
        if (it != _ctx.end() && *it != '}')
        {
            throw std::format_error("Invalid format specifier for WString");
        }
        return it;
    }

    auto format(const jug::WString& _str, std::format_context& _ctx) const
    {
        return std::format_to(_ctx.out(), "{}", jug::ToUtf8(_str));
    }
};

template<>
struct std::formatter<jug::WStringView, char>
{
    constexpr auto parse(   // NOLINT
        std::format_parse_context& _ctx)
    {
        auto it = _ctx.begin();
        if (it != _ctx.end() && *it != '}')
        {
            throw std::format_error("Invalid format specifier for WStringView");
        }
        return it;
    }

    auto format(
        const jug::WStringView& _str,
        std::format_context&    _ctx) const
    {
        return std::format_to(_ctx.out(), "{}", jug::ToUtf8(_str));
    }
};