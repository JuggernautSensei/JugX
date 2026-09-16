#include "pch.h"
#include "Event.h"

namespace jug
{

void Event::SetHandled(
	const bool _bHandled)
{
    m_bHandled = _bHandled;
}

bool Event::IsHandled() const
{
    return m_bHandled;
}

}