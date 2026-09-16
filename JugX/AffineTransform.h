#pragma once
#include "Matrix.h"
#include "Quaternion.h"

namespace jug
{

struct AFFINE_TRANSFORM
{
    JUG_MATH_API constexpr AFFINE_TRANSFORM() = default;
    JUG_MATH_API constexpr AFFINE_TRANSFORM(
        const VECTOR3    _scale,
        const QUATERNION _rotation,
        const VECTOR3    _translation)
        : scale(_scale)
        , rotation(_rotation)
        , translation(_translation)
    {
    }

    // =====================================================
    //  Factory
    // =====================================================

    [[nodiscard]] JUG_MATH_API constexpr static AFFINE_TRANSFORM MakeFromMatrix(
        const MATRIX& _mtx)
    {
        AFFINE_TRANSFORM ret;
        Decompose(_mtx, &ret.scale, &ret.rotation, &ret.translation);
        return ret;
    }

    // =====================================================
    //  Operators
    // =====================================================

    [[nodiscard]] JUG_MATH_API AFFINE_TRANSFORM operator*(
        const AFFINE_TRANSFORM& _other) const
    {
        const VECTOR3    s = scale * _other.scale;
        const QUATERNION r = rotation * _other.rotation;
        const VECTOR3    t = Mul(translation * _other.scale, _other.rotation) + _other.translation;
        return AFFINE_TRANSFORM { s, r, t };
    }

    [[nodiscard]] JUG_MATH_API AFFINE_TRANSFORM& operator*=(
        const AFFINE_TRANSFORM& _other)
    {
        *this = *this * _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API bool operator==(const AFFINE_TRANSFORM& _other) const
    {
        return scale == _other.scale && rotation == _other.rotation && translation == _other.translation;
    }

    [[nodiscard]] JUG_MATH_API bool operator!=(const AFFINE_TRANSFORM& _other) const
    {
        return !(*this == _other);
    }

    // =====================================================
    //  Utils
    // =====================================================

    [[nodiscard]] JUG_MATH_API constexpr MATRIX ToSRT() const
    {
        return MATRIX::MakeSRT(scale, rotation, translation);
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX ToInvSRT() const
    {
        return MATRIX::MakeInvSRT(scale, rotation, translation);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetRight() const
    {
        return Mul(UnitX<VECTOR3>(), rotation);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetLeft() const
    {
        return -GetRight();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetUp() const
    {
        return Mul(UnitY<VECTOR3>(), rotation);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetDown() const
    {
        return -GetUp();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetForward() const
    {
        return Mul(UnitZ<VECTOR3>(), rotation);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetBackward() const
    {
        return -GetForward();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetAxisX() const
    {
        return GetRight();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetAxisY() const
    {
        return GetUp();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetAxisZ() const
    {
        return GetForward();
    }

    // =====================================================
    //  Fields
    // =====================================================

    const static AFFINE_TRANSFORM kZero;
    const static AFFINE_TRANSFORM kIdentity;

    VECTOR3    scale;
    QUATERNION rotation;
    VECTOR3    translation;
};
static_assert(PodT<AFFINE_TRANSFORM>, "AFFINE_TRANSFORM must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr AFFINE_TRANSFORM AFFINE_TRANSFORM::kZero     = AFFINE_TRANSFORM { MathConstants<VECTOR3>::kZero, MathConstants<QUATERNION>::kZero, MathConstants<VECTOR3>::kZero };
inline constexpr AFFINE_TRANSFORM AFFINE_TRANSFORM::kIdentity = AFFINE_TRANSFORM { MathConstants<VECTOR3>::kOne, MathConstants<QUATERNION>::kIdentity, MathConstants<VECTOR3>::kZero };

template<>
struct MathConstants<AFFINE_TRANSFORM>
{
    constexpr static AFFINE_TRANSFORM kZero     = AFFINE_TRANSFORM::kZero;
    constexpr static AFFINE_TRANSFORM kIdentity = AFFINE_TRANSFORM::kIdentity;
};

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr AFFINE_TRANSFORM Inverse(
    const AFFINE_TRANSFORM& _transform)
{
    const VECTOR3    s = RcpSafe(_transform.scale);
    const QUATERNION r = Inverse(_transform.rotation);
    const VECTOR3    t = Mul(-_transform.translation, r) * s;
    return AFFINE_TRANSFORM { s, r, t };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const AFFINE_TRANSFORM& _x,
    const AFFINE_TRANSFORM& _y)
{
    return IsEqualApprox(_x.scale, _y.scale) && IsEqualApprox(_x.rotation, _y.rotation) && IsEqualApprox(_x.translation, _y.translation);
}

// =======================================================
//  Transform
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 TransformPoint(
    const VECTOR3           _point,
    const AFFINE_TRANSFORM& _transform)
{
    return Mul(_point * _transform.scale, _transform.rotation) + _transform.translation;
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 TransformVector(
    const VECTOR3           _v,
    const AFFINE_TRANSFORM& _transform)
{
    return Mul(_v * _transform.scale, _transform.rotation);
}

}   // namespace jug
