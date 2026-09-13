#pragma once
#include <filesystem>

#include "FileReaderWriter.h"
#include "Logger.h"
#include "FileWriter.h"

namespace jug
{

class FileLogger : public Logger
{
public:
    [[nodiscard]] static Result<FileLogger> Open(const FilePath& _path, bool _bAppend = false);

    void Flush() override;

private:
    explicit FileLogger(FileWriter&& _writer);
    void WriteImpl(eLogLevel _level, StringView _msg, bool _bEndLog) override;
    void VWriteImpl(eLogLevel _level, StringView _msg, std::format_args _args, bool _bEndLog) override;

    FileWriter m_writer;
};

}   // namespace jug
