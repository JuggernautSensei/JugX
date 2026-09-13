#include "FileReaderWriter.h"

#include <cstdint>
#include <stdio.h>
#include <cstdio>
#include <utility>

#include "Assertion.h"

namespace jug
{

// ===============================================
//  File IO
// ===============================================

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

void FileReaderWriter::Seek(
    const int64_t     _offset,
    const eSeekOrigin _origin) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");

    int origin = 0;
    switch (_origin)
    {
        case eSeekOrigin::Set:
            origin = SEEK_SET;
            break;
        case eSeekOrigin::Cur:
            origin = SEEK_CUR;
            break;
        case eSeekOrigin::End:
            origin = SEEK_END;
            break;
        default:
            JUG_ASSERT(false, "Invalid seek origin.\n");
    }

    const int err = ::_fseeki64(m_pFile, _offset, origin);
    JUG_ASSERT(err == 0, "Failed to seek file position.\n");
}

int64_t FileReaderWriter::Tell() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");

    const int64_t n = ::_ftelli64(m_pFile);
    JUG_ASSERT(n != -1, "Failed to tell file position.\n");
    return n;
}

int64_t FileReaderWriter::GetSize() const
{
    const int64_t cur = Tell();
    Seek(0, eSeekOrigin::End);
    const int64_t size = Tell();
    Seek(cur, eSeekOrigin::Set);
    return size;
}

int64_t FileReaderWriter::GetRemain() const
{
    const int64_t cur  = Tell();
    const int64_t size = GetSize();
    return size - cur;
}

void FileReaderWriter::Rewind() const
{
    Seek(0, eSeekOrigin::Set);
}

FILE* FileReaderWriter::GetFile() const
{
    return m_pFile;
}

bool FileReaderWriter::IsOpened() const
{
    return m_pFile;
}

void FileReaderWriter::Close()
{
    if (m_pFile)
    {
        if (::fclose(m_pFile) == EOF)
        {
            JUG_ASSERT(false, "Failed to close file.\n");
        }

        m_pFile = nullptr;
    }
}

FileReaderWriter::operator bool() const
{
    return IsOpened();
}

}   // namespace jug