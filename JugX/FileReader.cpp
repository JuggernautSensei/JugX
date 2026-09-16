#include "pch.h"
#include "FileReader.h"

#include <cerrno>
#include <corecrt.h>
#include <corecrt_wstdio.h>
#include <cstddef>
#include <cstdio>
#include <stdio.h>

#include "Assert.h"
#include "CoreLogger.h"
#include "Error.h"
#include "MemoryView.h"
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
    const size_t read = ::fread_s(_outBuffer.GetPtr(), size, 1, size, m_pFile);
    if (read < size && !::feof(m_pFile))
    {
        JUG_CORE_LOG_ERROR("Failed to read from file. Error code: {}", errno);
        return MakeSystemError(errno, eSystemError::Errno);
    }
    return read;
}

Result<size_t> FileReader::Read(
    const MutableMemoryView _outBuffer,
    const char              _delimiter) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_outBuffer.GetSize() > 0, "Output buffer size is zero.\n");

    const size_t size = _outBuffer.GetSize();
    char*        buf  = reinterpret_cast<char*>(_outBuffer.GetPtr());

    size_t read = 0;
    while (read < size)
    {
        const int c = ::fgetc(m_pFile);
        if (c == EOF)
        {
            // End of file reached
            if (::feof(m_pFile))
            {
                break;
            }

            JUG_CORE_LOG_ERROR("Failed to read from file. Error code: {}", errno);
            return MakeSystemError(errno, eSystemError::Errno);
        }

        // delimiter found
        if (c == _delimiter)
        {
            break;
        }
        buf[read++] = static_cast<char>(c);
    }
    return read;
}

bool FileReader::IsEOF() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    return ::feof(m_pFile) != 0;
}

Error FileReader::Open_(
    const FilePath& _path)
{
    const errno_t err = ::_wfopen_s(&m_pFile, _path.c_str(), L"rb");
    if (err != 0)
    {
        JUG_CORE_LOG_ERROR("Failed to open file: {}. Error code: {}", _path.string(), err);
        return MakeSystemError(errno, eSystemError::Errno);
    }
    return kOK;
}

}   // namespace jug