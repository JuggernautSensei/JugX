#pragma once
#include <cerrno>
#include <cstdio>
#include <format>

#include "Config.h"
#include "FileIOError.h"

namespace jug
{

// ============================================================
//  File Write with Format
// ============================================================

struct FileFormatState
{
    FILE*        pFile   = nullptr;
    eFileIOError error   = eFileIOError::None;
    size_t       written = 0;
};

struct FileFormatInserter
{
    using iterator_category = std::output_iterator_tag;
    using value_type        = void;
    using difference_type   = std::ptrdiff_t;
    using pointer           = void;
    using reference         = void;

    FileFormatInserter& operator=(
        const char c)
    {
        if (pState->error != eFileIOError::None)
        {
            return *this;
        }

        if (std::fputc(c, pState->pFile) == EOF)
        {
            if (::ferror(pState->pFile))
            {
                pState->error = ToFileIOError(errno);
            }

            return *this;
        }

        ++pState->written;
        return *this;
    }

    FileFormatInserter& operator*()
    {
        return *this;
    }

    FileFormatInserter& operator++()
    {
        return *this;
    }

    FileFormatInserter& operator++(int)
    {
        return *this;
    }

    FileFormatState* pState = nullptr;
};

template<typename... TArgs>
FileIOResult<size_t> Format(
    FILE*                        _pFile,
    std::format_string<TArgs...> _format,
    TArgs&&... _args)
{
    JUG_ASSERT(_pFile, "Format: _pFile must not be null");

    FileFormatState state = { _pFile, eFileIOError::None, 0 };
    std::format_to(FileFormatInserter { &state }, _format, std::forward<TArgs>(_args)...);
    if (state.error != eFileIOError::None)
    {
        return Failed { state.error };
    }
    return state.written;
}

}   // namespace jug