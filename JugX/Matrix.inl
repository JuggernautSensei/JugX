#pragma once
#include "Quaternion.h"

namespace jug
{

JUG_MATH_API constexpr MATRIX MATRIX::MakeRotation(
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");

    const float x = _q.v.e[0];
    const float y = _q.v.e[1];
    const float z = _q.v.e[2];
    const float w = _q.v.e[3];

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
    const VECTOR3    _scale,
    const QUATERNION _rotation,
    const VECTOR3    _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "QUATERNION must be normalized");
    JUG_ASSERT(!IsZeroApprox(_scale.x) && !IsZeroApprox(_scale.y) && !IsZeroApprox(_scale.z), "Scale cannot have a zero component.");

    const float sx = _scale.e[0], sy = _scale.e[1], sz = _scale.e[2];
    const float xx = _rotation.v.e[0] * _rotation.v.e[0], yy = _rotation.v.e[1] * _rotation.v.e[1], zz = _rotation.v.e[2] * _rotation.v.e[2];
    const float xy = _rotation.v.e[0] * _rotation.v.e[1], xz = _rotation.v.e[0] * _rotation.v.e[2], yz = _rotation.v.e[1] * _rotation.v.e[2];
    const float wx = _rotation.v.e[3] * _rotation.v.e[0], wy = _rotation.v.e[3] * _rotation.v.e[1], wz = _rotation.v.e[3] * _rotation.v.e[2];

    return MATRIX {
        VECTOR4 { sx * (1.f - 2.f * (yy + zz)),       sx * (2.f * (xy + wz)),       sx * (2.f * (xz - wy)), 0.f },
        VECTOR4 {       sy * (2.f * (xy - wz)), sy * (1.f - 2.f * (xx + zz)),       sy * (2.f * (yz + wx)), 0.f },
        VECTOR4 {       sz * (2.f * (xz + wy)),       sz * (2.f * (yz - wx)), sz * (1.f - 2.f * (xx + yy)), 0.f },
        VECTOR4 {            _translation.e[0],            _translation.e[1],            _translation.e[2], 1.f }
    };
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeInvSRT(
    const VECTOR3    _scale,
    const QUATERNION _rotation,
    const VECTOR3    _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "Rotation must be normalized.");
    JUG_ASSERT(!IsZeroApprox(_scale.x) && !IsZeroApprox(_scale.y) && !IsZeroApprox(_scale.z), "Scale cannot have a zero component.");

    const float sx = _scale.e[0], sy = _scale.e[1], sz = _scale.e[2];
    const float isx = 1.f / sx, isy = 1.f / sy, isz = 1.f / sz;
    const float tx = _translation.e[0], ty = _translation.e[1], tz = _translation.e[2];
    const float xx = _rotation.v.e[0] * _rotation.v.e[0], yy = _rotation.v.e[1] * _rotation.v.e[1], zz = _rotation.v.e[2] * _rotation.v.e[2];
    const float xy = _rotation.v.e[0] * _rotation.v.e[1], xz = _rotation.v.e[0] * _rotation.v.e[2], yz = _rotation.v.e[1] * _rotation.v.e[2];
    const float wx = _rotation.v.e[3] * _rotation.v.e[0], wy = _rotation.v.e[3] * _rotation.v.e[1], wz = _rotation.v.e[3] * _rotation.v.e[2];

    const float r00 = 1.f - 2.f * (yy + zz);
    const float r01 = 2.f * (xy + wz);
    const float r02 = 2.f * (xz - wy);
    const float r10 = 2.f * (xy - wz);
    const float r11 = 1.f - 2.f * (xx + zz);
    const float r12 = 2.f * (yz + wx);
    const float r20 = 2.f * (xz + wy);
    const float r21 = 2.f * (yz - wx);
    const float r22 = 1.f - 2.f * (xx + yy);

    return MATRIX {
        VECTOR4 {                               r00 * isx,                               r10 * isy,                               r20 * isz, 0.f },
        VECTOR4 {                               r01 * isx,                               r11 * isy,                               r21 * isz, 0.f },
        VECTOR4 {                               r02 * isx,                               r12 * isy,                               r22 * isz, 0.f },
        VECTOR4 { -(tx * r00 + ty * r01 + tz * r02) * isx, -(tx * r10 + ty * r11 + tz * r12) * isy, -(tx * r20 + ty * r21 + tz * r22) * isz, 1.f }
    };
}

JUG_MATH_API constexpr MATRIX MATRIX::MakeViewLookTo(
    const VECTOR3    _eye,
    const QUATERNION _rotation)
{
    return MakeInvSRT(One<VECTOR3>(), _rotation, _eye);
}

JUG_MATH_API constexpr void Decompose(
    const MATRIX& _mtx,
    VECTOR3*      _pOutScaleOrNull,
    QUATERNION*   _pOutRotationOrNull,
    VECTOR3*      _pOutTranslationOrNull)
{
    // S
    if (_pOutRotationOrNull || _pOutScaleOrNull)
    {
        const float sx = Length(_mtx.r[0]);
        const float sy = Length(_mtx.r[1]);
        const float sz = Length(_mtx.r[2]);
        if (_pOutScaleOrNull)
        {
            *_pOutScaleOrNull = VECTOR3 { sx, sy, sz };
        }

        // R
        if (_pOutRotationOrNull)
        {
            JUG_ASSERT(!IsZeroApprox(sx) && !IsZeroApprox(sy) && !IsZeroApprox(sz), "Scale components cannot be zero");
            MATRIX rotMtx { _mtx.r[0] / sx, _mtx.r[1] / sy, _mtx.r[2] / sz, MathConstants<VECTOR4>::kUnitW };
            *_pOutRotationOrNull = QUATERNION::MakeFromMatrix(rotMtx);
        }
    }

    // T
    if (_pOutTranslationOrNull)
    {
        *_pOutTranslationOrNull = VECTOR3 { _mtx.r[3].e[0], _mtx.r[3].e[1], _mtx.r[3].e[2] };
    }
}

}   // namespace jug
