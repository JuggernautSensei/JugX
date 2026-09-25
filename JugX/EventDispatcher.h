#pragma once
#include "Assert.h"
#include "Event.h"

namespace jug
{

template<typename T, typename Fn>
concept EventDispatchFnT = EventT<T> && (std::is_invocable_v<Fn, T&> || std::is_invocable_v<Fn, const T&>);

template<typename T, typename Caller, typename Method>
concept EventDispatchMethodT = EventT<T> && (std::is_invocable_v<Method, Caller*, T&> || std::is_invocable_v<Method, Caller*, const T&>);

class EventDispatcher
{
public:
    explicit EventDispatcher(Event& _event);

    template<EventT T, typename Fn>
        requires EventDispatchFnT<T, Fn>
    void Dispatch(
        Fn&& _fn)
    {
        if (m_pEvent->IsHandled())
        {
            return;
        }

        if (m_hash == T::kHash)
        {
            T& event = *static_cast<T*>(m_pEvent);
            _fn(event);
        }
    }

    template<EventT T, typename Caller, typename Method>
        requires EventDispatchMethodT<T, Caller, Method>
    void Dispatch(
        Caller*  _pCaller,
        Method&& _method)
    {
        JUG_ASSERT(_pCaller, "EventDispatcher::Dispatch: _pCaller is nullptr.\n");

        if (m_pEvent->IsHandled())
        {
            return;
        }

        if (m_hash == T::kHash)
        {
            T& event = *static_cast<T*>(m_pEvent);
            (_pCaller->*_method)(event);
        }
    }

private:
    Event*   m_pEvent = nullptr;
    uint64_t m_hash   = 0;
};

}   // namespace jug
