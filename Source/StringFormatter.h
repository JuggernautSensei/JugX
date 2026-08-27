#pragma once
#include <format>
#include <span>

#include "Config.h"

namespace jug
{

// 주의 _outBuf의 용량이 부족할 경우 오류가 아니라 단순히 잘린 문자열이 반환됨. 반환값은 실제로 쓰여진 문자열의 길이임.

template<typename... TArgs>
size_t Format(
    std::span<char>                    _outBuf,
    const std::format_string<TArgs...> _fmt,
    TArgs&&... _args)
{
    JUG_ASSERT(!_outBuf.empty(), "Format: _outBuf must not be empty");
    const auto [pEnd, _] = std::format_to_n(_outBuf.data(), _outBuf.size(), _fmt, std::forward<TArgs>(_args)...);
    return pEnd - _outBuf.data();
}

template<typename... TArgs>
size_t FormatWithTerminate(
    std::span<char>                    _outBuf,
    const std::format_string<TArgs...> _fmt,
    TArgs&&... _args)
{
    JUG_ASSERT(!_outBuf.empty(), "FormatWithTerminate: _outBuf must not be empty");
    const auto [pEnd, _] = std::format_to_n(_outBuf.data(), _outBuf.size() - 1, _fmt, std::forward<TArgs>(_args)...);
    *pEnd                = '\0';
    return pEnd - _outBuf.data();
}

}   // namespace jug