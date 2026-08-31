#include "FileLogger.h"

#include "Macros.h"

namespace jug
{

FileLogger::FileLogger(
    FileWriter&& _writer)
    : m_writer(std::move(_writer))
{
}

FileResult<FileLogger> FileLogger::Open(
    const FilePath& _path,
    const bool                   _bAppend)
{
    FileResult<FileWriter> writer = FileWriter::Open(_path, _bAppend);
    JUG_DISPATCH_FAILED(writer);
    return FileLogger { writer.Take() };
}

void FileLogger::Flush()
{
    m_writer.Flush();
}

void FileLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const bool             _bEndLog)
{
    m_writer.Write(_msg);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

void FileLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const std::format_args _args,
    const bool             _bEndLog)
{
    m_writer.WriteV(_msg, _args);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

}   // namespace jug
