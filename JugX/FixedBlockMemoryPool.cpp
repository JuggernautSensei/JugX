#include "pch.h"
#include "FixedBlockMemoryPool.h"

namespace jug
{

FixedBlockMemoryPool::FixedBlockMemoryPool(
    const size_t _blockSize,
    const size_t _numBlocksPerPool,
    const size_t _blockAlign)
    : m_alignedBlockSize(AlignUp(_blockSize, _blockAlign))
    , m_blockAlign(_blockAlign)
    , m_numBlocksPerPool(_numBlocksPerPool)
{
    JUG_ASSERT(std::has_single_bit(_blockAlign), "_blockAlign must be power of 2");
    JUG_ASSERT(_numBlocksPerPool > 0, "_numBlocksPerPool must be greater than 0");
    JUG_ASSERT(m_alignedBlockSize > 0, "_blockSize must be greater than 0");
}

FixedBlockMemoryPool::FixedBlockMemoryPool(
    FixedBlockMemoryPool&& _other) noexcept
    : m_pPoolList(_other.m_pPoolList)
    , m_pFreeListOrNull(_other.m_pFreeListOrNull)
    , m_pEnd(_other.m_pEnd)
    , m_pCur(_other.m_pCur)
    , m_alignedBlockSize(_other.m_alignedBlockSize)
    , m_blockAlign(_other.m_blockAlign)
    , m_numBlocksPerPool(_other.m_numBlocksPerPool)
{
    _other.m_pPoolList       = nullptr;
    _other.m_pFreeListOrNull = nullptr;
    _other.m_pEnd            = nullptr;
    _other.m_pCur            = nullptr;
}

FixedBlockMemoryPool& FixedBlockMemoryPool::operator=(
    FixedBlockMemoryPool&& _other) noexcept
{
    if (this != &_other)
    {
        Reset();
        m_pPoolList              = _other.m_pPoolList;
        m_pFreeListOrNull        = _other.m_pFreeListOrNull;
        m_pEnd                   = _other.m_pEnd;
        m_pCur                   = _other.m_pCur;
        m_alignedBlockSize       = _other.m_alignedBlockSize;
        m_blockAlign             = _other.m_blockAlign;
        m_numBlocksPerPool       = _other.m_numBlocksPerPool;
        _other.m_pPoolList       = nullptr;
        _other.m_pFreeListOrNull = nullptr;
        _other.m_pEnd            = nullptr;
        _other.m_pCur            = nullptr;
    }
    return *this;
}

FixedBlockMemoryPool::~FixedBlockMemoryPool()
{
    Reset();
}

void* FixedBlockMemoryPool::Alloc()
{
    // free list 재사용
    if (m_pFreeListOrNull)
    {
        Link* pBlock      = m_pFreeListOrNull;
        m_pFreeListOrNull = m_pFreeListOrNull->pLinkOrNull;
        return pBlock;
    }

    if (!m_pPoolList || m_pCur == m_pEnd)
    {
        // | Pool Header + Padding | Block 0 | Block 1 | ... | Block N |
        const size_t poolHeaderSize = AlignUp(sizeof(Link), m_blockAlign);
        const size_t poolSize       = poolHeaderSize + m_alignedBlockSize * m_numBlocksPerPool;
        JUG_ASSERT(IsAligned(poolSize, m_blockAlign), "poolSize must be aligned to m_blockAlign");

        void* pPool    = jug::Malloc(poolSize, m_blockAlign);
        Link* pHeader        = static_cast<Link*>(pPool);
        pHeader->pLinkOrNull = m_pPoolList;
        m_pPoolList          = pHeader;

        m_pCur = static_cast<std::byte*>(pPool) + poolHeaderSize;
        m_pEnd = static_cast<std::byte*>(pPool) + poolSize;
        JUG_ASSERT(IsAligned(m_pCur, m_blockAlign), "m_pCur must be aligned to m_blockAlign");
    }

    void* pMem = m_pCur;
    m_pCur += m_alignedBlockSize;
    return pMem;
}

void FixedBlockMemoryPool::Free(
    void* _pMem)
{
    if (_pMem)
    {
        Link* pBlock        = static_cast<Link*>(_pMem);
        pBlock->pLinkOrNull = m_pFreeListOrNull;
        m_pFreeListOrNull   = pBlock;
    }
}

void FixedBlockMemoryPool::Reset()
{
    Link* pPool = m_pPoolList;
    while (pPool)
    {
        Link* pNext = pPool->pLinkOrNull;
        jug::Free(pPool, m_blockAlign);
        pPool = pNext;
    }

    m_pPoolList       = nullptr;
    m_pFreeListOrNull = nullptr;
    m_pEnd            = nullptr;
    m_pCur            = nullptr;
}

}   // namespace jug