#pragma once
#include "Callable.h"

namespace jug
{

enum class eTimeUnit
{
    Nano,
    Micro,
    Milli,
    Sec
};

[[nodiscard]] float      TimeCastF(uint64_t _count, eTimeUnit _unit);
[[nodiscard]] double     TimeCast(uint64_t _count, eTimeUnit _unit);
[[nodiscard]] eTimeUnit  ToReadableTimeUnit(uint64_t _count);
[[nodiscard]] StringView ToTimeUnitString(eTimeUnit _unit);   // ns, us, ms, s

struct ReadableTime
{
    eTimeUnit  unit     = eTimeUnit::Nano;
    double     time     = 0.0;
    StringView unitName = {};
};

struct ReadableTimeF
{
    eTimeUnit  unit     = eTimeUnit::Nano;
    float      time     = 0.f;
    StringView unitName = {};
};

[[nodiscard]] ReadableTime  ToReadableTime(uint64_t _count);
[[nodiscard]] ReadableTimeF ToReadableTimeF(uint64_t _count);

class Timer
{
public:
    void Start();
    void Lap();
    void Stop();
    void Reset();

    [[nodiscard]] uint64_t GetElapsedCount() const;
    [[nodiscard]] uint64_t GetTotalCount() const;
    [[nodiscard]] bool     IsRunning() const;

private:
    TimePoint<SteadyClock> m_startCount   = {};
    uint64_t               m_elapsedCount = 0;
    uint64_t               m_totalCount   = 0;
    bool                   m_bRunning     = false;
};

}   // namespace jug
