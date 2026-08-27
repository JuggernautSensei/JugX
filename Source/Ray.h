#pragma once
#include "Matrix.h"
#include "TypeTraits.h"
#include "Vector.h"

namespace jug
{

struct HIT
{
    VECTOR3 point;
    VECTOR3 normal;
    float   t;
    bool    bFrontFace;
};

struct RAY
{
    JUG_MATH_API  RAY() = default;

    JUG_MATH_API constexpr RAY(
        const VECTOR3 _origin,
        const VECTOR3 _dir)
        : origin(_origin)
        , dir(_dir)
    {
    }

    // ========================================================
    //  Factory
    // ========================================================

    [[nodiscard]] JUG_MATH_API static constexpr RAY MakeRayOnNDC(
        const float   _ndcX,
        const float   _ndcY,
        const MATRIX& _invViewProj)
    {
        const VECTOR3 near = MulPoint(VECTOR3 { _ndcX, _ndcY, 0.f }, _invViewProj);
        const VECTOR3 far  = MulPoint(VECTOR3 { _ndcX, _ndcY, 1.f }, _invViewProj);
        return RAY { near, Normalize(far - near) };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RAY MakeRayOnScreen(
        const int     _screenX,
        const int     _screenY,
        const int     _screenWidth,
        const int     _screenHeight,
        const MATRIX& _invViewProj)
    {
        const float xScreen = static_cast<float>(_screenX) + 0.5f;
        const float yScreen = static_cast<float>(_screenY) + 0.5f;
        const float xNDC    = (xScreen / static_cast<float>(_screenWidth)) * 2.f - 1.f;
        const float yNDC    = 1.f - (yScreen / static_cast<float>(_screenHeight)) * 2.f;
        return MakeRayOnNDC(xNDC, yNDC, _invViewProj);
    }

    // ========================================================
    //  Fields
    // ========================================================

    const static RAY kZero;

    VECTOR3 origin;
    VECTOR3 dir;
};

JUG_CHECK_POD_BY_STATIC_ASSERT(RAY);

// ========================================================
//  Constants
// ========================================================

inline constexpr RAY RAY::kZero = RAY { Zero<VECTOR3>(), UnitX<VECTOR3>() };

template<>
struct MathConstants<RAY>
{
    static constexpr RAY kZero = RAY::kZero;
};

// ========================================================
//  Operators
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const RAY& _ray)
{
    return IsNormalized(_ray.dir);
}

[[nodiscard]] JUG_MATH_API constexpr RAY Normalize(
    const RAY& _ray)
{
    return RAY { _ray.origin, Normalize(_ray.dir) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 RayAt(
    const RAY&  _ray,
    const float _t)
{
    return _ray.origin + _ray.dir * _t;
}


}   // namespace jug
