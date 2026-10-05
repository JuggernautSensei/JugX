#pragma once
#include "Matrix.h"

namespace jug
{

struct TRIANGLE
{
    JUG_MATH_API TRIANGLE() = default;

    JUG_MATH_API constexpr TRIANGLE(
        const VEC3 _p0,
        const VEC3 _p1,
        const VEC3 _p2)
        : p0(_p0)
        , p1(_p1)
        , p2(_p2)
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetNormal() const
    {
        return Normalize(Cross(p1 - p0, p2 - p0));
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetCenter() const
    {
        return (p0 + p1 + p2) / 3.f;
    }

    [[nodiscard]] JUG_MATH_API static constexpr TRIANGLE Zero()
    {
        return TRIANGLE { VEC3::Zero(), VEC3::Zero(), VEC3::Zero() };
    }

    VEC3 p0;
    VEC3 p1;
    VEC3 p2;
};

static_assert(PodT<TRIANGLE>, "TRIANGLE must be POD type.");

// ========================================================
//  Method
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr TRIANGLE Xform(
    const TRIANGLE& _triangle,
    const MAT4&     _mtx)
{
    return TRIANGLE {
        XformPoint(_triangle.p0, _mtx),
        XformPoint(_triangle.p1, _mtx),
        XformPoint(_triangle.p2, _mtx)
    };
}

}   // namespace jug