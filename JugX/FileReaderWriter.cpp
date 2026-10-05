#include "pch.h"
#include "FileReaderWriter.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <utility>

#include "Assert.h"
#include "CoreLogger.h"
#include "File.h"
#include "Platform.h"
#include "Result.h"
#include "SystemError.h"

namespace jug
{

FileReaderWriter::FileReaderWriter(
    FileReaderWriter&& _other) noexcept
    : m_pFile(std::exchange(_other.m_pFile, nullptr))
{
}

FileReaderWriter& FileReaderWriter::operator=(
    FileReaderWriter&& _other) noexcept
{
    if (this != &_other)
    {
        Close();
        m_pFile = std::exchange(_other.m_pFile, nullptr);
    }
    return *this;
}

FileReaderWriter::~FileReaderWriter()
{
    Close();
}

Error FileReaderWriter::Seek(
    const int64_t     _offset,
    const eSeekOrigin _origin) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    if (jug::Seek(m_pFile, _offset, _origin) != 0)
    {
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to seek file (offset {}): {}", _offset, error.MakeMessage());
        return error;
    }
    return kOK;
}

Result<size_t> FileReaderWriter::Tell() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    const int64_t pos = jug::Tell(m_pFile);
    if (pos < 0)
    {
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to tell file position: {}", error.MakeMessage());
        return error;
    }
    return static_cast<size_t>(pos);
}

Result<size_t> FileReaderWriter::GetSize() const
{
    const Result<size_t> cur = Tell();
    JUG_RETURN_IF_ERROR(cur);
    JUG_RETURN_IF_ERROR(Seek(0, eSeekOrigin::End));
    Result<size_t> size = Tell();
    JUG_RETURN_IF_ERROR(Seek(static_cast<int64_t>(*cur), eSeekOrigin::Set));
    return size;
}

Result<size_t> FileReaderWriter::GetRemain() const
{
    const Result<size_t> cur = Tell();
    JUG_RETURN_IF_ERROR(cur);
    const Result<size_t> size = GetSize();
    JUG_RETURN_IF_ERROR(size);
    return *cur < *size ? *size - *cur : 0;
}

Error FileReaderWriter::Rewind() const
{
    return Seek(0, eSeekOrigin::Set);
}

FILE* FileReaderWriter::GetFile() const
{
    return m_pFile;
}

bool FileReaderWriter::IsOpened() const
{
    return m_pFile != nullptr;
}

Error FileReaderWriter::Close()
{
    if (!m_pFile)
    {
        return kOK;
    }

    FILE* pFile = std::exchange(m_pFile, nullptr);
    if (::fclose(pFile) == EOF)
    {
        const Error error = MakeSystemError(errno, eSystemError::Errno);
        JUG_CORE_LOG_ERROR("Failed to close file: {}", error.MakeMessage());
        return error;
    }
    return kOK;
}

FileReaderWriter::operator bool() const
{
    return IsOpened();
}

}   // namespace jug