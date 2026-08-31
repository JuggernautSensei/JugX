#include "FileError.h"

namespace jug
{

eFileError ToFileError(
    const int _errorCode)
{
    switch (_errorCode)
    {
        case 0:
            return eFileError::None;
        case EACCES:
            return eFileError::AccessDenied;
        case ENOENT:
            return eFileError::NotFound;
        case ENOSPC:
            return eFileError::DiskFull;
        case EIO:
            return eFileError::IOError;
        case EFBIG:
            return eFileError::FileTooLarge;
        case EROFS:
            return eFileError::ReadOnlyFileSystem;
        case EMFILE:
            return eFileError::TooManyOpenFiles;
        default:
            return eFileError::UnknownError;
    }
}

}   // namespace jug