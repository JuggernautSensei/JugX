#pragma once
#include <filesystem>

#include "FileUtils.h"
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

    eFileIOError                        Seek(int64_t _offset, eSeekOrigin _origin) const;
    FileIOResult<int64_t>               Tell() const;
    [[nodiscard]] FileIOResult<int64_t> GetSize() const;
    [[nodiscard]] FileIOResult<int64_t> GetRemain() const;
    eFileIOError                        Rewind() const;

    [[nodiscard]] FILE* GetFile() const;
    [[nodiscard]] bool  IsOpened() const;
    eFileIOError        Close();
    explicit            operator bool() const;

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
    FileIOResult<size_t> ReadUntil(MutableMemoryView _outBuffer, char _delimiter) const;
    FileIOResult<size_t> ReadLine(MutableMemoryView _outBuffer) const;

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
    eFileIOError         Flush() const;

    // string shortcut
    FileIOResult<size_t> Write(char _ch) const;
    FileIOResult<size_t> WriteLine(std::string_view _str) const;

    template<typename... TArgs>
    FileIOResult<size_t> Format(std::format_string<TArgs...> _format, TArgs&&... _args) const
    {
        return jug::Format(m_pFile, _format, std::forward<TArgs>(_args)...);
    }

private:
    FileWriter() = default;
    eFileIOError Open_(const std::filesystem::path& _path, bool _bAppend);
};

}   // namespace jug
