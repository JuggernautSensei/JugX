
#pragma once
#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "Config.h"
#include "Align.h"
#include "FixedBlockMemoryPool.h"

namespace jug
{

// =========================================================
//  Object Pool
//   고정 사이즈 객체 메모리 풀
//   최소한의 구현이기에 대부분의 메모리 관련 문제에 대응하지 못함
//   1. 풀에 속하지 않은 메모리 반납
//   2. 더블 프리
//   3. 멀티스레드 환경에서의 동기화 문제
//   -> 전부 사용자가 책임져야함
//
//   안전하게 쓰는 방법 -> Scoped<T, CustomDeleter> or std::unique_ptr<T, CustomDeleter> 사용
// =========================================================

template<typename T>
class ObjectPool
{
public:
    explicit ObjectPool(
        const size_t _numObjectPerPool = 1024)
        : m_pool(sizeof(T), _numObjectPerPool, alignof(T))
    {
    }

    template<typename... Args>
    T* New(
        Args&&... _args)
    {
        T* pObj = static_cast<T*>(m_pool.Alloc());
        return std::construct_at(pObj, std::forward<Args>(_args)...);
    }

    void Delete(
        T* const _pObj)
    {
        if (_pObj)
        {
            std::destroy_at(_pObj);
            m_pool.Free(_pObj);
        }
    }

private:
    FixedBlockMemoryPool m_pool;
};

}   // namespace jug