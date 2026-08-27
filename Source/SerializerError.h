// C++20 unified serialization library providing serialization/deserialization in various formats
#pragma once
#include <string_view>

#include "Result.h"

namespace jug
{

enum class eSerializerError
{
    None = 0,

    ParseFailed,    // 입력이 올바른 포맷이 아님
    OutputFailed,   // 출력이 올바른 포맷이 아님
    TypeMismatch,   // 요청한 타입과 실제 값의 타입이 다름
    MissingField,   // object에 해당 key가 없음
    WriteFailed,    // 직렬화 출력 실패
    FileError,      // 파일 입출력 실패
    UnknownError,
};

template<typename T>
using SerializerResult = Result<T, eSerializerError>;

}   // namespace jug
