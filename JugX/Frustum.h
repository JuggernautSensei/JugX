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

// 절두체를 이루는 평면의 법선은 절두체 안쪽을 향한다.

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
        : planes { _right, _left, _up, _down, _far, _near }
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr FRUSTUM MakeFromMatrix(
        const MATRIX& _mtx)
    {
        const VECTOR4 c0 = VECTOR4 { _mtx[0][0], _mtx[1][0], _mtx[2][0], _mtx[3][0] };
        const VECTOR4 c1 = VECTOR4 { _mtx[0][1], _mtx[1][1], _mtx[2][1], _mtx[3][1] };
        const VECTOR4 c2 = VECTOR4 { _mtx[0][2], _mtx[1][2], _mtx[2][2], _mtx[3][2] };
        const VECTOR4 c3 = VECTOR4 { _mtx[0][3], _mtx[1][3], _mtx[2][3], _mtx[3][3] };

        return FRUSTUM {
            PLANE { c3 - c0 },
            PLANE { c3 + c0 },
            PLANE { c3 - c1 },
            PLANE { c3 + c1 },
            PLANE { c3 - c2 },
            PLANE { c2 }
        };
    }

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
        ret[eCorner::LeftBottomNear]  = XformPoint(VECTOR3 { -1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::LeftBottomFar]   = XformPoint(VECTOR3 { -1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::LeftTopNear]     = XformPoint(VECTOR3 { -1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::LeftTopFar]      = XformPoint(VECTOR3 { -1.f, 1.f, 1.f }, _invMtx);
        ret[eCorner::RightBottomNear] = XformPoint(VECTOR3 { 1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::RightBottomFar]  = XformPoint(VECTOR3 { 1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::RightTopNear]    = XformPoint(VECTOR3 { 1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::RightTopFar]     = XformPoint(VECTOR3 { 1.f, 1.f, 1.f }, _invMtx);
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

    const static FRUSTUM kZero;

    JUG_DISABLE_ANON_WARNING_BEGIN
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
    JUG_DISABLE_ANON_WARNING_END
};
static_assert(PodT<FRUSTUM>, "FRUSTUM must be POD type.");

// ===========================================
//  Constants
// ===========================================

inline constexpr FRUSTUM FRUSTUM::kZero = FRUSTUM {};

template<>
struct MathConstants<FRUSTUM>
{
    static constexpr FRUSTUM kZero = FRUSTUM::kZero;
};

// ===========================================
//  Method
// ===========================================

[[nodiscard]] JUG_MATH_API constexpr FRUSTUM Normalize(
    const FRUSTUM& _frustum)
{
    return FRUSTUM {
        Normalize(_frustum.planes[eFrustumPlane::Right]),
        Normalize(_frustum.planes[eFrustumPlane::Left]),
        Normalize(_frustum.planes[eFrustumPlane::Up]),
        Normalize(_frustum.planes[eFrustumPlane::Down]),
        Normalize(_frustum.planes[eFrustumPlane::Far]),
        Normalize(_frustum.planes[eFrustumPlane::Near])
    };
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

[[nodiscard]] JUG_MATH_API constexpr FRUSTUM Xform(
    const FRUSTUM& _frustum,
    const MATRIX&  _invTransMtx)   // 평면 변환 * 6임. 역-전치 행렬을 사용해야함.
{
    return FRUSTUM {
        Xform(_frustum.planes[eFrustumPlane::Right], _invTransMtx),
        Xform(_frustum.planes[eFrustumPlane::Left], _invTransMtx),
        Xform(_frustum.planes[eFrustumPlane::Up], _invTransMtx),
        Xform(_frustum.planes[eFrustumPlane::Down], _invTransMtx),
        Xform(_frustum.planes[eFrustumPlane::Far], _invTransMtx),
        Xform(_frustum.planes[eFrustumPlane::Near], _invTransMtx)
    };
}

}   // namespace jug