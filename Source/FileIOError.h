#pragma once
#include "Result.h"

namespace jug
{

enum class eFileIOError
{
    None = 0,

    AccessDenied,         // 권한 없음
    NotFound,             // 파일/경로 없음
    DiskFull,             // 디스크 공간 부족
    IOError,              // 입출력 오류
    FileTooLarge,         // 파일 크기 제한 초과
    ReadOnlyFileSystem,   // 읽기 전용 파일시스템
    TooManyOpenFiles,     // 열린 파일 개수 초과
    RuntimeError,         // 런타임 오류
    UnknownError,         // 알 수 없는 오류
};

[[nodiscard]] eFileIOError ToFileIOError(int _errorCode);

template<typename T>
using FileIOResult = Result<T, eFileIOError>;

}   // namespace jug