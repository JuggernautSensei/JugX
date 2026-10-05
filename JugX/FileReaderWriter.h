#pragma once
#include "Result.h"

namespace jug
{

enum class eSeekOrigin;

class FileReaderWriter
{
    JUG_CLASS(FileReaderWriter, NO_COPY)

public:
    FileReaderWriter() = default;
    FileReaderWriter(FileReaderWriter&& _other) noexcept;
    FileReaderWriter& operator=(FileReaderWriter&& _other) noexcept;
    ~FileReaderWriter();

    Error                        Seek(int64_t _offset, eSeekOrigin _origin) const;
    [[nodiscard]] Result<size_t> Tell() const;
    [[nodiscard]] Result<size_t> GetSize() const;
    [[nodiscard]] Result<size_t> GetRemain() const;
    Error                        Rewind() const;

    [[nodiscard]] FILE* GetFile() const;
    [[nodiscard]] bool  IsOpened() const;
    Error               Close();

    explicit operator bool() const;

protected:
    FILE* m_pFile = nullptr;
};

}   // namespace jug
