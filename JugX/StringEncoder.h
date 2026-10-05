#pragma once

namespace jug
{

struct EncodeResult
{
    size_t totalSize = 0;   // 변환에 필요한 총 길이
    size_t written   = 0;   // 실제로 변환된 길이. 널 문자는 포함되지 않음.
};

EncodeResult EncodeUtf8To(Span<char> _outBuffer, WStringView _utf16, bool _bNullTerminated);
EncodeResult EncodeUtf16To(Span<wchar_t> _outBuffer, StringView _utf8, bool _bNullTerminated);

[[nodiscard]] String  EncodeUtf8(WStringView _utf16);
[[nodiscard]] WString EncodeUtf16(StringView _utf8);

void AppendUtf16(String& _out, WStringView _utf16);   // Append
void AppendUtf8(WString& _out, StringView _utf8);     // Append

}   // namespace jug
