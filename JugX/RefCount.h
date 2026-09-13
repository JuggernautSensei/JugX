#pragma once
#include "Assertion.h"
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
    JUG_CLASS(RefCounted, DEFAULT_COPY, DEFAULT_MOVE)

public:
    RefCounted()          = default;
    virtual ~RefCounted() = default;

    void              Retain();
    int               Release();
    [[nodiscard]] int GetRefCount() const;
    
private:
    int m_refCount = 0;
};

}   // namespace jug
