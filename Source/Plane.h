#pragma once
#include "Matrix.h"
#include "TypeTraits.h"
#include "Vector.h"

namespace jug
{

struct PLANE
{
    JUG_MATH_API  PLANE() = default;

    JUG_MATH_API constexpr PLANE(
        const VECTOR3 _normal,
        const float   _d)
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
        const VECTOR4 _v)
        : normal(_v.e[0], _v.e[1], _v.e[2])
        , d(_v.e[3])
    {
    }

    JUG_MATH_API constexpr PLANE(
        const VECTOR3 _point,
        const VECTOR3 _normal)
        : normal(_normal)
        , d(-Dot(_normal, _point))
    {
    }

    JUG_MATH_API constexpr PLANE(
        const VECTOR3 _p0,
        const VECTOR3 _p1,
        const VECTOR3 _p2)
        : normal(Cross(_p1 - _p0, _p2 - _p0))
        , d(-Dot(normal, _p0))
    {
    }

    // =======================================================
    //  Fields
    // =======================================================

    const static PLANE kZero;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            VECTOR3 normal;
            float   d;
        };
        VECTOR4 v;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(PodT<PLANE>, "PLANE must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr PLANE PLANE::kZero { Zero<VECTOR3>(), 0.f };

template<>
struct MathConstants<PLANE>
{
    constexpr static PLANE kZero = PLANE::kZero;
};

// =======================================================
//  Operators
// =======================================================

// 세 평면이 만나는 한 점. 교점이 정의되지 않으면 영벡터를 리턴.
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 IntersectOf3Planes(
    const PLANE _x,
    const PLANE _y,
    const PLANE _z)
{
    const VECTOR3 cross23 = Cross(_y.normal, _z.normal);
    const float   denom   = Dot(_x.normal, cross23);
    if (IsZeroApprox(denom))   // 세 평면중 한 쌍이 거의 평행함 -> 교점이 정의되지 않음
    {
        return MathConstants<VECTOR3>::kZero;
    }

    const VECTOR3 cross31 = Cross(_z.normal, _x.normal);
    const VECTOR3 cross12 = Cross(_x.normal, _y.normal);
    return (cross23 * -_x.d + cross31 * -_y.d + cross12 * -_z.d) / denom;
}

// 주의. Plane은 벡터가 주 정보기 때문에 변환시 역-전치 행렬을 사용해야함.
[[nodiscard]] JUG_MATH_API constexpr PLANE Transform(
    const PLANE   _plane,
    const MATRIX& _invTransMtx)
{
    if (std::is_constant_evaluated())  // constexpr 상황에서 union 활성멤버 제약 때문에 fallback
    {
        const VECTOR4 v { _plane.normal.e[0], _plane.normal.e[1], _plane.normal.e[2], _plane.d };
        return PLANE { Mul(v, _invTransMtx) };
    }

    return PLANE { Mul(_plane.v, _invTransMtx) };
}

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


[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const PLANE   _plane,
    const VECTOR3 _point)
{
    JUG_ASSERT(IsNormalized(_plane), "Plane must be normalized to calculate distance");
    return Dot(_plane.normal, _point) + _plane.d;
}


}   // namespace jug