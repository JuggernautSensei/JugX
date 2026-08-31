#pragma once
#include <array>

#include "AABB.h"
#include "Corner.h"
#include "TypeTraits.h"
#include "AffineTransform.h"
#include "Quaternion.h"
#include "Vector.h"
#include "EnumArray.h"

namespace jug
{

struct OBB
{
    JUG_MATH_API  OBB() = default;
    JUG_MATH_API constexpr explicit OBB(
        const AFFINE_TRANSFORM& _transform)
        : transform(_transform)
    {
    }

    // ========================================================
    //  Factory
    // ========================================================

    [[nodiscard]] JUG_MATH_API static constexpr OBB MakeFromAABB(
        const AABB& _aabb)
    {
        return OBB {
            AFFINE_TRANSFORM { _aabb.extends, MathConstants<QUATERNION>::kIdentity, _aabb.center }
        };
    }
    
    // =======================================================
    //  Utils
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetCenter() const
    {
        return transform.translation;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetExtents() const
    {
        return Abs(transform.scale);
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetWidth() const
    {
        return GetExtents().e[0] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetHeight() const
    {
        return GetExtents().e[1] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetDepth() const
    {
        return GetExtents().e[2] * 2.f;
    }

    [[nodiscard]] JUG_MATH_API constexpr DIRECT_ENUM_ARRAY<eCorner, VECTOR3> CalcCorners() const
    {
        const VECTOR3 axisX = transform.GetAxisX() * transform.scale.e[0];
        const VECTOR3 axisY = transform.GetAxisY() * transform.scale.e[1];
        const VECTOR3 axisZ = transform.GetAxisZ() * transform.scale.e[2];
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

    // =======================================================
    //  Fields
    // =======================================================

    const static OBB kZero;
    const static OBB kUnit;

    AFFINE_TRANSFORM transform;
};

JUG_STATIC_ASSERT_POD(OBB);

// ========================================================
//  Constants
// ========================================================

inline constexpr OBB OBB::kZero = OBB { Zero<AFFINE_TRANSFORM>() };
inline constexpr OBB OBB::kUnit = OBB { Identity<AFFINE_TRANSFORM>() };

template<>
struct MathConstants<OBB>
{
    static constexpr OBB kZero = OBB::kZero;
    static constexpr OBB kUnit = OBB::kUnit;
};

// ========================================================
//  Operators
// ========================================================

[[nodiscard]] JUG_MATH_API constexpr OBB Transform(
    const OBB&              _obb,
    const AFFINE_TRANSFORM& _transform)
{
    return OBB { _obb.transform * _transform };
}

}   // namespace jug
