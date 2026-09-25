#pragma once
#include "Corner.h"
#include "Matrix.h"
#include "EnumArray.h"

namespace jug
{

struct AABB
{
    JUG_MATH_API AABB() = default;

    JUG_MATH_API constexpr AABB(
        const VECTOR3 _center,
        const VECTOR3 _extends)
        : center(_center)
        , extends(_extends)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr AABB MakeFromMinMax(
        const VECTOR3 _min,
        const VECTOR3 _max)
    {
        AABB aabb;
        aabb.center  = (_min + _max) * 0.5f;
        aabb.extends = (_max - _min) * 0.5f;
        return aabb;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetMin() const
    {
        return center - extends;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetMax() const
    {
        return center + extends;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetWidth() const
    {
        return extends[0] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetHeight() const
    {
        return extends[1] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetDepth() const
    {
        return extends[2] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners() const
    {
        const VECTOR3 min = GetMin();
        const VECTOR3 max = GetMax();

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners;
        corners[eCorner::LeftBottomNear]  = VECTOR3 { min[0], min[1], min[2] };
        corners[eCorner::LeftBottomFar]   = VECTOR3 { min[0], min[1], max[2] };
        corners[eCorner::LeftTopNear]     = VECTOR3 { min[0], max[1], min[2] };
        corners[eCorner::LeftTopFar]      = VECTOR3 { min[0], max[1], max[2] };
        corners[eCorner::RightBottomNear] = VECTOR3 { max[0], min[1], min[2] };
        corners[eCorner::RightBottomFar]  = VECTOR3 { max[0], min[1], max[2] };
        corners[eCorner::RightTopNear]    = VECTOR3 { max[0], max[1], min[2] };
        corners[eCorner::RightTopFar]     = VECTOR3 { max[0], max[1], max[2] };
        return corners;
    }

    const static AABB kZero;
    const static AABB kUnit;

    VECTOR3 center;
    VECTOR3 extends;
};
static_assert(PodT<AABB>, "AABB must be POD type.");

// ===========================================
//  Constants
// ===========================================

inline constexpr AABB AABB::kZero = AABB { Zero<VECTOR3>(), Zero<VECTOR3>() };
inline constexpr AABB AABB::kUnit = AABB { Zero<VECTOR3>(), VECTOR3 { 0.5f } };

template<>
struct MathConstants<AABB>
{
    static constexpr AABB kZero = AABB::kZero;
    static constexpr AABB kOne  = AABB::kUnit;
};

// ===========================================
//  Method
// ===========================================

[[nodiscard]] JUG_MATH_API constexpr AABB Merge(
    const AABB& _x,
    const AABB& _y)
{
    const VECTOR3 min = Min(_x.GetMin(), _y.GetMin());
    const VECTOR3 max = Max(_x.GetMax(), _y.GetMax());
    return AABB::MakeFromMinMax(min, max);
}

[[nodiscard]] JUG_MATH_API constexpr AABB Merge(
    const AABB&   _x,
    const VECTOR3 _point)
{
    const VECTOR3 min = Min(_x.GetMin(), _point);
    const VECTOR3 max = Max(_x.GetMax(), _point);
    return AABB::MakeFromMinMax(min, max);
}

[[nodiscard]] JUG_MATH_API constexpr AABB Xform(
    const AABB&   _aabb,
    const MATRIX& _mtx)
{
    const VECTOR3 e       = Abs(_aabb.extends);
    const VECTOR4 extends = Abs(_mtx[0]) * e[0] + Abs(_mtx[1]) * e[1] + Abs(_mtx[2]) * e[2];
    return AABB { XformPoint(_aabb.center, _mtx), extends.ToVector3() };
}

}   // namespace jug
