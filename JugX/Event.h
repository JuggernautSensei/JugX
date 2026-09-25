#pragma once
#include "TypeRefl.h"

namespace jug
{

class Event
{
    JUG_CLASS(Event, DEFAULT_COPY, DEFAULT_MOVE)

public:
    Event()          = default;
    virtual ~Event() = default;

    void               SetHandled(bool _bHandled);
    [[nodiscard]] bool IsHandled() const;

    [[nodiscard]] virtual String     ToString() const = 0;
    [[nodiscard]] virtual uint64_t   GetHash() const  = 0;
    [[nodiscard]] virtual StringView GetName() const  = 0;

private:
    bool m_bHandled = false;
};

template<typename T>
concept EventT = std::is_base_of_v<Event, T>;

}   // namespace jug

#define JUG_EVENT_BODY(_type)                                                  \
public:                                                                        \
    static constexpr uint64_t          kHash = ::jug::HashOf<_type>();         \
    static constexpr ::jug::StringView kName = ::jug::NameOf<_type>();         \
                                                                               \
    [[nodiscard]] uint64_t          GetHash() const override { return kHash; } \
    [[nodiscard]] ::jug::StringView GetName() const override { return kName; } \
    static_assert(true, "JUG_EVENT_BODY must be terminated with a semicolon.")
