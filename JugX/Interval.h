#pragma once
#include "Math.h"

namespace jug
{

struct INTERVAL
{
    JUG_MATH_API INTERVAL() = default;

    JUG_MATH_API constexpr INTERVAL(
        const float _min,
        const float _max)
        : min(_min)
        , max(_max)
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetLength() const
    {
        return max - min;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] JUG_MATH_API static constexpr INTERVAL Zero()
    {
        return INTERVAL { 0.f, 0.f };
    }

    [[nodiscard]] JUG_MATH_API static constexpr INTERVAL Universe()
    {
        return INTERVAL { Min<float>(), Max<float>() };
    }

    float min;
    float max;
};

static_assert(PodT<INTERVAL>, "INTERVAL must be POD type.");

// =======================================================
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const INTERVAL& _interval,
    const float     _value)
{
    return (_value >= _interval.min) && (_value <= _interval.max);
}

[[nodiscard]] JUG_MATH_API constexpr bool Surround(
    const INTERVAL& _interval,
    const float     _value)
{
    return (_value >= _interval.min) && (_value < _interval.max);
}

[[nodiscard]] JUG_MATH_API constexpr float Clamp(
    const INTERVAL& _interval,
    const float     _value)
{
    return Clamp(_value, _interval.min, _interval.max);
}

}   // namespace jug