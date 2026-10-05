#pragma once
#include "Matrix.h"

namespace jug
{

struct HIT
{
    VEC3  point;
    VEC3  normal;
    float t;
    bool  bFrontFace;
};

struct RAY
{
    JUG_MATH_API RAY() = default;

    JUG_MATH_API constexpr RAY(
        const VEC3 _origin,
        const VEC3 _dir)
        : origin(_origin)
        , dir(_dir)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr RAY MakeRayOnNDC(
        const float _ndcX,
        const float _ndcY,
        const MAT4& _invViewProj)
    {
        const VEC3 n = XformPoint(VEC3 { _ndcX, _ndcY, 0.f }, _invViewProj);
        const VEC3 f = XformPoint(VEC3 { _ndcX, _ndcY, 1.f }, _invViewProj);
        return RAY { n, Normalize(f - n) };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RAY MakeRayOnScreen(
        const int   _screenX,
        const int   _screenY,
        const int   _screenWidth,
        const int   _screenHeight,
        const MAT4& _invViewProj)
    {
        const float xScreen = static_cast<float>(_screenX) + 0.5f;
        const float yScreen = static_cast<float>(_screenY) + 0.5f;
        const float xNDC    = (xScreen / static_cast<float>(_screenWidth)) * 2.f - 1.f;
        const float yNDC    = 1.f - (yScreen / static_cast<float>(_screenHeight)) * 2.f;
        return MakeRayOnNDC(xNDC, yNDC, _invViewProj);
    }

    [[nodiscard]] JUG_MATH_API static constexpr RAY Zero()
    {
        return RAY { VEC3::Zero(), VEC3::UnitX() };
    }

    VEC3 origin;
    VEC3 dir;
};

static_assert(PodT<RAY>, "RAY must be POD type.");

// ========================================================
//  Method
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

[[nodiscard]] JUG_MATH_API constexpr VEC3 RayAt(
    const RAY&  _ray,
    const float _t)
{
    return _ray.origin + _ray.dir * _t;
}

}   // namespace jug
