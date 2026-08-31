#pragma once
#include "Result.h"
#include "Typedef.h"

namespace jug
{

struct EncodeResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이. 널 문자는 포함되지 않음.
};

// 버퍼 용량이 부족하면 잘린 문자열이 생성됨
// 유니코드 오류 발생시 _outBuffer는 빈 문자열이 생성됨.

EncodeResult ToUtf8(Span<char> _outBuffer, StringView _utf16, bool _bNullTerminated);
EncodeResult ToUtf16(Span<wchar_t> _outBuffer, StringView _utf8, bool _bNullTerminated);

[[nodiscard]] std::string  ToUtf8(StringView _utf16);
[[nodiscard]] std::wstring ToUtf16(StringView _utf8);

void AppendUtf16(std::string& _outStr, WStringView _utf16);   // Append
void AppendUtf8(std::wstring& _outStr, StringView _utf8);     // Append

}   // namespace jug
