#pragma once
#include "Matrix.h"

namespace jug
{

struct TRIANGLE
{
    JUG_MATH_API  TRIANGLE() = default;

    JUG_MATH_API constexpr TRIANGLE(
        const VECTOR3 _p0,
        const VECTOR3 _p1,
        const VECTOR3 _p2)
        : p0(_p0)
        , p1(_p1)
        , p2(_p2)
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetNormal() const
    {
        return Normalize(Cross(p1 - p0, p2 - p0));
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetCenter() const
    {
        return (p0 + p1 + p2) / 3.f;
    }

    const static TRIANGLE kZero;

    VECTOR3 p0;
    VECTOR3 p1;
    VECTOR3 p2;
};

static_assert(PodT<TRIANGLE>, "TRIANGLE must be POD type.");

// ========================================================
//  Constants
// ========================================================

inline constexpr TRIANGLE TRIANGLE::kZero = TRIANGLE { Zero<VECTOR3>(), Zero<VECTOR3>(), Zero<VECTOR3>() };

template<>
struct MathConstants<TRIANGLE>
{
    static constexpr TRIANGLE kZero = TRIANGLE::kZero;
};

// ========================================================
//  Method
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr TRIANGLE Xform(
    const TRIANGLE& _triangle,
    const MATRIX&   _mtx)
{
    return TRIANGLE {
        XformPoint(_triangle.p0, _mtx),
        XformPoint(_triangle.p1, _mtx),
        XformPoint(_triangle.p2, _mtx)
    };
}

}   // namespace jug