#pragma once
#include "Scoped.h"
#include "Config.h"
#include "Scoped.h"   // NOLINT

namespace jug
{

// ==========================================================
//  RefCounted
//   reference counting base class
// ==========================================================

class RefCounted
{
public:
    RefCounted()                             = default;
    virtual ~RefCounted()                    = default;
    RefCounted(const RefCounted&)            = default;
    RefCounted& operator=(const RefCounted&) = default;
    RefCounted(RefCounted&&)                 = default;
    RefCounted& operator=(RefCounted&&)      = default;

    void Retain()
    {
        ++m_refCount;
    }

    int Release()
    {
        JUG_ASSERT(m_refCount > 0, "Reference count underflow");
        --m_refCount;
        return m_refCount;
    }

    [[nodiscard]] int GetRefCount() const
    {
        return m_refCount;
    }

private:
    int m_refCount = 0;
};

}   // namespace jug
