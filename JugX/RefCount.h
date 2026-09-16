#pragma once

namespace jug
{

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
