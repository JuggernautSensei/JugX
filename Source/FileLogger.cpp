#include "FileLogger.h"

#include "FileFormatter.h"
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
    JUG_DISPATCH_FAILED(writer);
    return FileLogger { writer.Take() };
}

void FileLogger::Flush()
{
    m_writer.Flush();
}

void FileLogger::LogImpl(
    const eLogLevel,
    const std::string_view _msg,
    const bool             _bEndLog)
{
    m_writer.Write(_msg);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

void FileLogger::VFormatImpl(
    const eLogLevel,
    const std::string_view _msg,
    const std::format_args _args,
    const bool             _bEndLog)
{
    m_writer.Write(_msg, _args);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

}   // namespace jug
