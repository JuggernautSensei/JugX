#include "pch.h"
#include "File.h"

#ifndef JUG_OS_WINDOWS
#    include <sys/types.h>
#endif

namespace jug
{

int Seek(
    FILE*             _pFile,
    const int64_t     _offset,
    const eSeekOrigin _origin)
{
#if JUG_OS_WINDOWS
    return ::_fseeki64(_pFile, _offset, static_cast<int>(_origin));
#else
    static_assert(sizeof(off_t) >= sizeof(int64_t), "off_t must be 64-bit. Define _FILE_OFFSET_BITS=64.");
    return ::fseeko(_pFile, static_cast<off_t>(_offset), static_cast<int>(_origin));
#endif
}

int64_t Tell(
    FILE* _pFile)
{
#if JUG_OS_WINDOWS
    return ::_ftelli64(_pFile);
#else
    static_assert(sizeof(off_t) >= sizeof(int64_t), "off_t must be 64-bit. Define _FILE_OFFSET_BITS=64.");
    return static_cast<int64_t>(::ftello(_pFile));
#endif
}

void LockFile(
    FILE* _pFile)
{
#if JUG_OS_WINDOWS
    ::_lock_file(_pFile);
#else
    ::flockfile(_pFile);
#endif
}

void UnlockFile(
    FILE* _pFile)
{
#if JUG_OS_WINDOWS
    ::_unlock_file(_pFile);
#else
    ::funlockfile(_pFile);
#endif
}

int GetCharNoLock(
    FILE* _pFile)
{
#if JUG_OS_WINDOWS
    return ::_fgetc_nolock(_pFile);
#else
    return ::getc_unlocked(_pFile);
#endif
}

int PutCharNoLock(
    FILE* _pFile,
    int   _ch)
{
#if JUG_OS_WINDOWS
    return ::_fputc_nolock(_ch, _pFile);
#else
    return ::putc_unlocked(_ch, _pFile);
#endif
}

}   // namespace jug