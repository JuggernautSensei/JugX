#pragma once
#include "Align.h"
#include "Alloc.h"

namespace jug
{

// =========================================================
//  MemoryArena
//   메모리 해제없이 빠른 메모리 할당을 위해 사용되는 메모리 풀
//   영속적으로 사용하는 개념 아님
//   제한된 스코프에서 힙할당을 최소화하기 위해 사용
// =========================================================

class MemoryArena
{
    JUG_CLASS(MemoryArena, NO_COPY)

    struct LINK
    {
        LINK* pLink;
    };

public:
    explicit MemoryArena(size_t _poolSize);
    MemoryArena(MemoryArena&& _other) noexcept;
    MemoryArena& operator=(MemoryArena&& _other) noexcept;
    ~MemoryArena();

    void* Alloc(size_t _size);
    void* Alloc(size_t _size, size_t _alignment);
    void  Reset();

private:
    LINK*      m_pPoolList = nullptr;   // 풀 리스트
    std::byte* m_pCur      = nullptr;
    std::byte* m_pEnd      = nullptr;
    size_t     m_poolSize  = 0;
};

}   // namespace jug