#pragma once

namespace jug
{

enum class eTimeUnit
{
    Nano,
    Micro,
    Milli,
    Sec
};

[[nodiscard]] float  TimeCastF(uint64_t _count, eTimeUnit _unit);
[[nodiscard]] double TimeCast(uint64_t _count, eTimeUnit _unit);

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