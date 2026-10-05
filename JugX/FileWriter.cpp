#include "pch.h"
#include "FileWriter.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <format>
#include <iterator>

#include "Assert.h"
#include "CoreLogger.h"
#include "Error.h"
#include "File.h"
#include "MemoryView.h"
#include "Platform.h"
#include "Result.h"
#include "SystemError.h"
#include "Typedef.h"

namespace jug
{

namespace
{
    class Formatter
    {
        JUG_CLASS(Formatter, NO_COPY, NO_MOVE)

    public:
        using value_type = char;

        explicit Formatter(
            FILE* _pFile)
            : m_pFile(_pFile)
        {
            JUG_ASSERT(m_pFile, "File is null.\n");
            jug::LockFile(m_pFile);
        }

        ~Formatter()
        {
            jug::UnlockFile(m_pFile);
        }

        void push_back(
            const char _ch)
        {
            if (m_error)
            {
                return;
            }

            if (PutCharNoLock(m_pFile, _ch) == EOF)
            {
                m_error = MakeSystemError(errno, eSystemError::Errno);
                JUG_CORE_LOG_ERROR("Failed to write to file: {}", m_error.MakeMessage());
                return;
            }
            ++m_written;
        }

        [[nodiscard]] size_t GetWritten() const
        {
            return m_written;
        }

        [[nodiscard]] Error GetError() const
        {
            return m_error;
        }

    private:
        FILE*  m_pFile   = nullptr;
        size_t m_written = 0;
        Error  m_error   = kOK;
    };
}   // namespace

Result<FileWriter> FileWriter::Open(
    const FilePath& _path,
    const bool      _bAppend)
{
    FileWriter writer = {};
    JUG_RETURN_IF_ERROR(writer.Open_(_path, _bAppend));
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
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to write to file. Expected {} bytes, but wrote {} bytes: {}", size, written, error.MakeMessage());
        return error;
    }
    return written;
}

Result<size_t> FileWriter::Write(
    const char* _str) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_str, "String is null.\n");

    const size_t size = std::strlen(_str);
    if (size == 0)
    {
        return size_t { 0 };
    }
    return Write(MemoryView { _str, size });
}

Result<size_t> FileWriter::VWrite(
    const StringView       _format,
    const std::format_args _args) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");

    Formatter formatter { m_pFile };
    std::vformat_to(std::back_inserter(formatter), _format, _args);
    JUG_RETURN_IF_ERROR(formatter.GetError());
    return formatter.GetWritten();
}

Error FileWriter::Flush() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    if (std::fflush(m_pFile) != 0)
    {
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to flush file: {}", error.MakeMessage());
        return error;
    }
    return kOK;
}

Error FileWriter::Open_(
    const FilePath& _path,
    const bool      _bAppend)
{
#if JUG_OS_WINDOWS
    const int err = ::_wfopen_s(&m_pFile, _path.c_str(), _bAppend ? L"ab" : L"wb");
#else
    m_pFile       = ::fopen(_path.c_str(), _bAppend ? "ab" : "wb");
    const int err = m_pFile ? 0 : errno;
#endif
    if (err != 0)
    {
        const Error error = MakeSystemError(err, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to open file '{}' for {}: {}", _path.string(), _bAppend ? "appending" : "writing", error.MakeMessage());
        return error;
    }
    return kOK;
}

}   // namespace jug