#pragma once
#include <string>
#include <string_view>
#include <format>
#include <span>

#define DT string_converter_detail

namespace jug
{

// 용량부족시 문자열은 단순히 잘린 문자열을 리턴함
// 반환값은 실제로 쓰여진 문자열의 길이임.

size_t                    ToUtf8(std::span<char> _outUtf8, std::wstring_view _utf16, bool _bTerminate = true);
[[nodiscard]] std::string ToUtf8(std::wstring_view _utf16);
[[nodiscard]] size_t      CalcSizeToUtf8(std::wstring_view _utf16);

size_t                     ToUtf16(std::span<wchar_t> _outUtf16, std::string_view _utf8, bool _bTerminate = true);
[[nodiscard]] std::wstring ToUtf16(std::string_view _utf8);
[[nodiscard]] size_t       CalcSizeToUtf16(std::string_view _utf8);

}   // namespace jug

#undef DT