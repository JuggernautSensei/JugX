#include "FileWriter.h"

#include "CoreLogger.h"
#include "Assertion.h"
#include "Error.h"
#include "Result.h"
#include "MemoryView.h"
#include "SystemError.h"
#include <cstdio>
#include <cerrno>
#include <cstddef>
#include "Typedef.h"
#include <format>
#include <iterator>
#include <corecrt.h>
#include <stdio.h>

namespace jug
{

namespace
{
    struct Formatter
    {
        using value_type = char;

        explicit Formatter(
            FILE* _pFile)
            : pFile(_pFile)
        {
        }

        void push_back(
            const char _value)
        {
            if (error)
            {
                return;
            }

            if (::fputc(_value, pFile) == EOF)
            {
                JUG_CORE_LOG_ERROR("Failed to write to file.");
                error = MakeSystemError(errno, eSystemError::Errno);
            }
            else
            {
                ++written;
            }
        }

        FILE*  pFile   = nullptr;
        size_t written = 0;
        Error  error   = kOK;
    };
}   // namespace

Result<FileWriter> FileWriter::Open(
    const FilePath& _path,
    const bool      _bAppend)
{
    FileWriter  writer = {};
    const Error err    = writer.Open_(_path, _bAppend);
    if (err)
    {
        return err;
    }
    return writer;
}

Result<size_t> FileWriter::Write(
    const MemoryView _mem) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_mem.GetSize() > 0, "Binary data size is zero.\n");

    const size_t size    = _mem.GetSize();
    const size_t written = ::fwrite(_mem.GetPtr(), 1, size, m_pFile);
    if (written < size)
    {
        JUG_CORE_LOG_ERROR("Failed to write to file. Expected {} bytes, but wrote {} bytes.", size, written);
        return MakeSystemError(errno, eSystemError::Errno);
    }
    return written;
}

Result<size_t> FileWriter::Write(
    const char* _str) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_str, "String is null.\n");

    const int written = ::fputs(_str, m_pFile);
    if (written == EOF)
    {
        JUG_CORE_LOG_ERROR("Failed to write string to file.");
        return MakeSystemError(errno, eSystemError::Errno);
    }
    return written;
}

Result<size_t> FileWriter::VWrite(
    const StringView       _format,
    const std::format_args _args) const
{
    Formatter formatter { m_pFile };
    std::vformat_to(std::back_inserter(formatter), _format, _args);
    if (formatter.error.IsOK())
    {
        return formatter.written;
    }
    return formatter.error;
}

Error FileWriter::Flush() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    if (std::fflush(m_pFile) != 0)
    {
        JUG_CORE_LOG_ERROR("Failed to flush file.");
        return MakeSystemError(errno, eSystemError::Errno);
    }
    return kOK;
}

Error FileWriter::Open_(
    const FilePath& _path,
    const bool      _bAppend)
{
    const errno_t err = ::fopen_s(&m_pFile, _path.string().c_str(), _bAppend ? "ab" : "wb");
    if (err != 0)
    {
        JUG_CORE_LOG_ERROR("Failed to open file '{}'.", _path.string());
        return MakeSystemError(err, eSystemError::Errno);
    }
    return kOK;
}

}   // namespace jug