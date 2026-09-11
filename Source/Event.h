#pragma once
#include "Typedef.h"

namespace jug
{

class Event
{
public:
    virtual ~Event() = default;

    Event(const Event&)            = default;
    Event& operator=(const Event&) = default;
    Event(Event&&)                 = default;
    Event& operator=(Event&&)      = default;

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

#define EVENT_BODY(_type)                                                                   \
public:                                                                                     \
    [[nodiscard]] uint64_t         GetHash() const override { return HashOfType<_type>(); } \
    [[nodiscard]] StringView GetName() const override { return #_type; }