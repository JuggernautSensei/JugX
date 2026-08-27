#include "FileIOError.h"

namespace jug
{

eFileIOError ToFileIOError(
    const int _errorCode)
{
    switch (_errorCode)
    {
        case 0:
            return eFileIOError::None;
        case EACCES:
            return eFileIOError::AccessDenied;
        case ENOENT:
            return eFileIOError::NotFound;
        case ENOSPC:
            return eFileIOError::DiskFull;
        case EIO:
            return eFileIOError::IOError;
        case EFBIG:
            return eFileIOError::FileTooLarge;
        case EROFS:
            return eFileIOError::ReadOnlyFileSystem;
        case EMFILE:
            return eFileIOError::TooManyOpenFiles;
        default:
            return eFileIOError::UnknownError;
    }
}

}   // namespace jug