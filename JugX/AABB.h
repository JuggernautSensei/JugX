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
        const VEC3 _center,
        const VEC3 _extends)
        : center(_center)
        , extends(_extends)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr AABB MakeFromMinMax(
        const VEC3 _min,
        const VEC3 _max)
    {
        AABB aabb;
        aabb.center  = (_min + _max) * 0.5f;
        aabb.extends = (_max - _min) * 0.5f;
        return aabb;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetMin() const
    {
        return center - extends;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetMax() const
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

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcCorners() const
    {
        const VEC3 min = GetMin();
        const VEC3 max = GetMax();

        DIRECT_ENUM_ARRAY<eCorner, VEC3> corners = {};
        corners[eCorner::LeftBottomNear]         = VEC3 { min[0], min[1], min[2] };
        corners[eCorner::LeftBottomFar]   = VEC3 { min[0], min[1], max[2] };
        corners[eCorner::LeftTopNear]     = VEC3 { min[0], max[1], min[2] };
        corners[eCorner::LeftTopFar]      = VEC3 { min[0], max[1], max[2] };
        corners[eCorner::RightBottomNear] = VEC3 { max[0], min[1], min[2] };
        corners[eCorner::RightBottomFar]  = VEC3 { max[0], min[1], max[2] };
        corners[eCorner::RightTopNear]    = VEC3 { max[0], max[1], min[2] };
        corners[eCorner::RightTopFar]     = VEC3 { max[0], max[1], max[2] };
        return corners;
    }

    [[nodiscard]] JUG_MATH_API static constexpr AABB Zero()
    {
        return AABB { VEC3::Zero(), VEC3::Zero() };
    }

    [[nodiscard]] JUG_MATH_API static constexpr AABB Unit()
    {
        return AABB { VEC3::Zero(), VEC3 { 0.5f } };
    }

    VEC3 center;
    VEC3 extends;
};
static_assert(PodT<AABB>, "AABB must be POD type.");

// ===========================================
//  Method
// ===========================================

[[nodiscard]] JUG_MATH_API constexpr AABB Merge(
    const AABB& _x,
    const AABB& _y)
{
    const VEC3 min = Min(_x.GetMin(), _y.GetMin());
    const VEC3 max = Max(_x.GetMax(), _y.GetMax());
    return AABB::MakeFromMinMax(min, max);
}

[[nodiscard]] JUG_MATH_API constexpr AABB Merge(
    const AABB& _x,
    const VEC3  _point)
{
    const VEC3 min = Min(_x.GetMin(), _point);
    const VEC3 max = Max(_x.GetMax(), _point);
    return AABB::MakeFromMinMax(min, max);
}

// 아핀 행렬 전용. 중심은 w 로 나누지 않는다.
[[nodiscard]] JUG_MATH_API constexpr AABB Xform(
    const AABB& _aabb,
    const MAT4& _mtx)
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
        return simd::NoInlineConstruct<AABB>(VEC3::FromM128(center), VEC3::FromM128(ext));
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
