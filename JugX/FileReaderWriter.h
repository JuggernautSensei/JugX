#pragma once

namespace jug
{

enum class eSeekOrigin
{
    Set,
    Cur,
    End
};

// ==========================================================
//  FileReaderWriter
// ==========================================================

class FileReaderWriter
{
    JUG_CLASS(FileReaderWriter, NO_COPY)

public:
    FileReaderWriter() = default;
    FileReaderWriter(FileReaderWriter&& _other) noexcept;
    FileReaderWriter& operator=(FileReaderWriter&& _other) noexcept;
    ~FileReaderWriter();

    void                  Seek(int64_t _offset, eSeekOrigin _origin) const;
    int64_t               Tell() const;
    [[nodiscard]] int64_t GetSize() const;
    [[nodiscard]] int64_t GetRemain() const;
    void                  Rewind() const;

    [[nodiscard]] FILE* GetFile() const;
    [[nodiscard]] bool  IsOpened() const;
    void                Close();

    explicit operator bool() const;

protected:
    FILE* m_pFile = nullptr;
};

}   // namespace jug
