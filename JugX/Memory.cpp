#include "pch.h"
#include "Memory.h"

#include <cstddef>

namespace jug
{

Memory AllocMemory(const size_t _size)
{
    std::byte* pMem = new std::byte[_size];
    return Memory { pMem, _size };
}

}   // namespace jug