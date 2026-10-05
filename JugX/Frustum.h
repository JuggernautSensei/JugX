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
        const MAT4& _mtx)
    {
        const VEC4 c0 = { _mtx[0][0], _mtx[1][0], _mtx[2][0], _mtx[3][0] };
        const VEC4 c1 = { _mtx[0][1], _mtx[1][1], _mtx[2][1], _mtx[3][1] };
        const VEC4 c2 = { _mtx[0][2], _mtx[1][2], _mtx[2][2], _mtx[3][2] };
        const VEC4 c3 = { _mtx[0][3], _mtx[1][3], _mtx[2][3], _mtx[3][3] };

        return FRUSTUM {
            PLANE { c3 - c0 },
            PLANE { c3 + c0 },
            PLANE { c3 - c1 },
            PLANE { c3 + c1 },
            PLANE { c3 - c2 },
            PLANE { c2 }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcCorners() const
    {
        const PLANE right = planes[eFrustumPlane::Right];
        const PLANE left  = planes[eFrustumPlane::Left];
        const PLANE up    = planes[eFrustumPlane::Up];
        const PLANE down  = planes[eFrustumPlane::Down];
        const PLANE farZ  = planes[eFrustumPlane::Far];
        const PLANE nearZ = planes[eFrustumPlane::Near];

        DIRECT_ENUM_ARRAY<eCorner, VEC3> ret = {};
        ret[eCorner::LeftBottomNear]         = IntersectOf3Planes(left, down, nearZ);
        ret[eCorner::LeftBottomFar]          = IntersectOf3Planes(left, down, farZ);
        ret[eCorner::LeftTopNear]            = IntersectOf3Planes(left, up, nearZ);
        ret[eCorner::LeftTopFar]             = IntersectOf3Planes(left, up, farZ);
        ret[eCorner::RightBottomNear]        = IntersectOf3Planes(right, down, nearZ);
        ret[eCorner::RightBottomFar]         = IntersectOf3Planes(right, down, farZ);
        ret[eCorner::RightTopNear]           = IntersectOf3Planes(right, up, nearZ);
        ret[eCorner::RightTopFar]            = IntersectOf3Planes(right, up, farZ);
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcCorners(
        const MAT4& _invMtx)
    {
        DIRECT_ENUM_ARRAY<eCorner, VEC3> ret = {};
        ret[eCorner::LeftBottomNear]         = XformPoint(VEC3 { -1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::LeftBottomFar]          = XformPoint(VEC3 { -1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::LeftTopNear]            = XformPoint(VEC3 { -1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::LeftTopFar]             = XformPoint(VEC3 { -1.f, 1.f, 1.f }, _invMtx);
        ret[eCorner::RightBottomNear]        = XformPoint(VEC3 { 1.f, -1.f, 0.f }, _invMtx);
        ret[eCorner::RightBottomFar]         = XformPoint(VEC3 { 1.f, -1.f, 1.f }, _invMtx);
        ret[eCorner::RightTopNear]           = XformPoint(VEC3 { 1.f, 1.f, 0.f }, _invMtx);
        ret[eCorner::RightTopFar]            = XformPoint(VEC3 { 1.f, 1.f, 1.f }, _invMtx);
        return ret;
    }

    // Perspective Frustum
    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcViewSpaceCornersFromPersp(
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

        DIRECT_ENUM_ARRAY<eCorner, VEC3> ret = {};
        ret[eCorner::LeftBottomNear]         = VEC3 { -nw, -nh, _nearZ };
        ret[eCorner::LeftBottomFar]          = VEC3 { -fw, -fh, _farZ };
        ret[eCorner::LeftTopNear]            = VEC3 { -nw, nh, _nearZ };
        ret[eCorner::LeftTopFar]             = VEC3 { -fw, fh, _farZ };
        ret[eCorner::RightBottomNear]        = VEC3 { nw, -nh, _nearZ };
        ret[eCorner::RightBottomFar]         = VEC3 { fw, -fh, _farZ };
        ret[eCorner::RightTopNear]           = VEC3 { nw, nh, _nearZ };
        ret[eCorner::RightTopFar]            = VEC3 { fw, fh, _farZ };
        return ret;
    }

    // Orthographic Frustum
    [[nodiscard]] JUG_MATH_API constexpr static DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcViewSpaceCornersFromOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        const float hw = _width * 0.5f;
        const float hh = _height * 0.5f;

        DIRECT_ENUM_ARRAY<eCorner, VEC3> ret = {};
        ret[eCorner::LeftBottomNear]         = VEC3 { -hw, -hh, _nearZ };
        ret[eCorner::LeftBottomFar]          = VEC3 { -hw, -hh, _farZ };
        ret[eCorner::LeftTopNear]            = VEC3 { -hw, hh, _nearZ };
        ret[eCorner::LeftTopFar]             = VEC3 { -hw, hh, _farZ };
        ret[eCorner::RightBottomNear]        = VEC3 { hw, -hh, _nearZ };
        ret[eCorner::RightBottomFar]         = VEC3 { hw, -hh, _farZ };
        ret[eCorner::RightTopNear]           = VEC3 { hw, hh, _nearZ };
        ret[eCorner::RightTopFar]            = VEC3 { hw, hh, _farZ };
        return ret;
    }

    [[nodiscard]] JUG_MATH_API static constexpr FRUSTUM Zero()
    {
        return FRUSTUM {};
    }

    DIRECT_ENUM_ARRAY<eFrustumPlane, PLANE> planes;
};
static_assert(PodT<FRUSTUM>, "FRUSTUM must be POD type.");

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
    const MAT4&    _invTransMtx)   // 평면 변환 * 6임. 역-전치 행렬을 사용해야함.
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