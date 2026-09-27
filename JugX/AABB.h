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

// 아핀 행렬 전용. 중심은 w 로 나누지 않는다.
[[nodiscard]] JUG_MATH_API constexpr AABB Xform(
    const AABB&   _aabb,
    const MATRIX& _mtx)
{
    // c = (cx cy cz -), e 의 1 ~ 3 레인 = (ex ey ez)
    simd::M128 c {};
    simd::M128 e {};
    if (std::is_constant_evaluated())
    {
        c = _aabb.center.ToM128();
        e = simd::Shuffle<0, 0, 1, 2>(_aabb.extends.ToM128());
    }
    else
    {
        const float* pIn = reinterpret_cast<const float*>(&_aabb);
        c                = simd::Load(pIn);
        e                = simd::Load(pIn + 2);
    }
    e = simd::Abs(e);

    const simd::M128X4 m = _mtx.ToM128X4();

    simd::M128 center = simd::MultAdd(simd::Splat<0>(c), m.r[0], m.r[3]);
    center            = simd::MultAdd(simd::Splat<1>(c), m.r[1], center);
    center            = simd::MultAdd(simd::Splat<2>(c), m.r[2], center);

    simd::M128 ext = simd::Mult(simd::Splat<1>(e), simd::Abs(m.r[0]));
    ext            = simd::MultAdd(simd::Splat<2>(e), simd::Abs(m.r[1]), ext);
    ext            = simd::MultAdd(simd::Splat<3>(e), simd::Abs(m.r[2]), ext);

    if (std::is_constant_evaluated())
    {
        return simd::NoInlineConstruct<AABB>(VECTOR3::FromM128(center), VECTOR3::FromM128(ext));
    }
    else
    {
        AABB   ret;
        float* pOut = reinterpret_cast<float*>(&ret);
        simd::Store(pOut, simd::Blend<0b1000>(center, simd::Splat<0>(ext)));
        simd::StoreFloat2(pOut + 4, simd::Shuffle<1, 2, 2, 3>(ext));
        return ret;
    }
}

}   // namespace jug
