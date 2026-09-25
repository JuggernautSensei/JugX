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
        // 단위 사원수에서 1 - 2(yy + zz) = 2xx + 2ww - 1 이므로 행 i = q[i] * 2q + w * P_i - e_i
        //   P_0 = (2w, 2z, -2y), P_1 = (-2z, 2w, 2x), P_2 = (2y, -2x, 2w)
        // DirectXMath XMMatrixRotationQuaternion 보다 셔플이 적다.
        const simd::M128 q   = _q.ToM128();
        const simd::M128 q2  = simd::Add(q, q);
        const simd::M128 q2z = simd::ZeroW(q2);   // 2x 2y 2z 0
        const simd::M128 w   = simd::Splat<3>(q);

        const simd::M128 p0 = simd::Xor(simd::Shuffle2<3, 2, 1, 3>(q2, q2z), simd::Set(0.f, 0.f, -0.f, 0.f));   //  2w  2z -2y 0
        const simd::M128 p1 = simd::Xor(simd::Shuffle2<2, 3, 0, 3>(q2, q2z), simd::Set(-0.f, 0.f, 0.f, 0.f));   // -2z  2w  2x 0
        const simd::M128 p2 = simd::Xor(simd::Shuffle<1, 0, 3, 3>(q2), simd::Set(0.f, -0.f, 0.f, 0.f));         //  2y -2x  2w 2w

        const simd::M128 row0 = simd::MultAdd(simd::Splat<0>(q), q2z, simd::MultSub(w, p0, simd::Set(1.f, 0.f, 0.f, 0.f)));
        const simd::M128 row1 = simd::MultAdd(simd::Splat<1>(q), q2z, simd::MultSub(w, p1, simd::Set(0.f, 1.f, 0.f, 0.f)));
        const simd::M128 row2 = simd::ZeroW(simd::MultAdd(simd::Splat<2>(q), q2z, simd::MultSub(w, p2, simd::Set(0.f, 0.f, 1.f, 0.f))));
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
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        if (_pOutRotationOrNull || _pOutScaleOrNull)
        {
            // 행 길이 셋을 전치해서 한 번에: (|r0|, |r1|, |r2|, 0)
            const simd::M128 r0 = _mtx[0].ToM128();
            const simd::M128 r1 = _mtx[1].ToM128();
            const simd::M128 r2 = _mtx[2].ToM128();
            simd::M128       x  = simd::Mult(r0, r0);
            simd::M128       y  = simd::Mult(r1, r1);
            simd::M128       z  = simd::Mult(r2, r2);
            simd::M128       w  = simd::Zero();
            simd::Transpose4(x, y, z, w);
            const simd::M128 scale = simd::Sqrt(simd::Add(simd::Add(x, y), simd::Add(z, w)));
            if (_pOutScaleOrNull)
            {
                *_pOutScaleOrNull = VECTOR3::MakeFromM128(scale);
            }

            if (_pOutRotationOrNull)
            {
                JUG_ASSERT(!IsZeroApprox(simd::GetX(scale)) && !IsZeroApprox(simd::GetY(scale)) && !IsZeroApprox(simd::GetZ(scale)), "Scale components cannot be zero");
                const simd::M128 inv = simd::Div(simd::SetAll(1.f), scale);
                *_pOutRotationOrNull = QUATERNION::MakeFromMatrix(MATRIX::MakeFromM128(
                    simd::Mult(r0, simd::Splat<0>(inv)),
                    simd::Mult(r1, simd::Splat<1>(inv)),
                    simd::Mult(r2, simd::Splat<2>(inv)),
                    simd::Set(0.f, 0.f, 0.f, 1.f)));
            }
        }

        if (_pOutTranslationOrNull)
        {
            *_pOutTranslationOrNull = _mtx[3].ToVector3();
        }
        return;
    }
#endif
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
