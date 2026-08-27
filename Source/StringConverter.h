#pragma once
#include <string>
#include <string_view>
#include <format>
#include <span>

namespace jug
{

struct ToUtf16Result
{
    wchar_t* pEndOrNull = nullptr;   // 변환된 문자열의 끝을 가리키는 포인터. 인자로 빈 span을 넘길 경우 nullptr이 됨
    size_t   size       = 0;         // 변환될 문자열의 길이
};

struct ToUtf8Result
{
    char*  pEndOrNull = nullptr;   // 변환된 문자열의 끝을 가리키는 포인터. 인자로 빈 span을 넘길 경우 nullptr이 됨
    size_t size       = 0;         // 변환될 문자열의 길이
};

// 버퍼의 용량이 부족할 시 잘린 문자열이 반환됨.
// pEndOrNull == nullptr          : _outBufOrEmpty가 비어있음. 변환될 문자열의 길이만 필요한 경우
// pEndOrNull - pBegin            : 변환된 문자열의 길이
// pEndOrNull - pBegin == size    : 변환된 문자열이 버퍼에 완전히 들어감
// pEndOrNull - pBegin < size     : 변환된 문자열이 버퍼에 다 들어가지 못함

// ==========================================================
//  Utf8 -> Utf16
// ==========================================================

ToUtf16Result              ToUtf16(std::span<wchar_t> _outBufOrEmpty, std::string_view _utf8, bool _bTerminate = false);
[[nodiscard]] std::wstring ToUtf16(std::string_view _str);

// ==========================================================
//  Utf16 -> Utf8
// =========================================================

ToUtf8Result              ToUtf8(std::span<char> _outBufOrEmpty, std::wstring_view _utf16, bool _bTerminate = false);
[[nodiscard]] std::string ToUtf8(std::wstring_view _str);

}   // namespace jug