#pragma once
#include <chrono>

namespace jug
{

enum class eDayOfTheWeek
{
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

struct TimeStamp
{
    [[nodiscard]] static TimeStamp Now();

    int           sec           = 0;
    int           min           = 0;
    int           hour          = 0;
    int           year          = 0;
    int           month         = 0;
    int           dayOfTheMonth = 0;
    eDayOfTheWeek dayOfTheWeek  = eDayOfTheWeek::Sunday;
    int           dayOfTheYear  = 0;
};

}   // namespace jug