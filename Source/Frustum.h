#pragma once
#include "Plane.h"
#include "Corner.h"
#include "Matrix.h"
#include "TypeTraits.h"
#include "Vector.h"
#include "EnumArray.h"

namespace jug
{

enum class eFrustumPlane
{
    Right,
    Left,
    Up,
    Down,
    Far,
    Near
};

// =======================================================
//  Frustum
//   절두체를 이루는 평면의 법선은 절두체 안쪽을 향한다.
// =======================================================

struct FRUSTUM
{
    JUG_MATH_API FRUSTUM() = default;

    JUG_MATH_API constexpr FRUSTUM(
        const PLANE _right,
        const PLANE _left,
        const PLANE _up,
        const PLANE _down,
        const PLANE _far,
        const PLANE _near)
    {
        planes[eFrustumPlane::Right] = _right;
        planes[eFrustumPlane::Left]  = _left;
        planes[eFrustumPlane::Up]    = _up;
        planes[eFrustumPlane::Down]  = _down;
        planes[eFrustumPlane::Far]   = _far;
        planes[eFrustumPlane::Near]  = _near;
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API static constexpr FRUSTUM MakeFromMatrix(
        const MATRIX& _mtx)
    {
        const VECTOR4 c0 = VECTOR4 { _mtx.r[0].e[0], _mtx.r[1].e[0], _mtx.r[2].e[0], _mtx.r[3].e[0] };
        const VECTOR4 c1 = VECTOR4 { _mtx.r[0].e[1], _mtx.r[1].e[1], _mtx.r[2].e[1], _mtx.r[3].e[1] };
        const VECTOR4 c2 = VECTOR4 { _mtx.r[0].e[2], _mtx.r[1].e[2], _mtx.r[2].e[2], _mtx.r[3].e[2] };
        const VECTOR4 c3 = VECTOR4 { _mtx.r[0].e[3], _mtx.r[1].e[3], _mtx.r[2].e[3], _mtx.r[3].e[3] };

        FRUSTUM ret;
        ret.planes[eFrustumPlane::Right] = PLANE { c3 - c0 };
        ret.planes[eFrustumPlane::Left]  = PLANE { c3 + c0 };
        ret.planes[eFrustumPlane::Up]    = PLANE { c3 - c1 };
        ret.planes[eFrustumPlane::Down]  = PLANE { c3 + c1 };
        ret.planes[eFrustumPlane::Near]  = PLANE { c2 };
        ret.planes[eFrustumPlane::Far]   = PLANE { c3 - c2 };
        return ret;
    }

    // =======================================================
    //  Utils
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners() const
    {
        const PLANE rightP = planes[eFrustumPlane::Right];
        const PLANE leftP  = planes[eFrustumPlane::Left];
        const PLANE upP    = planes[eFrustumPlane::Up];
        const PLANE downP  = planes[eFrustumPlane::Down];
        const PLANE farP   = planes[eFrustumPlane::Far];
        const PLANE nearP  = planes[eFrustumPlane::Near];

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> ret;
        ret[eCorner::LeftBottomNear]  = IntersectOf3Planes(leftP, downP, nearP);
        ret[eCorner::LeftBottomFar]   = IntersectOf3Planes(leftP, downP, farP);
        ret[eCorner::LeftTopNear]     = IntersectOf3Planes(leftP, upP, nearP);
        ret[eCorner::LeftTopFar]      = IntersectOf3Planes(leftP, upP, farP);
        ret[eCorner::RightBottomNear] = IntersectOf3Planes(rightP, downP, nearP);
        ret[eCorner::RightBottomFar]  = IntersectOf3Planes(rightP, downP, farP);
        ret[eCorner::RightTopNear]    = IntersectOf3Planes(rightP, upP, nearP);
        ret[eCorner::RightTopFar]     = IntersectOf3Planes(rightP, upP, farP);
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners(
        const MATRIX& _invMtx)
    {
        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> ret;
        ret[eCorner::LeftBottomNear]  = MulPoint(VECTOR3 { -1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::LeftBottomFar]   = MulPoint(VECTOR3 { -1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::LeftTopNear]     = MulPoint(VECTOR3 { -1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::LeftTopFar]      = MulPoint(VECTOR3 { -1.f, 1.f, 1.f }, _invMtx);
        ret[eCorner::RightBottomNear] = MulPoint(VECTOR3 { 1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::RightBottomFar]  = MulPoint(VECTOR3 { 1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::RightTopNear]    = MulPoint(VECTOR3 { 1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::RightTopFar]     = MulPoint(VECTOR3 { 1.f, 1.f, 1.f }, _invMtx);
        return ret;
    }

    // Perspective Frustum
    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcViewSpaceCornersFromPersp(
        const float _fovRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        // z = 1
        const float tanHFov = Tan(_fovRad * 0.5f);
        const float h       = tanHFov;
        const float w       = h * _aspectRatio;

        // scala factor
        const float nw = w * _nearZ;
        const float nh = h * _nearZ;
        const float fw = w * _farZ;
        const float fh = h * _farZ;

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> ret;
        ret[eCorner::LeftBottomNear]  = VECTOR3 { -nw, -nh, _nearZ };
        ret[eCorner::LeftBottomFar]   = VECTOR3 { -fw, -fh, _farZ };
        ret[eCorner::LeftTopNear]     = VECTOR3 { -nw, nh, _nearZ };
        ret[eCorner::LeftTopFar]      = VECTOR3 { -fw, fh, _farZ };
        ret[eCorner::RightBottomNear] = VECTOR3 { nw, -nh, _nearZ };
        ret[eCorner::RightBottomFar]  = VECTOR3 { fw, -fh, _farZ };
        ret[eCorner::RightTopNear]    = VECTOR3 { nw, nh, _nearZ };
        ret[eCorner::RightTopFar]     = VECTOR3 { fw, fh, _farZ };
        return ret;
    }

    // Orthographic Frustum
    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcViewSpaceCornersFromOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        const float hw = _width * 0.5f;
        const float hh = _height * 0.5f;

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> ret;
        ret[eCorner::LeftBottomNear]  = VECTOR3 { -hw, -hh, _nearZ };
        ret[eCorner::LeftBottomFar]   = VECTOR3 { -hw, -hh, _farZ };
        ret[eCorner::LeftTopNear]     = VECTOR3 { -hw, hh, _nearZ };
        ret[eCorner::LeftTopFar]      = VECTOR3 { -hw, hh, _farZ };
        ret[eCorner::RightBottomNear] = VECTOR3 { hw, -hh, _nearZ };
        ret[eCorner::RightBottomFar]  = VECTOR3 { hw, -hh, _farZ };
        ret[eCorner::RightTopNear]    = VECTOR3 { hw, hh, _nearZ };
        ret[eCorner::RightTopFar]     = VECTOR3 { hw, hh, _farZ };
        return ret;
    }

    // =======================================================
    //  Fields
    // =======================================================

    const static FRUSTUM kZero;

    JUG_MATH_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            PLANE right;
            PLANE left;
            PLANE up;
            PLANE down;
            PLANE farZ;
            PLANE nearZ;
        };
        DIRECT_ENUM_ARRAY<eFrustumPlane, PLANE> planes;
    };
    JUG_MATH_DISABLE_ANON_WARNING_END
};

JUG_STATIC_ASSERT_POD(FRUSTUM);

// ========================================================
//  Constants
// ========================================================

inline constexpr FRUSTUM FRUSTUM::kZero = FRUSTUM {};

template<>
struct MathConstants<FRUSTUM>
{
    static constexpr FRUSTUM kZero = FRUSTUM::kZero;
};

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr FRUSTUM Transform(
    const FRUSTUM& _frustum,
    const MATRIX&  _invTransMtx)   // 평면 변환 * 6임. 역-전치 행렬을 사용해야함.
{
    FRUSTUM ret;
    ret.planes[eFrustumPlane::Right] = Transform(_frustum.planes[eFrustumPlane::Right], _invTransMtx);
    ret.planes[eFrustumPlane::Left]  = Transform(_frustum.planes[eFrustumPlane::Left], _invTransMtx);
    ret.planes[eFrustumPlane::Up]    = Transform(_frustum.planes[eFrustumPlane::Up], _invTransMtx);
    ret.planes[eFrustumPlane::Down]  = Transform(_frustum.planes[eFrustumPlane::Down], _invTransMtx);
    ret.planes[eFrustumPlane::Far]   = Transform(_frustum.planes[eFrustumPlane::Far], _invTransMtx);
    ret.planes[eFrustumPlane::Near]  = Transform(_frustum.planes[eFrustumPlane::Near], _invTransMtx);
    return ret;
}

[[nodiscard]] JUG_MATH_API constexpr FRUSTUM Normalize(
    const FRUSTUM& _frustum)
{
    FRUSTUM ret;
    ret.planes[eFrustumPlane::Right] = Normalize(_frustum.planes[eFrustumPlane::Right]);
    ret.planes[eFrustumPlane::Left]  = Normalize(_frustum.planes[eFrustumPlane::Left]);
    ret.planes[eFrustumPlane::Up]    = Normalize(_frustum.planes[eFrustumPlane::Up]);
    ret.planes[eFrustumPlane::Down]  = Normalize(_frustum.planes[eFrustumPlane::Down]);
    ret.planes[eFrustumPlane::Far]   = Normalize(_frustum.planes[eFrustumPlane::Far]);
    ret.planes[eFrustumPlane::Near]  = Normalize(_frustum.planes[eFrustumPlane::Near]);
    return ret;
}

[[nodiscard]] JUG_MATH_API inline bool IsNormalized(
    const FRUSTUM& _frustum)
{
    for (const PLANE p: _frustum.planes)
    {
        if (!IsNormalized(p))
        {
            return false;
        }
    }
    return true;
}

}   // namespace jug
