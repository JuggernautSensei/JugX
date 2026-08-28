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
    void LogImpl(eLogLevel _level, std::string_view _msg, bool _bEndLog) override;
    void VFormatImpl(eLogLevel _level, std::string_view _msg, std::format_args _args, bool _bEndLog) override;

    FileWriter m_writer;
};

}   // namespace jug
