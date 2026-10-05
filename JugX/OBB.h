#pragma once
#include "AABB.h"
#include "Corner.h"
#include "EnumArray.h"
#include "Matrix.h"
#include "TypeTraits.h"
#include "Vector.h"

namespace jug
{

struct OBB
{
    JUG_MATH_API OBB() = default;

    JUG_MATH_API constexpr explicit OBB(
        const MAT4& _mtx)
        : mtx(_mtx)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr OBB MakeFromAABB(
        const AABB& _aabb)
    {
        return OBB {
            MAT4 { VEC4 { _aabb.extends[0], 0.f, 0.f, 0.f },
                  VEC4 { 0.f, _aabb.extends[1], 0.f, 0.f },
                  VEC4 { 0.f, 0.f, _aabb.extends[2], 0.f },
                  VEC4 { _aabb.center[0], _aabb.center[1], _aabb.center[2], 1.f } }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetCenter() const
    {
        return VEC3 { mtx[3][0], mtx[3][1], mtx[3][2] };
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC3 GetExtents() const
    {
        return VEC3 {
            Length(VEC3 { mtx[0][0], mtx[0][1], mtx[0][2] }),
            Length(VEC3 { mtx[1][0], mtx[1][1], mtx[1][2] }),
            Length(VEC3 { mtx[2][0], mtx[2][1], mtx[2][2] })
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetWidth() const
    {
        return GetExtents()[0] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetHeight() const
    {
        return GetExtents()[1] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetDepth() const
    {
        return GetExtents()[2] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VEC3> CalcCorners() const
    {
        const VEC3 axisX = VEC3 { mtx[0][0], mtx[0][1], mtx[0][2] };
        const VEC3 axisY = VEC3 { mtx[1][0], mtx[1][1], mtx[1][2] };
        const VEC3 axisZ = VEC3 { mtx[2][0], mtx[2][1], mtx[2][2] };
        const VEC3 c     = GetCenter();

        DIRECT_ENUM_ARRAY<eCorner, VEC3> corners = {};
        corners[eCorner::LeftBottomNear]         = c - axisX - axisY - axisZ;
        corners[eCorner::LeftBottomFar]   = c - axisX - axisY + axisZ;
        corners[eCorner::LeftTopNear]     = c - axisX + axisY - axisZ;
        corners[eCorner::LeftTopFar]      = c - axisX + axisY + axisZ;
        corners[eCorner::RightBottomNear] = c + axisX - axisY - axisZ;
        corners[eCorner::RightBottomFar]  = c + axisX - axisY + axisZ;
        corners[eCorner::RightTopNear]    = c + axisX + axisY - axisZ;
        corners[eCorner::RightTopFar]     = c + axisX + axisY + axisZ;
        return corners;
    }

    [[nodiscard]] JUG_MATH_API static constexpr OBB Zero()
    {
        return OBB { MAT4::Zero() };
    }

    [[nodiscard]] JUG_MATH_API static constexpr OBB Unit()
    {
        return OBB { MAT4::Identity() };
    }

    MAT4 mtx;
};

static_assert(PodT<OBB>, "OBB must be POD type.");

// ========================================================
//  Method
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr OBB Xform(
    const OBB&  _obb,
    const MAT4& _mtx)
{
    return OBB { _obb.mtx * _mtx };
}

}   // namespace jug
