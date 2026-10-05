#pragma once
#include "Matrix.h"

namespace jug
{

struct PLANE
{
    JUG_MATH_API PLANE() = default;

    JUG_MATH_API constexpr PLANE(
        const VEC3  _normal,
        const float _d)
        : normal(_normal)
        , d(_d)
    {
    }

    JUG_MATH_API constexpr PLANE(
        const float _a,
        const float _b,
        const float _c,
        const float _d)
        : normal(_a, _b, _c)
        , d(_d)
    {
    }

    explicit JUG_MATH_API constexpr PLANE(
        const VEC4 _v)
        : normal(_v[0], _v[1], _v[2])
        , d(_v[3])
    {
    }

    JUG_MATH_API constexpr PLANE(
        const VEC3 _point,
        const VEC3 _normal)
        : normal(_normal)
        , d(-Dot(_normal, _point))
    {
    }

    JUG_MATH_API constexpr PLANE(
        const VEC3 _p0,
        const VEC3 _p1,
        const VEC3 _p2)
        : normal(Cross(_p1 - _p0, _p2 - _p0))
        , d(-Dot(normal, _p0))
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr PLANE Zero()
    {
        return PLANE { VEC3::Zero(), 0.f };
    }

    VEC3  normal;
    float d;
};

static_assert(PodT<PLANE>, "PLANE must be POD type.");

// =======================================================
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr PLANE Normalize(
    const PLANE _plane)
{
    const float lenSq = LengthSq(_plane.normal);
    if (IsZeroApprox(lenSq))   // 영벡터에 대한 정규화는 영벡터로 정의
    {
        return Zero<PLANE>();
    }

    const float rsqrt = RSqrt(lenSq);
    return PLANE { _plane.normal * rsqrt, _plane.d * rsqrt };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const PLANE _plane)
{
    return IsNormalized(_plane.normal);
}

// 세 평면이 만나는 한 점. 교점이 정의되지 않으면 영벡터를 리턴.
[[nodiscard]] JUG_MATH_API constexpr VEC3 IntersectOf3Planes(
    const PLANE _x,
    const PLANE _y,
    const PLANE _z)
{
    const VEC3  cross23 = Cross(_y.normal, _z.normal);
    const float denom   = Dot(_x.normal, cross23);
    if (IsZeroApprox(denom))   // 세 평면중 한 쌍이 거의 평행함 -> 교점이 정의되지 않음
    {
        return VEC3::Zero();
    }

    const VEC3 cross31 = Cross(_z.normal, _x.normal);
    const VEC3 cross12 = Cross(_x.normal, _y.normal);
    return (cross23 * -_x.d + cross31 * -_y.d + cross12 * -_z.d) / denom;
}

[[nodiscard]] JUG_MATH_API constexpr PLANE Xform(
    const PLANE _plane,
    const MAT4& _invTransMtx)   // Plane은 벡터이기 때문에 변환시 역-전치 행렬을 사용해야함.
{
    return PLANE { Xform(VEC4 { _plane.normal[0], _plane.normal[1], _plane.normal[2], _plane.d }, _invTransMtx) };
}

[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const PLANE _plane,
    const VEC3  _point)
{
    JUG_ASSERT(IsNormalized(_plane), "PLANE must be normalized to calculate distance");
    return Dot(_plane.normal, _point) + _plane.d;
}

}   // namespace jug