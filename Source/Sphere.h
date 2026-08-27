#pragma once
#include "Matrix.h"
#include "TypeTraits.h"
#include "Vector.h"

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

    // =======================================================
    //  Fields
    // =======================================================

    const static SPHERE kZero;
    const static SPHERE kUnit;

    VECTOR3 center;
    float   radius;
};

JUG_CHECK_POD_BY_STATIC_ASSERT(SPHERE);

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
//  Operators
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr SPHERE Transform(
    const SPHERE& _sphere,
    const MATRIX& _mtx)
{
    const VECTOR3 p  = MulPoint(_sphere.center, _mtx);
    const float   sx = Length(MulVector(UnitX<VECTOR3>(), _mtx));
    const float   sy = Length(MulVector(UnitY<VECTOR3>(), _mtx));
    const float   sz = Length(MulVector(UnitZ<VECTOR3>(), _mtx));
    return SPHERE { p, _sphere.radius * Max(sx, sy, sz) };
}

}   // namespace jug
