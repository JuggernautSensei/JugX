#pragma once
#include "Quaternion.h"

namespace jug
{

JUG_MATH_API constexpr MATRIX JUG_VECTORCALL MATRIX::MakeRotation(
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");

#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        // DirectXMath XMMatrixRotationQuaternion 과 같은 셔플
        const simd::M128 q  = _q.ToM128();
        const simd::M128 q0 = simd::Add(q, q);
        const simd::M128 q1 = simd::Mult(q, q0);   // 2xx 2yy 2zz 2ww

        // 대각: 1 - 2(yy + zz), 1 - 2(xx + zz), 1 - 2(xx + yy), 0
        const simd::M128 d0   = simd::ZeroW(simd::Shuffle<1, 0, 0, 3>(q1));
        const simd::M128 d1   = simd::ZeroW(simd::Shuffle<2, 2, 1, 3>(q1));
        const simd::M128 diag = simd::Sub(simd::Sub(simd::Set(1.f, 1.f, 1.f, 0.f), d0), d1);

        // 비대각: (2xz, 2xy, 2yz) ± (2wy, 2wz, 2wx)
        const simd::M128 a    = simd::Mult(simd::Shuffle<0, 0, 1, 3>(q), simd::Shuffle<2, 1, 2, 3>(q0));
        const simd::M128 b    = simd::Mult(simd::Splat<3>(q), simd::Shuffle<1, 2, 0, 3>(q0));
        const simd::M128 plus = simd::Add(a, b);
        const simd::M128 minu = simd::Sub(a, b);

        const simd::M128 v0 = simd::Shuffle<0, 2, 3, 1>(simd::Shuffle2<1, 2, 0, 1>(plus, minu));   // 2(xy+wz) 2(xz-wy) 2(xy-wz) 2(yz+wx)
        const simd::M128 v1 = simd::Shuffle<0, 2, 0, 2>(simd::Shuffle2<0, 0, 2, 2>(plus, minu));   // 2(xz+wy) 2(yz-wx) ...

        const simd::M128 row0 = simd::Shuffle<0, 2, 3, 1>(simd::Shuffle2<0, 3, 0, 1>(diag, v0));
        const simd::M128 row1 = simd::Shuffle<2, 0, 3, 1>(simd::Shuffle2<1, 3, 2, 3>(diag, v0));
        const simd::M128 row2 = simd::Shuffle2<0, 1, 2, 3>(v1, diag);
        return MakeFromM128(row0, row1, row2, simd::Set(0.f, 0.f, 0.f, 1.f));
    }
#endif
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

JUG_MATH_API constexpr MATRIX JUG_VECTORCALL MATRIX::MakeSRT(
    const VECTOR3    _scale,
    const QUATERNION _rotation,
    const VECTOR3    _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "QUATERNION must be normalized");
    JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        // 회전 행렬의 행마다 스케일을 곱하고 이동을 넣는다 (DirectXMath 는 스케일 행렬과 행렬 곱을 한다)
        const MATRIX     rot = MakeRotation(_rotation);
        const simd::M128 s   = _scale.ToM128();
        return MakeFromM128(
            simd::Mult(rot[0].ToM128(), simd::Splat<0>(s)),
            simd::Mult(rot[1].ToM128(), simd::Splat<1>(s)),
            simd::Mult(rot[2].ToM128(), simd::Splat<2>(s)),
            simd::Insert<3>(_translation.ToM128(), 1.f));
    }
#endif
    const float sx = _scale[0], sy = _scale[1], sz = _scale[2];
    const float xx = _rotation[0] * _rotation[0], yy = _rotation[1] * _rotation[1], zz = _rotation[2] * _rotation[2];
    const float xy = _rotation[0] * _rotation[1], xz = _rotation[0] * _rotation[2], yz = _rotation[1] * _rotation[2];
    const float wx = _rotation[3] * _rotation[0], wy = _rotation[3] * _rotation[1], wz = _rotation[3] * _rotation[2];

    return MATRIX {
        VECTOR4 { sx * (1.f - 2.f * (yy + zz)),       sx * (2.f * (xy + wz)),       sx * (2.f * (xz - wy)), 0.f },
        VECTOR4 {       sy * (2.f * (xy - wz)), sy * (1.f - 2.f * (xx + zz)),       sy * (2.f * (yz + wx)), 0.f },
        VECTOR4 {       sz * (2.f * (xz + wy)),       sz * (2.f * (yz - wx)), sz * (1.f - 2.f * (xx + yy)), 0.f },
        VECTOR4 {              _translation[0],              _translation[1],              _translation[2], 1.f }
    };
}

JUG_MATH_API constexpr MATRIX JUG_VECTORCALL MATRIX::MakeInvSRT(
    const VECTOR3    _scale,
    const QUATERNION _rotation,
    const VECTOR3    _translation)
{
    JUG_ASSERT(IsNormalized(_rotation), "Rotation must be normalized.");
    JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

    const float sx = _scale[0], sy = _scale[1], sz = _scale[2];
    const float isx = 1.f / sx, isy = 1.f / sy, isz = 1.f / sz;
    const float tx = _translation[0], ty = _translation[1], tz = _translation[2];
    const float xx = _rotation[0] * _rotation[0], yy = _rotation[1] * _rotation[1], zz = _rotation[2] * _rotation[2];
    const float xy = _rotation[0] * _rotation[1], xz = _rotation[0] * _rotation[2], yz = _rotation[1] * _rotation[2];
    const float wx = _rotation[3] * _rotation[0], wy = _rotation[3] * _rotation[1], wz = _rotation[3] * _rotation[2];

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

JUG_MATH_API constexpr MATRIX JUG_VECTORCALL MATRIX::MakeViewLookTo(
    const VECTOR3    _eye,
    const QUATERNION _rotation)
{
    return MakeInvSRT(One<VECTOR3>(), _rotation, _eye);
}

JUG_MATH_API constexpr void JUG_VECTORCALL Decompose(
    const MATRIX _mtx,
    VECTOR3*     _pOutScaleOrNull,
    QUATERNION*  _pOutRotationOrNull,
    VECTOR3*     _pOutTranslationOrNull)
{
    // S
    if (_pOutRotationOrNull || _pOutScaleOrNull)
    {
        const float sx = Length(_mtx[0]);
        const float sy = Length(_mtx[1]);
        const float sz = Length(_mtx[2]);
        if (_pOutScaleOrNull)
        {
            *_pOutScaleOrNull = VECTOR3 { sx, sy, sz };
        }

        // R
        if (_pOutRotationOrNull)
        {
            JUG_ASSERT(!IsZeroApprox(sx) && !IsZeroApprox(sy) && !IsZeroApprox(sz), "Scale components cannot be zero");
            const MATRIX rotMtx { _mtx[0] / sx, _mtx[1] / sy, _mtx[2] / sz, MathConstants<VECTOR4>::kUnitW };
            *_pOutRotationOrNull = QUATERNION::MakeFromMatrix(rotMtx);
        }
    }

    // T
    if (_pOutTranslationOrNull)
    {
        *_pOutTranslationOrNull = _mtx[3].ToVector3();
    }
}

}   // namespace jug
