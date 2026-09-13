#include "MemoryArena.h"

namespace jug
{

MemoryArena::MemoryArena(const size_t _poolSize)
    : m_poolSize(AlignUp(_poolSize, alignof(std::max_align_t)))
{
    JUG_ASSERT(m_poolSize > 0, "_poolSize must be greater than 0");
}

MemoryArena::~MemoryArena()
{
    Reset();
}

MemoryArena::MemoryArena(MemoryArena&& _other) noexcept
    : m_pPoolList(_other.m_pPoolList)
    , m_pCur(_other.m_pCur)
    , m_pEnd(_other.m_pEnd)
    , m_poolSize(_other.m_poolSize)
{
    _other.m_pPoolList = nullptr;
    _other.m_pCur      = nullptr;
    _other.m_pEnd      = nullptr;
}

MemoryArena& MemoryArena::operator=(MemoryArena&& _other) noexcept
{
    if (this != &_other)
    {
        Reset();
        m_pPoolList        = _other.m_pPoolList;
        m_pCur             = _other.m_pCur;
        m_pEnd             = _other.m_pEnd;
        m_poolSize         = _other.m_poolSize;
        _other.m_pPoolList = nullptr;
        _other.m_pCur      = nullptr;
        _other.m_pEnd      = nullptr;
    }
    return *this;
}

void* MemoryArena::Alloc(const size_t _size)
{
    return Alloc(_size, alignof(std::max_align_t));
}

void* MemoryArena::Alloc(const size_t _size, const size_t _alignment)
{
    JUG_ASSERT(std::has_single_bit(_alignment), "_alignment must be power of 2");
    JUG_ASSERT(sizeof(LINK) + (_alignment - 1) + _size <= m_poolSize, "MemoryArena pool size is too small for the requested allocation size and alignment.\n");

    std::byte* pAlignedCur = m_pCur ? AlignUp(m_pCur, _alignment) : nullptr;
    if (!m_pPoolList || pAlignedCur + _size > m_pEnd)
    {
        LINK* pHeader  = static_cast<LINK*>(jug::Alloc(m_poolSize));
        pHeader->pLink = m_pPoolList;
        m_pPoolList    = pHeader;
        m_pCur         = reinterpret_cast<std::byte*>(pHeader) + sizeof(LINK);
        m_pEnd         = reinterpret_cast<std::byte*>(pHeader) + m_poolSize;
        pAlignedCur    = AlignUp(m_pCur, _alignment);
    }

    m_pCur = pAlignedCur + _size;
    return pAlignedCur;
}

void MemoryArena::Reset()
{
    LINK* pPool = m_pPoolList;
    while (pPool)
    {
        LINK* pNext = pPool->pLink;
        jug::Free(pPool);
        pPool = pNext;
    }

    m_pPoolList = nullptr;
    m_pCur      = nullptr;
    m_pEnd      = nullptr;
}

}   // namespace jug