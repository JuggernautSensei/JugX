#pragma once
#include "FileReaderWriter.h"
#include "MemoryView.h"
#include "Result.h"

namespace jug
{

class FileWriter : public FileReaderWriter
{
public:
    [[nodiscard]] static Result<FileWriter> Open(const FilePath& _path, bool _bAppend = false);

    Result<size_t> Write(MemoryView _mem) const;
    Result<size_t> Write(const char* _str) const;
    Result<size_t> VWrite(StringView _format, std::format_args _args) const;

    template<typename... Args>
    Result<size_t> Write(
        std::format_string<Args...> _format,
        Args&&... _args) const
    {
        return VWrite(_format.get(), std::make_format_args(_args...));
    }

    Error Flush() const;

private:
    FileWriter() = default;
    Error Open_(const FilePath& _path, bool _bAppend);
};

}   // namespace jug