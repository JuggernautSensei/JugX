#include "FileReaderWriter.h"

#include <cstdarg>
#include <utility>

#include "Config.h"

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
            if (error != eFileError::None)
            {
                return;
            }

            if (::fputc(_value, pFile) == EOF)
            {
                error = eFileError::IOError;
            }
            else
            {
                ++written;
            }
        }

        FILE*        pFile   = nullptr;
        size_t       written = 0;
        eFileError error   = eFileError::None;
    };
}   // namespace

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

FileResult<FileReader> FileReader::Open(
    const FilePath& _path)
{
    FileReader         reader = {};
    const eFileError err    = reader.Open_(_path);
    if (err != eFileError::None)
    {
        return Failed { err };
    }
    return reader;
}

FileResult<size_t> FileReader::Read(
    const MutableMemoryView _outBuffer) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_outBuffer.GetSize() > 0, "Output buffer size is zero.\n");

    const size_t size = _outBuffer.GetSize();
    const size_t read = ::fread_s(_outBuffer.GetPtr(), size, 1, size, m_pFile);
    if (read < size && !::feof(m_pFile))
    {
        return Failed { ToFileError(errno) };
    }
    return read;
}

FileResult<size_t> FileReader::Read(
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

            return Failed { ToFileError(errno) };
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

eFileError FileReader::Open_(
    const FilePath& _path)
{
    const errno_t err = ::_wfopen_s(&m_pFile, _path.c_str(), L"rb");
    if (err != 0)
    {
        return ToFileError(err);
    }
    return eFileError::None;
}

// ===============================================
//  File Writer
// ===============================================

FileResult<FileWriter> FileWriter::Open(
    const FilePath& _path,
    const bool                   _bAppend)
{
    FileWriter         writer = {};
    const eFileError err    = writer.Open_(_path, _bAppend);
    if (err != eFileError::None)
    {
        return Failed { err };
    }
    return writer;
}

FileResult<size_t> FileWriter::Write(
    const MemoryView _mem) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_mem.GetSize() > 0, "Binary data size is zero.\n");

    const size_t size    = _mem.GetSize();
    const size_t written = ::fwrite(_mem.GetPtr(), 1, size, m_pFile);
    if (written < size)
    {
        return Failed { ToFileError(errno) };
    }
    return written;
}

FileResult<size_t> FileWriter::Write(
    const char* _str) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_str, "String is null.\n");

    const int written = ::fputs(_str, m_pFile);
    if (written == EOF)
    {
        return Failed { ToFileError(errno) };
    }
    return written;
}

FileResult<size_t> FileWriter::WriteV(
    const StringView       _format,
    const std::format_args _args) const
{
    Formatter formatter { m_pFile };
    std::vformat_to(std::back_inserter(formatter), _format, _args);
    if (formatter.error == eFileError::None)
    {
        return formatter.written;
    }
    return Failed { formatter.error };
}

eFileError FileWriter::Flush() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    if (std::fflush(m_pFile) != 0)
    {
        return ToFileError(errno);
    }
    return eFileError::None;
}

eFileError FileWriter::Open_(
    const FilePath& _path,
    const bool                   _bAppend)
{
    const wchar_t* mode = _bAppend ? L"ab" : L"wb";
    const errno_t  err  = ::_wfopen_s(&m_pFile, _path.c_str(), mode);
    if (err != 0)
    {
        return ToFileError(err);
    }
    return eFileError::None;
}

}   // namespace jug