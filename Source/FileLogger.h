#pragma once
#include <filesystem>

#include "FileIO.h"
#include "Logger.h"

namespace jug
{

class FileLogger : public Logger
{
public:
    [[nodiscard]] static FileIOResult<FileLogger> Open(const std::filesystem::path& _path, bool _bAppend = false);

    void Flush() override;

private:
    explicit FileLogger(FileWriter&& _writer);
    void LogImpl(eLogLevel _level, std::string_view _message, bool _bNewLine) override;
    void FormatImpl(eLogLevel _level, std::string_view _message, std::format_args _args, bool _bNewLine) override;

    FileWriter m_writer;
};

}   // namespace jug
