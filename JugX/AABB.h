#pragma once
#include "Corner.h"
#include "Matrix.h"
#include "EnumArray.h"

namespace jug
{

struct AABB
{
    JUG_MATH_API constexpr AABB() = default;

    JUG_MATH_API constexpr AABB(
        const VECTOR3 _center,
        const VECTOR3 _extends)
        : center(_center)
        , extends(_extends)
    {
    }

    // ========================================================
    //  Factory
    // ========================================================

    [[nodiscard]] JUG_MATH_API static constexpr AABB MakeFromMinMax(
        const VECTOR3 _min,
        const VECTOR3 _max)
    {
        AABB aabb;
        aabb.center  = (_min + _max) * 0.5f;
        aabb.extends = (_max - _min) * 0.5f;
        return aabb;
    }

    // ========================================================
    //  Utils
    // ========================================================

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
        return extends.e[0] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetHeight() const
    {
        return extends.e[1] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetDepth() const
    {
        return extends.e[2] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners() const
    {
        const VECTOR3 min = GetMin();
        const VECTOR3 max = GetMax();

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners;
        corners[eCorner::LeftBottomNear]  = VECTOR3 { min.e[0], min.e[1], min.e[2] };
        corners[eCorner::LeftBottomFar]   = VECTOR3 { min.e[0], min.e[1], max.e[2] };
        corners[eCorner::LeftTopNear]     = VECTOR3 { min.e[0], max.e[1], min.e[2] };
        corners[eCorner::LeftTopFar]      = VECTOR3 { min.e[0], max.e[1], max.e[2] };
        corners[eCorner::RightBottomNear] = VECTOR3 { max.e[0], min.e[1], min.e[2] };
        corners[eCorner::RightBottomFar]  = VECTOR3 { max.e[0], min.e[1], max.e[2] };
        corners[eCorner::RightTopNear]    = VECTOR3 { max.e[0], max.e[1], min.e[2] };
        corners[eCorner::RightTopFar]     = VECTOR3 { max.e[0], max.e[1], max.e[2] };
        return corners;
    }

    // ========================================================
    //  Fields
    // =======================================================

    const static AABB kZero;
    const static AABB kUnit;

    VECTOR3 center;
    VECTOR3 extends;
};
static_assert(PodT<AABB>, "AABB must be POD type.");

// ========================================================
//  Constant
// ========================================================

inline constexpr AABB AABB::kZero = AABB { Zero<VECTOR3>(), Zero<VECTOR3>() };
inline constexpr AABB AABB::kUnit = AABB { Zero<VECTOR3>(), VECTOR3 { 0.5f } };

template<>
struct MathConstants<AABB>
{
    static constexpr AABB kZero = AABB::kZero;
    static constexpr AABB kOne  = AABB::kUnit;
};

// ========================================================
//  Operators
// ========================================================

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

[[nodiscard]] JUG_MATH_API constexpr AABB Transform(
    const AABB&   _aabb,
    const MATRIX& _mtx)
{
    const VECTOR3 newCenter = MulPoint(_aabb.center, _mtx);
    const VECTOR3 ex        = Abs(MulVector(VECTOR3 { _aabb.extends.e[0], 0.f, 0.f }, _mtx));
    const VECTOR3 ey        = Abs(MulVector(VECTOR3 { 0.f, _aabb.extends.e[1], 0.f }, _mtx));
    const VECTOR3 ez        = Abs(MulVector(VECTOR3 { 0.f, 0.f, _aabb.extends.e[2] }, _mtx));
    return AABB { newCenter, ex + ey + ez };
}

}   // namespace jug
