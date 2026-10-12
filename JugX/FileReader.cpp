#include "pch.h"
#include "FileReader.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>

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

Result<FileReader> FileReader::Open(
    const FilePath& _path)
{
    FileReader reader = {};
    JUG_RETURN_IF_ERROR(reader.Open_(_path));
    return reader;
}

Result<size_t> FileReader::Read(
    const MutableMemoryView _outBuffer) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_outBuffer.GetSize() > 0, "Output buffer size is zero.\n");

    const size_t size = _outBuffer.GetSize();
    const size_t read = ::fread(_outBuffer.GetPtr(), 1, size, m_pFile);
    if (read < size && !::feof(m_pFile))
    {
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to read from file: {}", error.MakeMessage());
        return error;
    }
    return read;
}

Result<size_t> FileReader::Read(
    const MutableMemoryView _outBuffer,
    const char              _delimiter) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(!_outBuffer.IsEmpty(), "Output buffer is empty.\n");

    jug::LockFile(m_pFile);

    const size_t size = _outBuffer.GetSize();
    char*        buf  = reinterpret_cast<char*>(_outBuffer.GetPtr());

    size_t read = 0;
    while (read < size)
    {
        const int c = GetCharNoLock(m_pFile);
        if (c == EOF)
        {
            // End of file reached
            if (::feof(m_pFile))
            {
                break;
            }

            const Error error = MakeSystemError(errno, eSystemError::Errno);
            jug::UnlockFile(m_pFile);
            JUG_CORE_LOG_ERROR("Failed to read from file: {}", error.MakeMessage());
            return error;
        }

        // delimiter found
        if (c == _delimiter)
        {
            break;
        }
        buf[read++] = static_cast<char>(c);
    }

    jug::UnlockFile(m_pFile);
    return read;
}

bool FileReader::IsEOF() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    return ::feof(m_pFile) != 0;
}

Result<ByteBuffer> ReadFile(
    const FilePath& _path)
{
    Result<FileReader> reader = FileReader::Open(_path);
    JUG_RETURN_IF_ERROR(reader);

    const Result<size_t> size = reader->GetSize();
    JUG_RETURN_IF_ERROR(size);
    if (*size == 0)
    {
        return ByteBuffer {};
    }

    ByteBuffer           buffer  = { kNoInit, *size };
    const Result<size_t> numRead = reader->Read(MutableMemoryView { buffer.GetPtr(), buffer.GetSize() });
    JUG_RETURN_IF_ERROR(numRead);
    if (*numRead != *size)
    {
        JUG_CORE_LOG_ERROR("Failed to read the whole file '{}': expected {} bytes, but read {} bytes", _path.string(), *size, *numRead);
        return MakeSystemError(std::errc::io_error);
    }
    return std::move(buffer);
}

Error FileReader::Open_(
    const FilePath& _path)
{
#if JUG_OS_WINDOWS
    const int err = ::_wfopen_s(&m_pFile, _path.c_str(), L"rb");
#else
    m_pFile       = ::fopen(_path.c_str(), "rb");
    const int err = m_pFile ? 0 : errno;
#endif
    if (err != 0)
    {
        const Error error = MakeSystemError(err, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to open file '{}' for reading: {}", _path.string(), error.MakeMessage());
        return error;
    }
    return kOK;
}

}   // namespace jug