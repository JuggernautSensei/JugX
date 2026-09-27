#pragma once
#include "Quaternion.h"

namespace jug
{

JUG_MATH_API constexpr MATRIX MATRIX::MakeRotation(
    const QUAT& _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUAT must be normalized");

    return FromM128X4(matrix_detail::RotationQuat(_q.ToM128()));
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeSRT(
    const VECTOR3 _scale,
    const QUAT&   _rotation,
    const VECTOR3 _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "QUAT must be normalized");

    return FromM128X4(matrix_detail::ScaleTranslate(matrix_detail::RotationQuat(_rotation.ToM128()), _scale.ToM128(), _translation.ToM128()));
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeInvSRT(
    const VECTOR3 _scale,
    const QUAT    _rotation,
    const VECTOR3 _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "QUAT must be normalized");
    JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

    return FromM128X4(matrix_detail::InvSRT(matrix_detail::RotationQuat(_rotation.ToM128()), _scale.ToM128(), _translation.ToM128()));
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeViewLookTo(
    const VECTOR3 _eye,
    const QUAT    _rotation)
{
    JUG_ASSERT(IsNormalized(_rotation), "QUAT must be normalized");

    return FromM128X4(matrix_detail::InvRT(matrix_detail::RotationQuat(_rotation.ToM128()), _eye.ToM128()));
}

JUG_MATH_API constexpr void Decompose(
    const MATRIX& _mtx,
    VECTOR3*      _pOutScaleOrNull,
    QUAT*         _pOutRotationOrNull,
    VECTOR3*      _pOutTranslationOrNull)
{
    const simd::M128X4 m = _mtx.ToM128X4();
    if (_pOutRotationOrNull || _pOutScaleOrNull)
    {
        simd::M128 x = simd::Mult(m.r[0], m.r[0]);
        simd::M128 y = simd::Mult(m.r[1], m.r[1]);
        simd::M128 z = simd::Mult(m.r[2], m.r[2]);
        simd::M128 w = simd::Zero();
        simd::Transpose4(x, y, z, w);
        const simd::M128 scale = simd::Sqrt(simd::Add(simd::Add(x, y), simd::Add(z, w)));
        if (_pOutScaleOrNull)
        {
            *_pOutScaleOrNull = VECTOR3::FromM128(scale);
        }

        if (_pOutRotationOrNull)
        {
            JUG_ASSERT(simd::MoveMask(simd::CmpLt(scale, simd::SetAll(kEpsilon))) == 0b1000, "Scale components cannot be zero");

            const simd::M128 inv = simd::Div(simd::SetAll(1.f), scale);
            *_pOutRotationOrNull = QUAT::MakeFromMatrix(MATRIX::FromM128X4(simd::M128X4 {
                {
                 simd::Mult(m.r[0], simd::Splat<0>(inv)),
                 simd::Mult(m.r[1], simd::Splat<1>(inv)),
                 simd::Mult(m.r[2], simd::Splat<2>(inv)),
                 simd::Set(0.f, 0.f, 0.f, 1.f),
                 }
            }));
        }
    }

    if (_pOutTranslationOrNull)
    {
        *_pOutTranslationOrNull = VECTOR3::FromM128(m.r[3]);
    }
}

}   // namespace jug
