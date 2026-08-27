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

eFileIOError FileIO::Seek(
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
            return eFileIOError::RuntimeError;
    }

    const int err = ::_fseeki64(m_pFile, _offset, origin);
    if (err != 0)
    {
        return ToFileIOError(errno);
    }
    return eFileIOError::None;
}

FileIOResult<int64_t> FileIO::Tell() const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");

    const int64_t n = ::_ftelli64(m_pFile);
    if (n == -1)
    {
        return Failed { ToFileIOError(errno) };
    }
    return n;
}

FileIOResult<int64_t> FileIO::GetSize() const
{
    FileIOResult<int64_t> curPosOr = Tell();
    if (!curPosOr)
    {
        return curPosOr;
    }

    const eFileIOError err = Seek(0, eSeekOrigin::End);
    if (err != eFileIOError::None)
    {
        return Failed { err };
    }

    FileIOResult<int64_t> sizeOr = Tell();
    if (!sizeOr)
    {
        return sizeOr;
    }

    const eFileIOError rewindErr = Seek(curPosOr.GetValue(), eSeekOrigin::Set);
    if (rewindErr != eFileIOError::None)
    {
        return Failed { rewindErr };
    }

    return sizeOr;
}

FileIOResult<int64_t> FileIO::GetRemain() const
{
    FileIOResult<int64_t> curPosOr = Tell();
    if (!curPosOr)
    {
        return curPosOr;
    }

    const eFileIOError err = Seek(0, eSeekOrigin::End);
    if (err != eFileIOError::None)
    {
        return Failed { err };
    }

    FileIOResult<int64_t> sizeOr = Tell();
    if (!sizeOr)
    {
        return sizeOr;
    }

    const eFileIOError rewindErr = Seek(curPosOr.GetValue(), eSeekOrigin::Set);
    if (rewindErr != eFileIOError::None)
    {
        return Failed { rewindErr };
    }

    return sizeOr.GetValue() - curPosOr.GetValue();
}

eFileIOError FileIO::Rewind() const
{
    return Seek(0, eSeekOrigin::Set);
}

FILE* FileIO::GetFile() const
{
    return m_pFile;
}

bool FileIO::IsOpened() const
{
    return m_pFile != nullptr;
}

eFileIOError FileIO::Close()
{
    if (m_pFile)
    {
        const int err = ::fclose(m_pFile);
        m_pFile       = nullptr;
        if (err != 0)
        {
            return ToFileIOError(err);
        }
    }
    return eFileIOError::None;
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

    const size_t bufSize   = _outBuffer.GetSize();
    const size_t elemCount = bufSize;
    const size_t read      = ::fread_s(_outBuffer.GetPtr(), bufSize, 1, elemCount, m_pFile);
    if (read < elemCount && !::feof(m_pFile))
    {
        return Failed { ToFileIOError(errno) };
    }
    return read;
}

FileIOResult<size_t> FileReader::ReadUntil(
    const MutableMemoryView _outBuffer,
    const char              _delimiter) const
{
    JUG_ASSERT(m_pFile, "File is not open.\n");
    JUG_ASSERT(_outBuffer.GetSize() > 0, "Output buffer size is zero.\n");

    const size_t bufSize   = _outBuffer.GetSize();
    const size_t elemCount = bufSize;   // 읽을 수 있는 최대 크기 계산
    char*        buffer    = reinterpret_cast<char*>(_outBuffer.GetPtr());
    size_t       read      = 0;

    while (read < elemCount)
    {
        const int c = ::fgetc(m_pFile);
        if (c == EOF)
        {
            if (::feof(m_pFile))
            {
                break;   // End of file reached
            }
            return Failed { ToFileIOError(errno) };
        }

        if (c == _delimiter)
        {
            break;   // Delimiter found
        }
        buffer[read++] = static_cast<char>(c);
    }
    return read;
}

FileIOResult<size_t> FileReader::ReadLine(
    const MutableMemoryView _outBuffer) const
{
    return ReadUntil(_outBuffer, '\n');
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

    const size_t bufSize   = _mem.GetSize();
    const size_t elemCount = bufSize;
    const size_t written   = ::fwrite(_mem.GetPtr(), 1, elemCount, m_pFile);
    if (written < elemCount)
    {
        return Failed { ToFileIOError(errno) };
    }
    return written;
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

FileIOResult<size_t> FileWriter::Write(
    const char _ch) const
{
    return Write(_ch);
}

FileIOResult<size_t> FileWriter::WriteLine(
    const std::string_view _str) const
{
    size_t n = 0;

    // write string 
    if (!_str.empty())
    {
        FileIOResult<size_t> len0 = Write(_str);
        if (!len0)
        {
            return Failed { len0.GetError() };
        }

        n += len0.GetValue();
    }

    // write newline
    FileIOResult<size_t> len1 = Write('\n');
    if (!len1)
    {
        return Failed { len1.GetError() };
    }
    n += len1.GetValue();

    return n;
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