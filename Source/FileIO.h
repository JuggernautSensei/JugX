#pragma once
#include <filesystem>

#include "FileFormatter.h"
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
    [[nodiscard]] static FileIOResult<FileReader> Open(const std::filesystem::path& _path);

    // 최대 _outBuffer의 크기만큼 읽고 실제로 읽은 바이트 수를 반환함.
    FileIOResult<size_t> Read(MutableMemoryView _outBuffer) const;
    FileIOResult<size_t> Read(MutableMemoryView _outBuffer, char _delimiter) const;

    bool IsEOF() const;

private:
    FileReader() = default;
    eFileIOError Open_(const std::filesystem::path& _path);
};

// ===========================================================
//  File Writer
// ===========================================================

class FileWriter : public FileIO
{
public:
    [[nodiscard]] static FileIOResult<FileWriter> Open(const std::filesystem::path& _path, bool _bAppend = false);

    FileIOResult<size_t> Write(MemoryView _mem) const;
    FileIOResult<size_t> Write(const char* _str) const;   // for string literal
    FileIOResult<size_t> Write(std::string_view _format, std::format_args _args) const;

    template<typename... TArgs>
    FileIOResult<size_t> Write(std::format_string<TArgs...> _format, TArgs&&... _args) const
    {
        return Format(m_pFile, _format, std::forward<TArgs>(_args)...);
    }

    eFileIOError Flush() const;

private:
    FileWriter() = default;
    eFileIOError Open_(const std::filesystem::path& _path, bool _bAppend);
};

}   // namespace jug
