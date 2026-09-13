#pragma once
#include <new>

#include "Align.h"
#include "Alloc.h"

namespace jug
{

// =========================================================
//  Fixed Block Pool
//   고정 사이즈 블록 메모리 풀
//   최소한의 구현이기에 대부분의 메모리 관련 문제에 대응하지 못함
//   1. 풀에 속하지 않은 메모리 반납
//   2. 더블 프리
//   3. 멀티스레드 환경에서의 동기화 문제
//   -> 전부 사용자가 책임져야함
// =========================================================

class FixedBlockMemoryPool
{
    JUG_CLASS(FixedBlockMemoryPool, NO_COPY)

    struct LINK
    {
        LINK* pLink;
    };

public:
    FixedBlockMemoryPool(size_t _blockSize, size_t _numBlocksPerPool, size_t _blockAlign = alignof(std::max_align_t));
    FixedBlockMemoryPool(FixedBlockMemoryPool&& _other) noexcept;
    FixedBlockMemoryPool& operator=(FixedBlockMemoryPool&& _other) noexcept;
    ~FixedBlockMemoryPool();

    void* Alloc();
    void  Free(void* _pMem);
    void  Reset();

private:
    LINK*      m_pPoolList        = nullptr;
    LINK*      m_pFreeList        = nullptr;
    std::byte* m_pEnd             = nullptr;
    std::byte* m_pCur             = nullptr;
    size_t     m_alignedBlockSize = 0;
    size_t     m_blockAlign       = 0;
    size_t     m_numBlocksPerPool = 0;
};

}   // namespace jug
