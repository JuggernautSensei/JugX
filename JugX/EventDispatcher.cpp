#include "pch.h"
#include "EventDispatcher.h"

#include "Event.h"

namespace jug
{

EventDispatcher::EventDispatcher(
    Event& _event)
    : m_pEvent(&_event)
    , m_hash(_event.GetHash())
{
}

}   // namespace jug