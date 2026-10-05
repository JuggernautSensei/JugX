#pragma once
#include "Matrix.h"

namespace jug
{

struct SPHERE
{
    JUG_MATH_API SPHERE() = default;

    JUG_MATH_API constexpr SPHERE(
        const VEC3  _center,
        const float _radius)
        : center(_center)
        , radius(_radius)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr SPHERE Zero()
    {
        return SPHERE { VEC3::Zero(), 0.f };
    }

    [[nodiscard]] JUG_MATH_API static constexpr SPHERE Unit()
    {
        return SPHERE { VEC3::Zero(), 1.f };
    }

    VEC3  center;
    float radius;
};

static_assert(PodT<SPHERE>, "SPHERE must be POD type.");

// ========================================================
//  Method
// ========================================================

// 아핀 행렬 전용.
[[nodiscard]] JUG_MATH_API constexpr SPHERE Xform(
    const SPHERE& _sphere,
    const MAT4&   _mtx)
{
    // (cx cy cz r)
    simd::M128 s;
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
        return simd::NoInlineConstruct<SPHERE>(VEC3::FromM128(center), simd::Get<0>(radius));
    }
    else
    {
        SPHERE ret;
        simd::Store(reinterpret_cast<float*>(&ret), simd::Blend<0b1000>(center, radius));
        return ret;
    }
}

}   // namespace jug
