#pragma once
#include "MatrixReg.h"

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

// 아핀 행렬 전용. XformVector(UnitX) == row0 이므로 행 길이의 최대값이 최대 스케일이다.
[[nodiscard]] inline SPHERE JUG_VECTORCALL Xform(
    const SPHERE&    _sphere,
    const MATRIX_REG _mtx)
{
    // center(12) + radius(4) = 16바이트를 한 번에 읽고 한 번에 쓴다
    const simd::M128 s  = simd::Load(reinterpret_cast<const float*>(&_sphere));   // cx cy cz r
    const simd::M128 r0 = _mtx.r[0].v;
    const simd::M128 r1 = _mtx.r[1].v;
    const simd::M128 r2 = _mtx.r[2].v;

    simd::M128 center = simd::MultAdd(simd::Splat<0>(s), r0, _mtx.r[3].v);
    center            = simd::MultAdd(simd::Splat<1>(s), r1, center);
    center            = simd::MultAdd(simd::Splat<2>(s), r2, center);

    const simd::M128 scaleSq = simd::Max(simd::Max(simd::Dot3V(r0, r0), simd::Dot3V(r1, r1)), simd::Dot3V(r2, r2));
    const simd::M128 radius  = simd::Mult(simd::Splat<3>(s), simd::Sqrt(scaleSq));

    SPHERE ret;
    simd::Store(reinterpret_cast<float*>(&ret), simd::Blend<0b1000>(center, radius));
    return ret;
}

[[nodiscard]] JUG_MATH_API constexpr SPHERE Xform(
    const SPHERE& _sphere,
    const MATRIX& _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        return Xform(_sphere, Load(_mtx));
    }
#endif
    const float scaleSq = Max(LengthSq(_mtx[0].ToVector3()), LengthSq(_mtx[1].ToVector3()), LengthSq(_mtx[2].ToVector3()));
    return SPHERE { Xform(VECTOR4 { _sphere.center, 1.f }, _mtx).ToVector3(), _sphere.radius * Sqrt(scaleSq) };
}

}   // namespace jug
