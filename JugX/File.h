#pragma once

namespace jug
{

// ===========================================
//  Low level file I/O
// ===========================================

enum class eSeekOrigin
{
    Set = SEEK_SET,
    Cur = SEEK_CUR,
    End = SEEK_END
};

int                   Seek(FILE* _pFile, int64_t _offset, eSeekOrigin _origin);
[[nodiscard]] int64_t Tell(FILE* _pFile);

void LockFile(FILE* _pFile);
void UnlockFile(FILE* _pFile);

int GetCharNoLock(FILE* _pFile);
int PutCharNoLock(FILE* _pFile, int _ch);

}   // namespace jug