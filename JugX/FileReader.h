#pragma once
#include "FileReaderWriter.h"
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

    bool IsEOF() const;

private:
    FileReader() = default;
    Error Open_(const FilePath& _path);
};

}   // namespace jug