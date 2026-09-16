#include "pch.h"
#include "RefCount.h"

namespace jug
{

void RefCounted::Retain()
{
    ++m_refCount;
}

int RefCounted::Release()
{
    JUG_ASSERT(m_refCount > 0, "Reference count underflow");
    --m_refCount;
    return m_refCount;
}

int RefCounted::GetRefCount() const
{
    return m_refCount;
}

}