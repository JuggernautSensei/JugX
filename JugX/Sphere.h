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

// 아핀 행렬 전용.
[[nodiscard]] JUG_MATH_API constexpr SPHERE Xform(
    const SPHERE& _sphere,
    const MATRIX& _mtx)
{
    // (cx cy cz r)
    simd::M128 s {};
    if (std::is_constant_evaluated())
    {
        s = simd::Blend<0b1000>(_sphere.center.ToM128(), simd::SetAll(_sphere.radius));
    }
    else
    {
        s = simd::Load(reinterpret_cast<const float*>(&_sphere));
    }

    const simd::M128X4 m = _mtx.ToM128X4();

    simd::M128 center = simd::MultAdd(simd::Splat<0>(s), m.r[0], m.r[3]);
    center            = simd::MultAdd(simd::Splat<1>(s), m.r[1], center);
    center            = simd::MultAdd(simd::Splat<2>(s), m.r[2], center);

    const simd::M128 scaleSq = simd::Max(simd::Max(simd::Dot3V(m.r[0], m.r[0]), simd::Dot3V(m.r[1], m.r[1])), simd::Dot3V(m.r[2], m.r[2]));
    const simd::M128 radius  = simd::Mult(simd::Splat<3>(s), simd::Sqrt(scaleSq));

    if (std::is_constant_evaluated())
    {
        return simd::NoInlineConstruct<SPHERE>(VECTOR3::FromM128(center), simd::Get<0>(radius));
    }
    else
    {
        SPHERE ret;
        simd::Store(reinterpret_cast<float*>(&ret), simd::Blend<0b1000>(center, radius));
        return ret;
    }
}

}   // namespace jug
