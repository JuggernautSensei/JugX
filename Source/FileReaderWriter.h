#pragma once
#include <filesystem>

#include "Typedef.h"
#include "FileError.h"
#include "Memory.h"
#include "MemoryView.h"

namespace jug
{

enum class eSeekOrigin
{
    Set,
    Cur,
    End
};

// ==========================================================
//  FileIO
// ==========================================================

class FileIO
{
public:
    FileIO() = default;
    ~FileIO();

    FileIO(const FileIO&)            = delete;
    FileIO& operator=(const FileIO&) = delete;
    FileIO(FileIO&& _other) noexcept;
    FileIO& operator=(FileIO&& _other) noexcept;

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

// ===========================================================
//  File Reader
// ===========================================================

class FileReader : public FileIO
{
public:
    [[nodiscard]] static FileResult<FileReader> Open(const FilePath& _path);

    // 최대 _outBuffer의 크기만큼 읽고 실제로 읽은 바이트 수를 반환함.
    FileResult<size_t> Read(MutableMemoryView _outBuffer) const;
    FileResult<size_t> Read(MutableMemoryView _outBuffer, char _delimiter) const;

    bool IsEOF() const;

private:
    FileReader() = default;
    eFileError Open_(const FilePath& _path);
};

// ===========================================================
//  File Writer
// ===========================================================

class FileWriter : public FileIO
{
public:
    [[nodiscard]] static FileResult<FileWriter> Open(const FilePath& _path, bool _bAppend = false);

    FileResult<size_t> Write(MemoryView _mem) const;
    FileResult<size_t> Write(const char* _str) const;

    FileResult<size_t> WriteV(StringView _format, std::format_args _args) const;

    template<typename... TArgs>
    FileResult<size_t> Write(
        std::format_string<TArgs...> _format,
        TArgs&&... _args) const
    {
        return WriteV(_format.get(), std::make_format_args(_args...));
    }

    eFileError Flush() const;

private:
    FileWriter() = default;
    eFileError Open_(const FilePath& _path, bool _bAppend);
};

}   // namespace jug
