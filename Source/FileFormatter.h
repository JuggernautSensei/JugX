#pragma once
#include <cerrno>
#include <cstdio>
#include <format>
#include <iostream>

#include "Config.h"
#include "FileIOError.h"

namespace jug
{

struct FileOutputIterator
{
    using iterator_category = std::output_iterator_tag;
    using value_type        = void;
    using difference_type   = std::ptrdiff_t;
    using pointer           = void;
    using reference         = void;

    FileOutputIterator& operator=(
        const char c)
    {
        if (error != eFileIOError::None)
        {
            return *this;
        }

        if (std::fputc(c, pFile) == EOF)
        {
            if (::ferror(pFile))
            {
                error = ToFileIOError(errno);
            }

            return *this;
        }

        ++written;
        return *this;
    }

    FileOutputIterator& operator*()
    {
        return *this;
    }

    FileOutputIterator& operator++()
    {
        return *this;
    }

    FileOutputIterator& operator++(int)
    {
        return *this;
    }

    FILE*        pFile   = nullptr;
    eFileIOError error   = eFileIOError::None;
    size_t       written = 0;
};

template<typename... TArgs>
FileIOResult<size_t> Format(
    FILE*                        _pFile,
    std::format_string<TArgs...> _format,
    TArgs&&... _args)
{
    JUG_ASSERT(_pFile, "Log: _pFile must not be null");

    FileOutputIterator iter { _pFile };
    std::format_to(iter, _format, std::forward<TArgs>(_args)...);
    if (iter.error != eFileIOError::None)
    {
        return Failed { iter.error };
    }
    return iter.written;
}

FileIOResult<size_t> VFormat(FILE* _pFile, std::string_view _format, std::format_args _args);

}   // namespace jug