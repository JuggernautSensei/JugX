#pragma once
#include "Vector.h"

namespace jug
{

struct QUATERNION;

// 1. 4x4 행렬
// 2. 행 우선
// 3. 행 벡터
// 4. 왼손 좌표계
//
// 레지스터 타입 (VECTOR4 행 4개 = HVA). JUG_VECTORCALL 함수에 값으로 넘기면 xmm 레지스터 4개로 오간다.
// 인자가 여럿이면 첫 MATRIX 만 값, 나머지는 const& 로 받는다 (DirectXMath FXMMATRIX / CXMMATRIX 와 같은 규칙).
struct alignas(16) MATRIX
{
    using ValueT = float;

    JUG_MATH_API MATRIX() = default;

    JUG_MATH_API constexpr MATRIX(
        const VECTOR4 _row0,
        const VECTOR4 _row1,
        const VECTOR4 _row2,
        const VECTOR4 _row3)
        : r { _row0, _row1, _row2, _row3 }
    {
    }

    // clang-format off
    JUG_MATH_API constexpr MATRIX(
        const float _m00, const float _m01, const float _m02, const float _m03,
        const float _m10, const float _m11, const float _m12, const float _m13,
        const float _m20, const float _m21, const float _m22, const float _m23,
        const float _m30, const float _m31, const float _m32, const float _m33)
        : r {
           VECTOR4 { _m00, _m01, _m02, _m03 },
           VECTOR4 { _m10, _m11, _m12, _m13 },
           VECTOR4 { _m20, _m21, _m22, _m23 },
           VECTOR4 { _m30, _m31, _m32, _m33 }
        }
    {
    }
    // clang-format on

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static constexpr MATRIX JUG_VECTORCALL MakeFromM128(
        const simd::M128 _row0,
        const simd::M128 _row1,
        const simd::M128 _row2,
        const simd::M128 _row3)
    {
        MATRIX ret;
        ret.r[0] = VECTOR4::MakeFromM128(_row0);
        ret.r[1] = VECTOR4::MakeFromM128(_row1);
        ret.r[2] = VECTOR4::MakeFromM128(_row2);
        ret.r[3] = VECTOR4::MakeFromM128(_row3);
        return ret;
    }
#endif

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeTranslation(
        const VECTOR3 _translation)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            return MakeFromM128(simd::Set(1.f, 0.f, 0.f, 0.f), simd::Set(0.f, 1.f, 0.f, 0.f), simd::Set(0.f, 0.f, 1.f, 0.f), simd::Blend<0b1000>(_translation.ToM128(), simd::SetAll(1.f)));
        }
#endif
        return MATRIX {
            VECTOR4 { 1.f, 0.f, 0.f, 0.f },
            VECTOR4 { 0.f, 1.f, 0.f, 0.f },
            VECTOR4 { 0.f, 0.f, 1.f, 0.f },
            VECTOR4 { _translation, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeScale(
        const VECTOR3 _scale)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 s    = _scale.ToM128();
            const simd::M128 zero = simd::Zero();
            return MakeFromM128(simd::Blend<0b0001>(zero, s), simd::Blend<0b0010>(zero, s), simd::Blend<0b0100>(zero, s), simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        return MATRIX {
            { _scale[0],       0.f,       0.f, 0.f },
            {       0.f, _scale[1],       0.f, 0.f },
            {       0.f,       0.f, _scale[2], 0.f },
            {       0.f,       0.f,       0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotationX(
        const float _rad)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 sc = simd::SinCos1(_rad);   // s c s c
            const simd::M128 r1 = simd::Blend<0b0110>(simd::Zero(), sc);                                                                          // 0  c s 0
            const simd::M128 r2 = simd::Xor(simd::Blend<0b0110>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc)), simd::Set(0.f, -0.f, 0.f, 0.f));   // 0 -s c 0
            return MakeFromM128(simd::Set(1.f, 0.f, 0.f, 0.f), r1, r2, simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
        return MATRIX {
            { 1.f, 0.f, 0.f, 0.f },
            { 0.f,   c,   s, 0.f },
            { 0.f,  -s,   c, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotationY(
        const float _rad)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 sc = simd::SinCos1(_rad);   // s c s c
            const simd::M128 r0 = simd::Xor(simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc)), simd::Set(0.f, 0.f, -0.f, 0.f));   // c 0 -s 0
            const simd::M128 r2 = simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc));                                              // s 0  c 0
            return MakeFromM128(r0, simd::Set(0.f, 1.f, 0.f, 0.f), r2, simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
        return MATRIX {
            {   c, 0.f,  -s, 0.f },
            { 0.f, 1.f, 0.f, 0.f },
            {   s, 0.f,   c, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotationZ(
        const float _rad)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 sc = simd::SinCos1(_rad);   // s c s c
            const simd::M128 r0 = simd::Blend<0b0011>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc));                                     //  c s 0 0
            const simd::M128 r1 = simd::Xor(simd::Blend<0b0011>(simd::Zero(), sc), simd::Set(-0.f, 0.f, 0.f, 0.f));                    // -s c 0 0
            return MakeFromM128(r0, r1, simd::Set(0.f, 0.f, 1.f, 0.f), simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
        return MATRIX {
            {   c,   s, 0.f, 0.f },
            {  -s,   c, 0.f, 0.f },
            { 0.f, 0.f, 1.f, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotation(
        const VECTOR3 _pyrRad)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            // 세 각의 sin/cos 를 한 번에. A = (sp*sy, cp, sp*cy, 0), B = (cy, 0, -sy, 0) 로 두면
            // r0 = sr*A + cr*B, r1 = cr*A - sr*B, r2 = (cp*sy, -sp, cp*cy, 0)
            simd::M128 s;
            simd::M128 c;
            simd::SinCos(_pyrRad.ToM128(), s, c);
            const simd::M128 spcp = simd::Shuffle2<0, 0, 0, 0>(s, c);                                                      // sp sp cp cp
            const simd::M128 sycy = simd::Shuffle2<1, 1, 1, 1>(s, c);                                                      // sy sy cy cy
            const simd::M128 t    = simd::Blend<0b1010>(simd::Shuffle<0, 0, 2, 2>(sycy), simd::Set(0.f, 1.f, 0.f, 0.f));   // sy 1 cy 0
            const simd::M128 a    = simd::Mult(simd::Shuffle<0, 2, 0, 2>(spcp), t);
            const simd::M128 b    = simd::Mult(simd::Shuffle<2, 0, 0, 0>(sycy), simd::Set(1.f, 0.f, -1.f, 0.f));
            const simd::M128 sr   = simd::Splat<2>(s);
            const simd::M128 cr   = simd::Splat<2>(c);
            const simd::M128 r0   = simd::MultAdd(sr, a, simd::Mult(cr, b));
            const simd::M128 r1   = simd::NegMultAdd(sr, b, simd::Mult(cr, a));
            const simd::M128 r2   = simd::Mult(simd::Mult(simd::Shuffle<2, 0, 2, 2>(spcp), simd::Set(1.f, -1.f, 1.f, 0.f)), t);
            return MakeFromM128(r0, r1, r2, simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        float sx = 0.f, cx = 0.f;
        float sy = 0.f, cy = 0.f;
        float sz = 0.f, cz = 0.f;
        SinCos(_pyrRad[0], sx, cx);
        SinCos(_pyrRad[1], sy, cy);
        SinCos(_pyrRad[2], sz, cz);

        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return MATRIX {
            { r00, r01, r02, 0.f },
            { r10, r11, r12, 0.f },
            { r20, r21, r22, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotation(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            // 행 = n[i] * (1 - c) * n + (대각은 c, 나머지는 s * n 의 반대칭 행렬)
            const simd::M128 sc = simd::SinCos1(_rad);   // s c s c
            const simd::M128 n  = _axis.ToM128();
            const simd::M128 c  = simd::Splat<1>(sc);
            const simd::M128 tn = simd::Mult(simd::Sub(simd::SetAll(1.f), c), n);
            const simd::M128 sn = simd::Mult(simd::Splat<0>(sc), n);   // sx sy sz 0
            const simd::M128 e0 = simd::Blend<0b0001>(simd::Xor(simd::Shuffle<3, 2, 1, 3>(sn), simd::Set(0.f, 0.f, -0.f, 0.f)), c);   //   c  sz -sy 0
            const simd::M128 e1 = simd::Blend<0b0010>(simd::Xor(simd::Shuffle<2, 3, 0, 3>(sn), simd::Set(-0.f, 0.f, 0.f, 0.f)), c);   // -sz   c  sx 0
            const simd::M128 e2 = simd::Blend<0b0100>(simd::Xor(simd::Shuffle<1, 0, 3, 3>(sn), simd::Set(0.f, -0.f, 0.f, 0.f)), c);   //  sy -sx   c 0
            return MakeFromM128(simd::MultAdd(simd::Splat<0>(n), tn, e0), simd::MultAdd(simd::Splat<1>(n), tn, e1), simd::MultAdd(simd::Splat<2>(n), tn, e2), simd::Set(0.f, 0.f, 0.f, 1.f));
        }
#endif
        const float x = _axis[0];
        const float y = _axis[1];
        const float z = _axis[2];
        float       s = 0.f;
        float       c = 0.f;
        SinCos(_rad, s, c);
        const float t = 1.f - c;

        return MATRIX {
            {     t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0.f },
            { t * x * y - s * z,     t * y * y + c, t * y * z + s * x, 0.f },
            { t * x * z + s * y, t * y * z - s * x,     t * z * z + c, 0.f },
            {               0.f,               0.f,               0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotation(
        QUATERNION _q);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeRotationLookTo(
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_dir, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f = _dir;
        const VECTOR3 r = Normalize(Cross(_up, f));
        const VECTOR3 u = Cross(f, r);
        return MATRIX {
            VECTOR4 { r, 0.f },
            VECTOR4 { u, 0.f },
            VECTOR4 { f, 0.f },
            VECTOR4 { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f = _forward;
        const VECTOR3 r = Normalize(Cross(_up, f));
        const VECTOR3 u = Cross(f, r);

        return MATRIX {
            VECTOR4 { r * _scale[0], 0.f },
            VECTOR4 { u * _scale[1], 0.f },
            VECTOR4 { f * _scale[2], 0.f },
            VECTOR4 { _translation, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

        float sx = 0.f, cx = 0.f;
        float sy = 0.f, cy = 0.f;
        float sz = 0.f, cz = 0.f;
        SinCos(_pyrRad[0], sx, cx);
        SinCos(_pyrRad[1], sy, cy);
        SinCos(_pyrRad[2], sz, cz);
        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return MATRIX {
            VECTOR4 { _scale[0] * r00, _scale[0] * r01, _scale[0] * r02, 0.f },
            VECTOR4 { _scale[1] * r10, _scale[1] * r11, _scale[1] * r12, 0.f },
            VECTOR4 { _scale[2] * r20, _scale[2] * r21, _scale[2] * r22, 0.f },
            VECTOR4 { _translation, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeSRT(
        VECTOR3    _scale,
        QUATERNION _rotation,
        VECTOR3    _translation);

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f    = _forward;
        const VECTOR3 r    = Normalize(Cross(_up, f));
        const VECTOR3 u    = Cross(f, r);
        const VECTOR3 invs = RcpSafe(_scale);
        const float   tx = _translation[0], ty = _translation[1], tz = _translation[2];

        return MATRIX {
            {                                 r[0] * invs[0],                                 u[0] * invs[1],                                 f[0] * invs[2], 0.f },
            {                                 r[1] * invs[0],                                 u[1] * invs[1],                                 f[1] * invs[2], 0.f },
            {                                 r[2] * invs[0],                                 u[2] * invs[1],                                 f[2] * invs[2], 0.f },
            { -(tx * r[0] + ty * r[1] + tz * r[2]) * invs[0], -(tx * u[0] + ty * u[1] + tz * u[2]) * invs[1], -(tx * f[0] + ty * f[1] + tz * f[2]) * invs[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX JUG_VECTORCALL MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

        float sx = 0.f, cx = 0.f;
        float sy = 0.f, cy = 0.f;
        float sz = 0.f, cz = 0.f;
        SinCos(_pyrRad[0], sx, cx);
        SinCos(_pyrRad[1], sy, cy);
        SinCos(_pyrRad[2], sz, cz);

        const float isx = 1.f / _scale[0], isy = 1.f / _scale[1], isz = 1.f / _scale[2];
        const float tx = _translation[0], ty = _translation[1], tz = _translation[2];

        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return {
            {                               r00 * isx,                               r10 * isy,                               r20 * isz, 0.f },
            {                               r01 * isx,                               r11 * isy,                               r21 * isz, 0.f },
            {                               r02 * isx,                               r12 * isy,                               r22 * isz, 0.f },
            { -(tx * r00 + ty * r01 + tz * r02) * isx, -(tx * r10 + ty * r11 + tz * r12) * isy, -(tx * r20 + ty * r21 + tz * r22) * isz, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX JUG_VECTORCALL MakeInvSRT(
        VECTOR3    _scale,
        QUATERNION _rotation,
        VECTOR3    _translation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");

            // 축 r, u, f 와 -(eye · 축) 을 행으로 두고 전치한다 (_dir 은 정규화돼 있어야 한다)
            const simd::M128 f      = _dir.ToM128();
            const simd::M128 r      = simd::Normalize3(simd::Cross3(_up.ToM128(), f));
            const simd::M128 u      = simd::Cross3(f, r);
            const simd::M128 negEye = simd::Negate(_eye.ToM128());

            simd::M128 row0 = simd::Blend<0b1000>(r, simd::Dot3V(r, negEye));
            simd::M128 row1 = simd::Blend<0b1000>(u, simd::Dot3V(u, negEye));
            simd::M128 row2 = simd::Blend<0b1000>(f, simd::Dot3V(f, negEye));
            simd::M128 row3 = simd::Set(0.f, 0.f, 0.f, 1.f);
            simd::Transpose4(row0, row1, row2, row3);
            return MakeFromM128(row0, row1, row2, row3);
        }
#endif
        return MakeInvSRT(One<VECTOR3>(), _dir, _up, _eye);
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _pyrRad)
    {
        return MakeInvSRT(One<VECTOR3>(), _pyrRad, _eye);
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeViewLookTo(
        VECTOR3    _eye,
        QUATERNION _rotation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakePersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            // 나눗셈 셋 (cot/aspect, cot, f/(f-n)) 을 벡터 나눗셈 한 번으로
            const simd::M128 sc   = simd::SinCos1(_fovYRad * 0.5f);   // s c s c
            const simd::M128 num  = simd::Blend<0b0100>(simd::Splat<1>(sc), simd::SetAll(_farZ));
            const simd::M128 den  = simd::Blend<0b0100>(simd::Mult(simd::Splat<0>(sc), simd::Set(_aspectRatio, 1.f, 1.f, 1.f)), simd::SetAll(_farZ - _nearZ));
            const simd::M128 q    = simd::Div(num, den);   // xs ys range -
            const simd::M128 zero = simd::Zero();
            return MakeFromM128(
                simd::Blend<0b0001>(zero, q),
                simd::Blend<0b0010>(zero, q),
                simd::Blend<0b1000>(simd::Blend<0b0100>(zero, q), simd::SetAll(1.f)),
                simd::Blend<0b0100>(zero, simd::Mult(q, simd::SetAll(-_nearZ))));
        }
#endif
        float s = 0.f;
        float c = 0.f;
        SinCos(_fovYRad * 0.5f, s, c);
        const float ys    = c / s;
        const float xs    = ys / _aspectRatio;
        const float range = _farZ / (_farZ - _nearZ);

        return MATRIX {
            {  xs, 0.f,             0.f, 0.f },
            { 0.f,  ys,             0.f, 0.f },
            { 0.f, 0.f,           range, 1.f },
            { 0.f, 0.f, -range * _nearZ, 0.f }
        };
    }

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeInvPersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const float d    = _farZ - _nearZ;
        const float invF = Tan(_fovYRad * 0.5f);

        return MATRIX {
            { _aspectRatio * invF,  0.f, 0.f,                   0.f },
            {                 0.f, invF, 0.f,                   0.f },
            {                 0.f,  0.f, 0.f, -d / (_farZ * _nearZ) },
            {                 0.f,  0.f, 1.f,          1.f / _nearZ }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeOrtho(
        const float _left,
        const float _right,
        const float _bottom,
        const float _top,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsEqualApprox(_left, _right), "Invalid view volume width");
        JUG_ASSERT(!IsEqualApprox(_bottom, _top), "Invalid view volume height");
        JUG_ASSERT(_farZ > _nearZ, "Invalid near/far plane");

#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            // 역수 셋을 벡터 나눗셈 한 번으로
            const simd::M128 rcp  = simd::Div(simd::SetAll(1.f), simd::Set(_right - _left, _top - _bottom, _farZ - _nearZ, 1.f));   // invW invH range 1
            const simd::M128 two  = simd::Add(rcp, rcp);
            const simd::M128 zero = simd::Zero();
            const simd::M128 r3   = simd::Blend<0b1000>(simd::Negate(simd::Mult(simd::Set(_left + _right, _top + _bottom, _nearZ, 0.f), rcp)), simd::SetAll(1.f));
            return MakeFromM128(simd::Blend<0b0001>(zero, two), simd::Blend<0b0010>(zero, two), simd::Blend<0b0100>(zero, rcp), r3);
        }
#endif
        const float invW  = 1.f / (_right - _left);
        const float invH  = 1.f / (_top - _bottom);
        const float range = 1.f / (_farZ - _nearZ);

        return MATRIX {
            {               2.f * invW,                      0.f,             0.f, 0.f },
            {                      0.f,               2.f * invH,             0.f, 0.f },
            {                      0.f,                      0.f,           range, 0.f },
            { -(_left + _right) * invW, -(_top + _bottom) * invH, -range * _nearZ, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeInvOrtho(
        const float _left,
        const float _right,
        const float _bottom,
        const float _top,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsEqualApprox(_left, _right), "Invalid view volume width");
        JUG_ASSERT(!IsEqualApprox(_bottom, _top), "Invalid view volume height");
        JUG_ASSERT(_farZ > _nearZ, "Invalid near/far plane");

        const float w = _right - _left;
        const float h = _top - _bottom;
        const float d = _farZ - _nearZ;

        return MATRIX {
            {                w * 0.5f,                     0.f,    0.f, 0.f },
            {                     0.f,                h * 0.5f,    0.f, 0.f },
            {                     0.f,                     0.f,      d, 0.f },
            { (_right + _left) * 0.5f, (_top + _bottom) * 0.5f, _nearZ, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL MakeInvOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeInvOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4& operator[](
        const size_t _row)
    {
        return r[_row];
    }

    [[nodiscard]] JUG_MATH_API constexpr const VECTOR4& operator[](
        const size_t _row) const
    {
        return r[_row];
    }

    [[nodiscard]] JUG_MATH_API constexpr float operator()(
        const size_t _row,
        const size_t _col) const
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API float* GetPtr()
    {
        return r[0].GetPtr();
    }

    [[nodiscard]] JUG_MATH_API const float* GetPtr() const
    {
        return r[0].GetPtr();
    }

    // =======================================================
    //  Compound Assignment (정의는 자유 연산자 뒤)
    // =======================================================

    JUG_MATH_API constexpr MATRIX& operator+=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator-=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator*=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator*=(float _scalar);
    JUG_MATH_API constexpr MATRIX& operator/=(float _scalar);

    const static MATRIX kIdentity;
    const static MATRIX kZero;

    constexpr static size_t kRow = 4;
    constexpr static size_t kCol = 4;

    VECTOR4 r[4];
};

static_assert(sizeof(MATRIX) == 64, "Matrix4x4 must be tightly packed");
static_assert(alignof(MATRIX) == 16, "Matrix4x4 must be 16-byte aligned for SIMD");
static_assert(PodT<MATRIX>, "Matrix4x4 must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr MATRIX MATRIX::kIdentity = {
    VECTOR4 { 1.f, 0.f, 0.f, 0.f },
    VECTOR4 { 0.f, 1.f, 0.f, 0.f },
    VECTOR4 { 0.f, 0.f, 1.f, 0.f },
    VECTOR4 { 0.f, 0.f, 0.f, 1.f }
};

inline constexpr MATRIX MATRIX::kZero = {
    VECTOR4 { 0.f },
    VECTOR4 { 0.f },
    VECTOR4 { 0.f },
    VECTOR4 { 0.f }
};

template<>
struct MathConstants<MATRIX>
{
    constexpr static MATRIX kIdentity = MATRIX::kIdentity;
    constexpr static MATRIX kZero     = MATRIX::kZero;
};

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator+(
    const MATRIX  _x,
    const MATRIX& _y)
{
    return MATRIX { _x[0] + _y[0], _x[1] + _y[1], _x[2] + _y[2], _x[3] + _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator-(
    const MATRIX  _x,
    const MATRIX& _y)
{
    return MATRIX { _x[0] - _y[0], _x[1] - _y[1], _x[2] - _y[2], _x[3] - _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator*(
    const MATRIX _mtx,
    const float  _scalar)
{
    return MATRIX { _mtx[0] * _scalar, _mtx[1] * _scalar, _mtx[2] * _scalar, _mtx[3] * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator*(
    const float  _scalar,
    const MATRIX _mtx)
{
    return _mtx * _scalar;
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator/(
    const MATRIX _mtx,
    const float  _scalar)
{
    return _mtx * (1.f / _scalar);
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator/(
    const float  _scalar,
    const MATRIX _mtx)
{
    return MATRIX { _scalar / _mtx[0], _scalar / _mtx[1], _scalar / _mtx[2], _scalar / _mtx[3] };
}

namespace matrix_detail
{
    // constexpr / SIMD 없음 전용. SIMD 함수 본문을 작게 둬야 MSVC 가 인라인한다 (죽은 스칼라 경로도 인라인 비용에 잡힌다).
    [[nodiscard]] JUG_MATH_API constexpr MATRIX Multiply(
        const MATRIX& _x,
        const MATRIX& _y)
    {
        const float b00 = _y[0][0], b01 = _y[0][1], b02 = _y[0][2], b03 = _y[0][3];
        const float b10 = _y[1][0], b11 = _y[1][1], b12 = _y[1][2], b13 = _y[1][3];
        const float b20 = _y[2][0], b21 = _y[2][1], b22 = _y[2][2], b23 = _y[2][3];
        const float b30 = _y[3][0], b31 = _y[3][1], b32 = _y[3][2], b33 = _y[3][3];

        const float a00 = _x[0][0], a01 = _x[0][1], a02 = _x[0][2], a03 = _x[0][3];
        const float a10 = _x[1][0], a11 = _x[1][1], a12 = _x[1][2], a13 = _x[1][3];
        const float a20 = _x[2][0], a21 = _x[2][1], a22 = _x[2][2], a23 = _x[2][3];
        const float a30 = _x[3][0], a31 = _x[3][1], a32 = _x[3][2], a33 = _x[3][3];

        // clang-format off
        return MATRIX {
            (a00 * b00 + a01 * b10) + (a02 * b20 + a03 * b30),
            (a00 * b01 + a01 * b11) + (a02 * b21 + a03 * b31),
            (a00 * b02 + a01 * b12) + (a02 * b22 + a03 * b32),
            (a00 * b03 + a01 * b13) + (a02 * b23 + a03 * b33),

            (a10 * b00 + a11 * b10) + (a12 * b20 + a13 * b30),
            (a10 * b01 + a11 * b11) + (a12 * b21 + a13 * b31),
            (a10 * b02 + a11 * b12) + (a12 * b22 + a13 * b32),
            (a10 * b03 + a11 * b13) + (a12 * b23 + a13 * b33),

            (a20 * b00 + a21 * b10) + (a22 * b20 + a23 * b30),
            (a20 * b01 + a21 * b11) + (a22 * b21 + a23 * b31),
            (a20 * b02 + a21 * b12) + (a22 * b22 + a23 * b32),
            (a20 * b03 + a21 * b13) + (a22 * b23 + a23 * b33),

            (a30 * b00 + a31 * b10) + (a32 * b20 + a33 * b30),
            (a30 * b01 + a31 * b11) + (a32 * b21 + a33 * b31),
            (a30 * b02 + a31 * b12) + (a32 * b22 + a33 * b32),
            (a30 * b03 + a31 * b13) + (a32 * b23 + a33 * b33)
        };
        // clang-format on
    }
}   // namespace matrix_detail

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL operator*(
    const MATRIX  _x,
    const MATRIX& _y)
{
#if defined(JUG_MATH_SIMD_FMA)
    if (!std::is_constant_evaluated())
    {
        // 두 행씩 256비트로. _y 의 행은 메모리에서 바로 128 -> 256 브로드캐스트한다.
        const __m256 x01 = _mm256_insertf128_ps(_mm256_castps128_ps256(_x[0].ToM128()), _x[1].ToM128(), 1);
        const __m256 x23 = _mm256_insertf128_ps(_mm256_castps128_ps256(_x[2].ToM128()), _x[3].ToM128(), 1);
        const __m256 b0  = _mm256_broadcast_ps(reinterpret_cast<const __m128*>(_y[0].GetPtr()));
        const __m256 b1  = _mm256_broadcast_ps(reinterpret_cast<const __m128*>(_y[1].GetPtr()));
        const __m256 b2  = _mm256_broadcast_ps(reinterpret_cast<const __m128*>(_y[2].GetPtr()));
        const __m256 b3  = _mm256_broadcast_ps(reinterpret_cast<const __m128*>(_y[3].GetPtr()));

        // FMA 가 있으면 곱셈 1 + FMA 3 체인이 명령이 가장 적다 (행 둘이 독립이라 지연은 겹친다)
        __m256 r01 = _mm256_mul_ps(_mm256_shuffle_ps(x01, x01, 0x00), b0);
        __m256 r23 = _mm256_mul_ps(_mm256_shuffle_ps(x23, x23, 0x00), b0);
        r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0x55), b1, r01);
        r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0x55), b1, r23);
        r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xaa), b2, r01);
        r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xaa), b2, r23);
        r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xff), b3, r01);
        r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xff), b3, r23);

        return MATRIX::MakeFromM128(_mm256_castps256_ps128(r01), _mm256_extractf128_ps(r01, 1), _mm256_castps256_ps128(r23), _mm256_extractf128_ps(r23, 1));
    }
#elif defined(JUG_SIMD_AVAILABLE)
    if (!std::is_constant_evaluated())
    {
        const simd::M128 b0 = _y[0].ToM128();
        const simd::M128 b1 = _y[1].ToM128();
        const simd::M128 b2 = _y[2].ToM128();
        const simd::M128 b3 = _y[3].ToM128();

        // (x*b0 + z*b2) + (y*b1 + w*b3) 트리
        const simd::M128 a0   = _x[0].ToM128();
        const simd::M128 row0 = simd::Add(simd::MultAdd(simd::Splat<2>(a0), b2, simd::Mult(simd::Splat<0>(a0), b0)), simd::MultAdd(simd::Splat<3>(a0), b3, simd::Mult(simd::Splat<1>(a0), b1)));
        const simd::M128 a1   = _x[1].ToM128();
        const simd::M128 row1 = simd::Add(simd::MultAdd(simd::Splat<2>(a1), b2, simd::Mult(simd::Splat<0>(a1), b0)), simd::MultAdd(simd::Splat<3>(a1), b3, simd::Mult(simd::Splat<1>(a1), b1)));
        const simd::M128 a2   = _x[2].ToM128();
        const simd::M128 row2 = simd::Add(simd::MultAdd(simd::Splat<2>(a2), b2, simd::Mult(simd::Splat<0>(a2), b0)), simd::MultAdd(simd::Splat<3>(a2), b3, simd::Mult(simd::Splat<1>(a2), b1)));
        const simd::M128 a3   = _x[3].ToM128();
        const simd::M128 row3 = simd::Add(simd::MultAdd(simd::Splat<2>(a3), b2, simd::Mult(simd::Splat<0>(a3), b0)), simd::MultAdd(simd::Splat<3>(a3), b3, simd::Mult(simd::Splat<1>(a3), b1)));

        return MATRIX::MakeFromM128(row0, row1, row2, row3);
    }
#endif
    return matrix_detail::Multiply(_x, _y);
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator==(
    const MATRIX  _x,
    const MATRIX& _y)
{
    return _x[0] == _y[0] && _x[1] == _y[1] && _x[2] == _y[2] && _x[3] == _y[3];
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator!=(
    const MATRIX  _x,
    const MATRIX& _y)
{
    return !(_x == _y);
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator+=(
    const MATRIX& _other)
{
    *this = *this + _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator-=(
    const MATRIX& _other)
{
    *this = *this - _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator*=(
    const MATRIX& _other)
{
    *this = *this * _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

// =======================================================
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL Abs(
    const MATRIX _mtx)
{
    return MATRIX { Abs(_mtx[0]), Abs(_mtx[1]), Abs(_mtx[2]), Abs(_mtx[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsEqualApprox(
    const MATRIX  _x,
    const MATRIX& _y,
    const float   _epsilon = kEpsilon)
{
    return IsEqualApprox(_x[0], _y[0], _epsilon) && IsEqualApprox(_x[1], _y[1], _epsilon) && IsEqualApprox(_x[2], _y[2], _epsilon) && IsEqualApprox(_x[3], _y[3], _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL Transpose(
    const MATRIX _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        simd::M128 r0 = _mtx[0].ToM128();
        simd::M128 r1 = _mtx[1].ToM128();
        simd::M128 r2 = _mtx[2].ToM128();
        simd::M128 r3 = _mtx[3].ToM128();
        simd::Transpose4(r0, r1, r2, r3);
        return MATRIX::MakeFromM128(r0, r1, r2, r3);
    }
#endif
    return MATRIX {
        { _mtx[0][0], _mtx[1][0], _mtx[2][0], _mtx[3][0] },
        { _mtx[0][1], _mtx[1][1], _mtx[2][1], _mtx[3][1] },
        { _mtx[0][2], _mtx[1][2], _mtx[2][2], _mtx[3][2] },
        { _mtx[0][3], _mtx[1][3], _mtx[2][3], _mtx[3][3] }
    };
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Determinant(
    const MATRIX _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        // DirectXMath XMMatrixDeterminant 와 같은 식: 아래 두 행의 2x2 소행렬식 6개를 두 번에 나눠 구한다
        const simd::M128 r0 = _mtx[0].ToM128();
        const simd::M128 r1 = _mtx[1].ToM128();
        const simd::M128 r2 = _mtx[2].ToM128();
        const simd::M128 r3 = _mtx[3].ToM128();

        simd::M128 p0 = simd::Mult(simd::Shuffle<1, 0, 0, 0>(r2), simd::Shuffle<2, 2, 1, 1>(r3));
        simd::M128 p1 = simd::Mult(simd::Shuffle<1, 0, 0, 0>(r2), simd::Shuffle<3, 3, 3, 2>(r3));
        simd::M128 p2 = simd::Mult(simd::Shuffle<2, 2, 1, 1>(r2), simd::Shuffle<3, 3, 3, 2>(r3));

        p0 = simd::NegMultAdd(simd::Shuffle<2, 2, 1, 1>(r2), simd::Shuffle<1, 0, 0, 0>(r3), p0);
        p1 = simd::NegMultAdd(simd::Shuffle<3, 3, 3, 2>(r2), simd::Shuffle<1, 0, 0, 0>(r3), p1);
        p2 = simd::NegMultAdd(simd::Shuffle<3, 3, 3, 2>(r2), simd::Shuffle<2, 2, 1, 1>(r3), p2);

        const simd::M128 s   = simd::Mult(r0, simd::Set(1.f, -1.f, 1.f, -1.f));
        simd::M128       ret = simd::Mult(simd::Shuffle<3, 3, 3, 2>(r1), p0);
        ret                  = simd::NegMultAdd(simd::Shuffle<2, 2, 1, 1>(r1), p1, ret);
        ret                  = simd::MultAdd(simd::Shuffle<1, 0, 0, 0>(r1), p2, ret);
        return simd::Dot4(s, ret);
    }
#endif
    const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
    const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
    const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
    const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

    const float s0 = m00 * m11 - m10 * m01;
    const float s1 = m00 * m12 - m10 * m02;
    const float s2 = m00 * m13 - m10 * m03;
    const float s3 = m01 * m12 - m11 * m02;
    const float s4 = m01 * m13 - m11 * m03;
    const float s5 = m02 * m13 - m12 * m03;
    const float c5 = m22 * m33 - m32 * m23;
    const float c4 = m21 * m33 - m31 * m23;
    const float c3 = m21 * m32 - m31 * m22;
    const float c2 = m20 * m33 - m30 * m23;
    const float c1 = m20 * m32 - m30 * m22;
    const float c0 = m20 * m31 - m30 * m21;
    return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

// 역행렬이 없으면 (행렬식이 정확히 0) 영행렬
[[nodiscard]] JUG_MATH_API constexpr MATRIX JUG_VECTORCALL Inverse(
    const MATRIX _mtx,
    float*       _outDetOrNull = nullptr)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        // 2x2 블록 방법 (Eric Zhang, "Fast 4x4 Matrix Inverse with SSE SIMD"). DirectXMath 의 여인수 전개보다 연산이 30% 넘게 적다.
        //     | A B |    2x2 블록은 레지스터 하나에 (m00 m01 m10 m11) 로 담는다. X# 는 수반 행렬.
        // M = | C D |    M^-1 = 1/|M| * | X Y |  의 각 블록을 수반 행렬 꼴로 구한 뒤 마지막 셔플에서 되돌린다.
        //                               | Z W |
        const simd::M128 r0 = _mtx[0].ToM128();
        const simd::M128 r1 = _mtx[1].ToM128();
        const simd::M128 r2 = _mtx[2].ToM128();
        const simd::M128 r3 = _mtx[3].ToM128();

        const simd::M128 a = simd::Shuffle2<0, 1, 0, 1>(r0, r1);
        const simd::M128 b = simd::Shuffle2<2, 3, 2, 3>(r0, r1);
        const simd::M128 c = simd::Shuffle2<0, 1, 0, 1>(r2, r3);
        const simd::M128 d = simd::Shuffle2<2, 3, 2, 3>(r2, r3);

        // (|A| |B| |C| |D|)
        const simd::M128 detSub = simd::NegMultAdd(simd::Shuffle2<1, 3, 1, 3>(r0, r2), simd::Shuffle2<0, 2, 0, 2>(r1, r3), simd::Mult(simd::Shuffle2<0, 2, 0, 2>(r0, r2), simd::Shuffle2<1, 3, 1, 3>(r1, r3)));
        const simd::M128 detA   = simd::Splat<0>(detSub);
        const simd::M128 detB   = simd::Splat<1>(detSub);
        const simd::M128 detC   = simd::Splat<2>(detSub);
        const simd::M128 detD   = simd::Splat<3>(detSub);

        // D#C, A#B
        const simd::M128 dc = simd::NegMultAdd(simd::Shuffle<1, 1, 2, 2>(d), simd::Shuffle<2, 3, 0, 1>(c), simd::Mult(simd::Shuffle<3, 3, 0, 0>(d), c));
        const simd::M128 ab = simd::NegMultAdd(simd::Shuffle<1, 1, 2, 2>(a), simd::Shuffle<2, 3, 0, 1>(b), simd::Mult(simd::Shuffle<3, 3, 0, 0>(a), b));

        // X# = |D|A - B(D#C),   W# = |A|D - C(A#B)
        simd::M128 x = simd::NegMultAdd(simd::Shuffle<1, 0, 3, 2>(b), simd::Shuffle<2, 1, 2, 1>(dc), simd::MultSub(detD, a, simd::Mult(b, simd::Shuffle<0, 3, 0, 3>(dc))));
        simd::M128 w = simd::NegMultAdd(simd::Shuffle<1, 0, 3, 2>(c), simd::Shuffle<2, 1, 2, 1>(ab), simd::MultSub(detA, d, simd::Mult(c, simd::Shuffle<0, 3, 0, 3>(ab))));

        // Y# = |B|C - D(A#B)#,  Z# = |C|B - A(D#C)#
        simd::M128 y = simd::MultAdd(simd::Shuffle<1, 0, 3, 2>(d), simd::Shuffle<2, 1, 2, 1>(ab), simd::MultSub(detB, c, simd::Mult(d, simd::Shuffle<3, 0, 3, 0>(ab))));
        simd::M128 z = simd::MultAdd(simd::Shuffle<1, 0, 3, 2>(a), simd::Shuffle<2, 1, 2, 1>(dc), simd::MultSub(detC, b, simd::Mult(a, simd::Shuffle<3, 0, 3, 0>(dc))));

        // |M| = |A||D| + |B||C| - tr((A#B)(D#C))
        const simd::M128 tr   = simd::SumAll(simd::Mult(ab, simd::Shuffle<0, 2, 1, 3>(dc)));
        const simd::M128 detM = simd::Sub(simd::MultAdd(detB, detC, simd::Mult(detA, detD)), tr);
        const float      det  = simd::GetX(detM);
        if (_outDetOrNull)
        {
            *_outDetOrNull = det;
        }
        if (det == 0.f)
        {
            return MATRIX::kZero;
        }

        // (1/|M|, -1/|M|, -1/|M|, 1/|M|) 를 곱하고, 수반 행렬 셔플과 행 배치 셔플을 한 번에
        const simd::M128 rDetM = simd::Div(simd::Set(1.f, -1.f, -1.f, 1.f), detM);
        x                      = simd::Mult(x, rDetM);
        y                      = simd::Mult(y, rDetM);
        z                      = simd::Mult(z, rDetM);
        w                      = simd::Mult(w, rDetM);
        return MATRIX::MakeFromM128(simd::Shuffle2<3, 1, 3, 1>(x, y), simd::Shuffle2<2, 0, 2, 0>(x, y), simd::Shuffle2<3, 1, 3, 1>(z, w), simd::Shuffle2<2, 0, 2, 0>(z, w));
    }
#endif
    const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
    const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
    const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
    const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

    const float s0 = m00 * m11 - m10 * m01;
    const float s1 = m00 * m12 - m10 * m02;
    const float s2 = m00 * m13 - m10 * m03;
    const float s3 = m01 * m12 - m11 * m02;
    const float s4 = m01 * m13 - m11 * m03;
    const float s5 = m02 * m13 - m12 * m03;
    const float c5 = m22 * m33 - m32 * m23;
    const float c4 = m21 * m33 - m31 * m23;
    const float c3 = m21 * m32 - m31 * m22;
    const float c2 = m20 * m33 - m30 * m23;
    const float c1 = m20 * m32 - m30 * m22;
    const float c0 = m20 * m31 - m30 * m21;

    const float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    if (_outDetOrNull)
    {
        *_outDetOrNull = det;
    }
    if (det == 0.f)
    {
        return MathConstants<MATRIX>::kZero;
    }
    const float invDet = 1.f / det;

    // clang-format off
    return MATRIX {
        ( m11 * c5 - m12 * c4 + m13 * c3) * invDet,
        (-m01 * c5 + m02 * c4 - m03 * c3) * invDet,
        ( m31 * s5 - m32 * s4 + m33 * s3) * invDet,
        (-m21 * s5 + m22 * s4 - m23 * s3) * invDet,

        (-m10 * c5 + m12 * c2 - m13 * c1) * invDet,
        ( m00 * c5 - m02 * c2 + m03 * c1) * invDet,
        (-m30 * s5 + m32 * s2 - m33 * s1) * invDet,
        ( m20 * s5 - m22 * s2 + m23 * s1) * invDet,

        ( m10 * c4 - m11 * c2 + m13 * c0) * invDet,
        (-m00 * c4 + m01 * c2 - m03 * c0) * invDet,
        ( m30 * s4 - m31 * s2 + m33 * s0) * invDet,
        (-m20 * s4 + m21 * s2 - m23 * s0) * invDet,

        (-m10 * c3 + m11 * c1 - m12 * c0) * invDet,
        ( m00 * c3 - m01 * c1 + m02 * c0) * invDet,
        (-m30 * s3 + m31 * s1 - m32 * s0) * invDet,
        ( m20 * s3 - m21 * s1 + m22 * s0) * invDet
    };
    // clang-format on
}

JUG_MATH_API constexpr void JUG_VECTORCALL Decompose(
    MATRIX      _mtx,
    VECTOR3*    _pOutScaleOrNull,
    QUATERNION* _pOutRotationOrNull,
    VECTOR3*    _pOutTranslationOrNull);

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Xform(
    const VECTOR4 _v,
    const MATRIX  _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 v = _v.ToM128();
#    if defined(JUG_MATH_SIMD_FMA)
        // FMA 면 체인이 명령이 가장 적다
        simd::M128 ret = simd::Mult(simd::Splat<0>(v), _mtx[0].ToM128());
        ret            = simd::MultAdd(simd::Splat<1>(v), _mtx[1].ToM128(), ret);
        ret            = simd::MultAdd(simd::Splat<2>(v), _mtx[2].ToM128(), ret);
        ret            = simd::MultAdd(simd::Splat<3>(v), _mtx[3].ToM128(), ret);
        return VECTOR4::MakeFromM128(ret);
#    else
        // FMA 가 없으면 명령 수가 같으니 덧셈 트리로 지연을 줄인다
        const simd::M128 xz = simd::MultAdd(simd::Splat<2>(v), _mtx[2].ToM128(), simd::Mult(simd::Splat<0>(v), _mtx[0].ToM128()));
        const simd::M128 yw = simd::MultAdd(simd::Splat<3>(v), _mtx[3].ToM128(), simd::Mult(simd::Splat<1>(v), _mtx[1].ToM128()));
        return VECTOR4::MakeFromM128(simd::Add(xz, yw));
#    endif
    }
#endif
    return VECTOR4 {
        _v[0] * _mtx[0][0] + _v[1] * _mtx[1][0] + _v[2] * _mtx[2][0] + _v[3] * _mtx[3][0],
        _v[0] * _mtx[0][1] + _v[1] * _mtx[1][1] + _v[2] * _mtx[2][1] + _v[3] * _mtx[3][1],
        _v[0] * _mtx[0][2] + _v[1] * _mtx[1][2] + _v[2] * _mtx[2][2] + _v[3] * _mtx[3][2],
        _v[0] * _mtx[0][3] + _v[1] * _mtx[1][3] + _v[2] * _mtx[2][3] + _v[3] * _mtx[3][3]
    };
}

// (x, y, z, 1) 을 변환하고 w 로 나눈다
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 JUG_VECTORCALL XformPoint(
    const VECTOR3 _point,
    const MATRIX  _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 p = _point.ToM128();
#    if defined(JUG_MATH_SIMD_FMA)
        simd::M128 ret = simd::MultAdd(simd::Splat<0>(p), _mtx[0].ToM128(), _mtx[3].ToM128());
        ret            = simd::MultAdd(simd::Splat<1>(p), _mtx[1].ToM128(), ret);
        ret            = simd::MultAdd(simd::Splat<2>(p), _mtx[2].ToM128(), ret);
#    else
        const simd::M128 xw  = simd::MultAdd(simd::Splat<0>(p), _mtx[0].ToM128(), _mtx[3].ToM128());
        const simd::M128 yz  = simd::MultAdd(simd::Splat<2>(p), _mtx[2].ToM128(), simd::Mult(simd::Splat<1>(p), _mtx[1].ToM128()));
        const simd::M128 ret = simd::Add(xw, yz);
#    endif
        return VECTOR3::MakeFromM128(simd::Div(ret, simd::Splat<3>(ret)));
    }
#endif
    const VECTOR4 p    = Xform(VECTOR4 { _point, 1.f }, _mtx);
    const float   invW = 1.f / p[3];
    return p.ToVector3() * invW;
}

// (x, y, z, 0) 을 변환한다 (이동 무시)
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 JUG_VECTORCALL XformVector(
    const VECTOR3 _v,
    const MATRIX  _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 v  = _v.ToM128();
        const simd::M128 xz = simd::MultAdd(simd::Splat<2>(v), _mtx[2].ToM128(), simd::Mult(simd::Splat<0>(v), _mtx[0].ToM128()));
        return VECTOR3::MakeFromM128(simd::MultAdd(simd::Splat<1>(v), _mtx[1].ToM128(), xz));
    }
#endif
    return Xform(VECTOR4 { _v, 0.f }, _mtx).ToVector3();
}

}   // namespace jug

#include "Matrix.inl"
