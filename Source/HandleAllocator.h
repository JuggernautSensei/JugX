#pragma once
#include <vector>

#include "Handle.h"

#define DT handle_allocator_detail

namespace jug
{

// ===============================================================
//  HandleAllocator
//   핸들을 할당해줄뿐 아니라, 유효성 검사도 가능.
//   최대한 dense한 index를 유지하도록 설계됨.
//   할당, 해제 모두 O(1).
// ===============================================================

namespace DT
{
    constexpr size_t kNullIndex  = SIZE_MAX;
    constexpr size_t kGrowFactor = 2;
    constexpr size_t kMinSize    = 4;
}   // namespace DT

template<
    typename TTag,
    template<typename...> class TVector = std::vector>
class HandleAllocator
{
    using Handle = Handle<TTag>;

    struct Item
    {
        size_t  nextFree = DT::kNullIndex;   // free list의 다음 인덱스. intrusive linked list로 구현. DT::kNullIndex이면 free list의 끝.
        uint8_t token    = 0;
        bool    bAlloced = false;
    };

public:
    [[nodiscard]] Handle Alloc()
    {
        size_t index = 0;
        size_t token = 0;

        if (m_freeHead == DT::kNullIndex)   // free list가 없음
        {
            index = m_nextIndex;
            ++m_nextIndex;

            if (m_sparse.size() <= index)
            {
                const size_t size = std::max<size_t>(DT::kMinSize, index * DT::kGrowFactor);
                m_sparse.resize(size);
            }

            // token = 0; 새로 할당되는 인덱스는 token이 0.
        }
        else
        {
            index = m_freeHead;
            token = m_sparse[index].token;   // Free()에서 이미 다음 토큰으로 올려둠

            m_freeHead = m_sparse[index].nextFree;
            if (m_freeHead == DT::kNullIndex)
            {
                m_freeTail = DT::kNullIndex;
            }
        }

        JUG_ASSERT(index <= handle_detail::kMaxIndex, "Index out of range. Must be <= kMaxIndex.\n");
        JUG_ASSERT(token <= handle_detail::kMaxToken, "Token out of range. Must be <= kMaxToken.\n");
        m_sparse[index].bAlloced = true;
        ++m_numAlloced;
        return Handle { static_cast<uint32_t>(index), static_cast<uint8_t>(token) };
    }

    void Free(
        const Handle _handle)
    {
        JUG_ASSERT(IsValid(_handle), "Invalid handle passed to Free().\n");

        const uint32_t index = _handle.GetIndex();

        Item& item    = m_sparse[index];
        item.bAlloced = false;
        item.nextFree = DT::kNullIndex;
        ++item.token;   // 토큰을 1 증가시켜 무효화, 8비트 범위 [0, 256)에서 순환

        if (m_freeTail == DT::kNullIndex)
        {
            m_freeHead = index;
            m_freeTail = index;
        }
        else
        {
            // free list의 마지막 원소 뒤에 붙임
            m_sparse[m_freeTail].nextFree = index;
            m_freeTail                    = index;
        }

        --m_numAlloced;
    }

    [[nodiscard]] bool IsValid(
        const Handle _handle) const
    {
        // kNullHandle 인 경우도 여기서 false 분기로 처리됨.
        const int index = _handle.GetIndex();
        if (index >= m_sparse.size() || !m_sparse[index].bAlloced)
        {
            return false;
        }

        return m_sparse[index].token == _handle.GetToken();
    }

    [[nodiscard]] int GetNumAlloced() const
    {
        return m_numAlloced;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return m_numAlloced == 0;
    }

    void Clear()
    {
        m_sparse.clear();
        m_freeHead   = DT::kNullIndex;
        m_freeTail   = DT::kNullIndex;
        m_numAlloced = 0;
        m_nextIndex  = 0;
    }

private:
    constexpr static int kGrowFactor = 2;

    TVector<Item> m_sparse     = {};               // For Random Access. { next free index, token, alloced }
    size_t        m_freeHead   = DT::kNullIndex;   // free list의 첫 원소. 다음에 Alloc될 슬롯
    size_t        m_freeTail   = DT::kNullIndex;   // free list의 마지막 원소. 다음 Free가 여기 뒤에 붙음
    size_t        m_numAlloced = 0;
    size_t        m_nextIndex  = 0;
};

}   // namespace jug

#undef DT