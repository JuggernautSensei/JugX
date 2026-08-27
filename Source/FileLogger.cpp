#include "FileLogger.h"

#include "FileUtils.h"
#include "Macros.h"

namespace jug
{

FileLogger::FileLogger(
    FileWriter&& _writer)
    : m_writer(std::move(_writer))
{
}

FileIOResult<FileLogger> FileLogger::Open(
    const std::filesystem::path& _path,
    const bool                   _bAppend)
{
    FileIOResult<FileWriter> writer = FileWriter::Open(_path, _bAppend);
    if (!writer)
    {
        return Failed { writer.GetError() };
    }

    return FileLogger { writer.Take() };
}

void FileLogger::Flush()
{
    m_writer.Flush();
}

void FileLogger::LogImpl(
    const eLogLevel,
    const std::string_view _message,
    const bool             _bNewLine)
{
    m_writer.Write(_message);

    if (_bNewLine)
    {
        m_writer.Write('\n');
    }
}

void FileLogger::FormatImpl(
    const eLogLevel,
    const std::string_view _message,
    const std::format_args _args,
    const bool             _bNewLine)
{
    if (_bNewLine)
    {
        m_writer.Write('\n');
    }
}

}   // namespace jug
