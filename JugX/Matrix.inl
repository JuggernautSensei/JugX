#pragma once
#include "Quaternion.h"

namespace jug
{

JUG_MATH_API constexpr MATRIX MATRIX::MakeRotation(
    const QUATERNION& _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");

    const float x = _q[0];
    const float y = _q[1];
    const float z = _q[2];
    const float w = _q[3];

    const float xx = x * x, yy = y * y, zz = z * z;
    const float xy = x * y, xz = x * z, yz = y * z;
    const float wx = w * x, wy = w * y, wz = w * z;

    return MATRIX {
        VECTOR4 { 1.f - 2.f * (yy + zz),       2.f * (xy + wz),       2.f * (xz - wy), 0.f },
        VECTOR4 {       2.f * (xy - wz), 1.f - 2.f * (xx + zz),       2.f * (yz + wx), 0.f },
        VECTOR4 {       2.f * (xz + wy),       2.f * (yz - wx), 1.f - 2.f * (xx + yy), 0.f },
        VECTOR4 {                   0.f,                   0.f,                   0.f, 1.f }
    };
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeSRT(
    const VECTOR3     _scale,
    const QUATERNION& _rotation,
    const VECTOR3     _translation)
{
    JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

    const MATRIX rot = MakeRotation(_rotation);
    return MATRIX {
        rot[0] * _scale[0],
        rot[1] * _scale[1],
        rot[2] * _scale[2],
        VECTOR4 { _translation, 1.f }
    };
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeInvSRT(
    const VECTOR3     _scale,
    const QUATERNION& _rotation,
    const VECTOR3     _translation)
{
    JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

    const MATRIX  rot  = MakeRotation(_rotation);
    const VECTOR3 r0   = rot[0].ToVector3();
    const VECTOR3 r1   = rot[1].ToVector3();
    const VECTOR3 r2   = rot[2].ToVector3();
    const VECTOR3 invs = Rcp(_scale);
    const VECTOR3 t    = _translation;

    return MATRIX {
        {         r0[0] * invs[0],         r1[0] * invs[1],         r2[0] * invs[2], 0.f },
        {         r0[1] * invs[0],         r1[1] * invs[1],         r2[1] * invs[2], 0.f },
        {         r0[2] * invs[0],         r1[2] * invs[1],         r2[2] * invs[2], 0.f },
        { -Dot(t, r0) * invs[0], -Dot(t, r1) * invs[1], -Dot(t, r2) * invs[2], 1.f }
    };
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeViewLookTo(
    const VECTOR3     _eye,
    const QUATERNION& _rotation)
{
    return MakeInvSRT(One<VECTOR3>(), _rotation, _eye);
}

JUG_MATH_API constexpr void Decompose(
    const MATRIX& _mtx,
    VECTOR3*      _pOutScaleOrNull,
    QUATERNION*   _pOutRotationOrNull,
    VECTOR3*      _pOutTranslationOrNull)
{
    if (_pOutRotationOrNull || _pOutScaleOrNull)
    {
        const VECTOR3 scale { Length(_mtx[0]), Length(_mtx[1]), Length(_mtx[2]) };
        if (_pOutScaleOrNull)
        {
            *_pOutScaleOrNull = scale;
        }

        if (_pOutRotationOrNull)
        {
            JUG_ASSERT(!IsZeroApprox(scale[0]) && !IsZeroApprox(scale[1]) && !IsZeroApprox(scale[2]), "Scale components cannot be zero");
            const MATRIX rot { _mtx[0] / scale[0], _mtx[1] / scale[1], _mtx[2] / scale[2], MathConstants<VECTOR4>::kUnitW };
            *_pOutRotationOrNull = QUATERNION::MakeFromMatrix(rot);
        }
    }

    if (_pOutTranslationOrNull)
    {
        *_pOutTranslationOrNull = _mtx[3].ToVector3();
    }
}

}   // namespace jug
