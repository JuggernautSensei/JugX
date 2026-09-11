#include "Timer.h"

#include "Assertion.h"
#include "Config.h"

namespace jug
{

float TimeCastF(
    const uint64_t  _count,
    const eTimeUnit _unit)
{
    switch (_unit)
    {
        case eTimeUnit::Nano:
            return static_cast<float>(_count);
        case eTimeUnit::Micro:
            return static_cast<float>(_count) * 1e-3f;
        case eTimeUnit::Milli:
            return static_cast<float>(_count) * 1e-6f;
        case eTimeUnit::Sec:
            return static_cast<float>(_count) * 1e-9f;
        default:
            JUG_ASSERT(false, "Invalid time unit.\n");
            return 0.f;
    }
}

double TimeCast(
    const uint64_t  _count,
    const eTimeUnit _unit)
{
    switch (_unit)
    {
        case eTimeUnit::Nano:
            return static_cast<double>(_count);
        case eTimeUnit::Micro:
            return static_cast<double>(_count) * 1e-3;
        case eTimeUnit::Milli:
            return static_cast<double>(_count) * 1e-6;
        case eTimeUnit::Sec:
            return static_cast<double>(_count) * 1e-9;
        default:
            JUG_ASSERT(false, "Invalid time unit.\n");
            return 0.;
    }
}

void Timer::Start()
{
    JUG_ASSERT(!m_bRunning, "Timer is already running.\n");
    m_startCount = SteadyClock::now();
    m_bRunning   = true;
}

void Timer::Lap()
{
    JUG_ASSERT(m_bRunning, "Timer is not running.\n");

    const TimePoint now          = SteadyClock::now();
    const uint64_t  elapsedCount = std::chrono::duration_cast<Ns>(now - m_startCount).count();
    m_elapsedCount               = elapsedCount;
    m_totalCount += elapsedCount;
    m_startCount = now;
}

void Timer::Stop()
{
    Lap();
    m_bRunning = false;
}

void Timer::Reset()
{
    m_elapsedCount = 0;
    m_totalCount   = 0;
    m_bRunning     = false;
}

uint64_t Timer::GetElapsedCount() const
{
    return m_elapsedCount;
}

uint64_t Timer::GetTotalCount() const
{
    return m_totalCount;
}

bool Timer::IsRunning() const
{
    return m_bRunning;
}

}   // namespace jug