#pragma once
#include "Buffer.h"
#include "FileReaderWriter.h"
#include "MemoryView.h"
#include "Result.h"

namespace jug
{

class FileReader : public FileReaderWriter
{
public:
    [[nodiscard]] static Result<FileReader> Open(const FilePath& _path);

    // 최대 _outBuffer의 크기만큼 읽고 실제로 읽은 바이트 수를 반환함.
    Result<size_t> Read(MutableMemoryView _outBuffer) const;
    Result<size_t> Read(MutableMemoryView _outBuffer, char _delimiter) const;
    bool           IsEOF() const;

private:
    FileReader() = default;
    Error Open_(const FilePath& _path);
};

// 파일 전체를 읽음. 빈 파일은 에러가 아니라 빈 버퍼 (판단은 호출자 몫)
[[nodiscard]] Result<ByteBuffer> ReadFile(const FilePath& _path);

}   // namespace jug