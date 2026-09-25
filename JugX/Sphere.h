#pragma once
#include "Matrix.h"

namespace jug
{

struct SPHERE
{
    JUG_MATH_API  SPHERE() = default;

    JUG_MATH_API constexpr SPHERE(
        const VECTOR3 _center,
        const float   _radius)
        : center(_center)
        , radius(_radius)
    {
    }

    const static SPHERE kZero;
    const static SPHERE kUnit;

    VECTOR3 center;
    float   radius;
};

static_assert(PodT<SPHERE>, "SPHERE must be POD type.");

// ========================================================
//  Constants
// ========================================================

inline constexpr SPHERE SPHERE::kZero = SPHERE { Zero<VECTOR3>(), 0.f };
inline constexpr SPHERE SPHERE::kUnit = SPHERE { Zero<VECTOR3>(), 1.f };

template<>
struct MathConstants<SPHERE>
{
    static constexpr SPHERE kZero = SPHERE::kZero;
    static constexpr SPHERE kOne  = SPHERE::kUnit;
};

// ========================================================
//  Method
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr SPHERE Xform(
    const SPHERE& _sphere,
    const MATRIX& _mtx)
{
    // XformVector(UnitX) == row0 이므로 행 길이의 최대값이 최대 스케일이다.
    const float scaleSq = Max(LengthSq(_mtx[0].ToVector3()), LengthSq(_mtx[1].ToVector3()), LengthSq(_mtx[2].ToVector3()));
    return SPHERE { XformPoint(_sphere.center, _mtx), _sphere.radius * Sqrt(scaleSq) };
}

}   // namespace jug
