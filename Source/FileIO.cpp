#include "FileIO.h"

#include <cstdarg>
#include <utility>

#include "Config.h"

namespace jug
{

// ===============================================
//  File IO
// ===============================================

FileIO::~FileIO()
{
    Close();
}

FileIO::FileIO(
    FileIO&& _other) noexcept
    : m_pFile(std::exchange(_other.m_pFile, nullptr))
{
}

FileIO& FileIO::operator=(
    FileIO&& _other) noexcept
{
    if (this != &_other)
    {
        Close();
        m_pFile = std::exchange(_other.m_pFile, nullptr);
    }
    return *this;
}

void FileIO::Seek(
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

int64_t FileIO::Tell() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");

    const int64_t n = ::_ftelli64(m_pFile);
    JUG_ASSERT(n != -1, "Failed to tell file position.\n");
    return n;
}

int64_t FileIO::GetSize() const
{
    const int64_t cur = Tell();
    Seek(0, eSeekOrigin::End);
    const int64_t size = Tell();
    Seek(cur, eSeekOrigin::Set);
    return size;
}

int64_t FileIO::GetRemain() const
{
    const int64_t cur  = Tell();
    const int64_t size = GetSize();
    return size - cur;
}

void FileIO::Rewind() const
{
    Seek(0, eSeekOrigin::Set);
}

FILE* FileIO::GetFile() const
{
    return m_pFile;
}

bool FileIO::IsOpened() const
{
    return m_pFile != nullptr;
}

void FileIO::Close()
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

FileIO::operator bool() const
{
    return IsOpened();
}

FileIOResult<FileReader> FileReader::Open(
    const std::filesystem::path& _path)
{
    FileReader         reader = {};
    const eFileIOError err    = reader.Open_(_path);
    if (err != eFileIOError::None)
    {
        return Failed { err };
    }
    return reader;
}

FileIOResult<size_t> FileReader::Read(
    const MutableMemoryView _outBuffer) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_outBuffer.GetSize() > 0, "Output buffer size is zero.\n");

    const size_t size = _outBuffer.GetSize();
    const size_t read = ::fread_s(_outBuffer.GetPtr(), size, 1, size, m_pFile);
    if (read < size && !::feof(m_pFile))
    {
        return Failed { ToFileIOError(errno) };
    }
    return read;
}

FileIOResult<size_t> FileReader::Read(
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

            return Failed { ToFileIOError(errno) };
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

eFileIOError FileReader::Open_(
    const std::filesystem::path& _path)
{
    const errno_t err = ::_wfopen_s(&m_pFile, _path.c_str(), L"rb");
    if (err != 0)
    {
        return ToFileIOError(err);
    }
    return eFileIOError::None;
}

// ===============================================
//  File Writer
// ===============================================

FileIOResult<FileWriter> FileWriter::Open(
    const std::filesystem::path& _path,
    const bool                   _bAppend)
{
    FileWriter         writer = {};
    const eFileIOError err    = writer.Open_(_path, _bAppend);
    if (err != eFileIOError::None)
    {
        return Failed { err };
    }
    return writer;
}

FileIOResult<size_t> FileWriter::Write(
    const MemoryView _mem) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_mem.GetSize() > 0, "Binary data size is zero.\n");

    const size_t size    = _mem.GetSize();
    const size_t written = ::fwrite(_mem.GetPtr(), 1, size, m_pFile);
    if (written < size)
    {
        return Failed { ToFileIOError(errno) };
    }
    return written;
}

FileIOResult<size_t> FileWriter::Write(
    const char* _str) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_str, "String is null.\n");

    const int written = ::fputs(_str, m_pFile);
    if (written == EOF)
    {
        return Failed { ToFileIOError(errno) };
    }
    return written;
}

FileIOResult<size_t> FileWriter::Write(
    const std::string_view _format,
    const std::format_args _args) const
{
    return VFormat(m_pFile, _format, _args);
}

eFileIOError FileWriter::Flush() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    if (std::fflush(m_pFile) != 0)
    {
        return ToFileIOError(errno);
    }
    return eFileIOError::None;
}

eFileIOError FileWriter::Open_(
    const std::filesystem::path& _path,
    const bool                   _bAppend)
{
    const wchar_t* mode = _bAppend ? L"ab" : L"wb";
    const errno_t  err  = ::_wfopen_s(&m_pFile, _path.c_str(), mode);
    if (err != 0)
    {
        return ToFileIOError(err);
    }
    return eFileIOError::None;
}

}   // namespace jug