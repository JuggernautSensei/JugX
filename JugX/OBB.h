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
        const MATRIX& _mtx)
        : mtx(_mtx)
    {
    }

    [[nodiscard]] JUG_MATH_API static constexpr OBB MakeFromAABB(
        const AABB& _aabb)
    {
        return OBB {
            MATRIX { VECTOR4 { _aabb.extends[0], 0.f, 0.f, 0.f },
                    VECTOR4 { 0.f, _aabb.extends[1], 0.f, 0.f },
                    VECTOR4 { 0.f, 0.f, _aabb.extends[2], 0.f },
                    VECTOR4 { _aabb.center[0], _aabb.center[1], _aabb.center[2], 1.f } }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetCenter() const
    {
        return VECTOR3 { mtx[3][0], mtx[3][1], mtx[3][2] };
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetExtents() const
    {
        return VECTOR3 {
            Length(VECTOR3 { mtx[0][0], mtx[0][1], mtx[0][2] }),
            Length(VECTOR3 { mtx[1][0], mtx[1][1], mtx[1][2] }),
            Length(VECTOR3 { mtx[2][0], mtx[2][1], mtx[2][2] })
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

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners() const
    {
        const VECTOR3 axisX = VECTOR3 { mtx[0][0], mtx[0][1], mtx[0][2] };
        const VECTOR3 axisY = VECTOR3 { mtx[1][0], mtx[1][1], mtx[1][2] };
        const VECTOR3 axisZ = VECTOR3 { mtx[2][0], mtx[2][1], mtx[2][2] };
        const VECTOR3 c     = GetCenter();

        DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners;
        corners[eCorner::LeftBottomNear]  = c - axisX - axisY - axisZ;
        corners[eCorner::LeftBottomFar]   = c - axisX - axisY + axisZ;
        corners[eCorner::LeftTopNear]     = c - axisX + axisY - axisZ;
        corners[eCorner::LeftTopFar]      = c - axisX + axisY + axisZ;
        corners[eCorner::RightBottomNear] = c + axisX - axisY - axisZ;
        corners[eCorner::RightBottomFar]  = c + axisX - axisY + axisZ;
        corners[eCorner::RightTopNear]    = c + axisX + axisY - axisZ;
        corners[eCorner::RightTopFar]     = c + axisX + axisY + axisZ;
        return corners;
    }

    const static OBB kZero;
    const static OBB kUnit;

    MATRIX mtx;
};

static_assert(PodT<OBB>, "OBB must be POD type.");

// ========================================================
//  Constants
// ========================================================

inline constexpr OBB OBB::kZero = OBB { Zero<MATRIX>() };
inline constexpr OBB OBB::kUnit = OBB { Identity<MATRIX>() };

template<>
struct MathConstants<OBB>
{
    static constexpr OBB kZero = OBB::kZero;
    static constexpr OBB kUnit = OBB::kUnit;
};

// ========================================================
//  Method
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr OBB Xform(
    const OBB&    _obb,
    const MATRIX& _mtx)
{
    return OBB { _obb.mtx * _mtx };
}

}   // namespace jug
