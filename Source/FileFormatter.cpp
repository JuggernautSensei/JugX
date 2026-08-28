#include "FileFormatter.h"

namespace jug
{

FileIOResult<size_t> VFormat(
    FILE*                  _pFile,
    const std::string_view _format,
    const std::format_args _args)
{
    JUG_ASSERT(_pFile, "VFormat: _pFile must not be null");

    FileOutputIterator iter { _pFile };
    std::vformat_to(iter, _format, _args);
    if (iter.error != eFileIOError::None)
    {
        return Failed { iter.error };
    }
    return iter.written;
}

}   // namespace jug