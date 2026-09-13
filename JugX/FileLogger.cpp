#include "FileLogger.h"

#include "FileWriter.h"
#include "Logger.h"
#include <utility>
#include "Result.h"
#include "Typedef.h"
#include <format>

namespace jug
{

FileLogger::FileLogger(
    FileWriter&& _writer)
    : m_writer(std::move(_writer))
{
}

Result<FileLogger> FileLogger::Open(
    const FilePath& _path,
    const bool      _bAppend)
{
    Result<FileWriter> writer = FileWriter::Open(_path, _bAppend);
    JUG_RETURN_IF_ERROR(writer);
    return FileLogger { writer.Take() };
}

void FileLogger::Flush()
{
    m_writer.Flush();
}

void FileLogger::WriteImpl(
    const eLogLevel,
    const StringView _msg,
    const bool       _bEndLog)
{
    m_writer.Write(_msg);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

void FileLogger::VWriteImpl(
    const eLogLevel,
    const StringView       _msg,
    const std::format_args _args,
    const bool             _bEndLog)
{
    m_writer.VWrite(_msg, _args);

    if (_bEndLog)
    {
        m_writer.Write("\n");
    }
}

}   // namespace jug
