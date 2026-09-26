#pragma once
#include "Matrix.h"
#include "QuaternionReg.h"

namespace jug
{

// 연산 타입 (DirectXMath XMMATRIX). VECTOR4_REG 행 4개 = HVA 라 JUG_VECTORCALL 함수에 값으로 넘기면 레지스터 4개로 오간다.
// 인자가 여럿이면 첫 MATRIX_REG 만 값, 나머지는 const& 로 받는다 (DirectXMath FXMMATRIX / CXMMATRIX).
struct alignas(16) MATRIX_REG
{
    MATRIX_REG() = default;

    constexpr MATRIX_REG(
        const VECTOR4_REG _row0,
        const VECTOR4_REG _row1,
        const VECTOR4_REG _row2,
        const VECTOR4_REG _row3)
        : r { _row0, _row1, _row2, _row3 }
    {
    }

    explicit constexpr MATRIX_REG(
        const simd::M128 _row0,
        const simd::M128 _row1,
        const simd::M128 _row2,
        const simd::M128 _row3)
        : r { VECTOR4_REG { _row0 }, VECTOR4_REG { _row1 }, VECTOR4_REG { _row2 }, VECTOR4_REG { _row3 } }
    {
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeTranslation(
        const VECTOR3_REG _translation)
    {
        return MATRIX_REG { simd::Set(1.f, 0.f, 0.f, 0.f), simd::Set(0.f, 1.f, 0.f, 0.f), simd::Set(0.f, 0.f, 1.f, 0.f), simd::Blend<0b1000>(_translation.v, simd::SetAll(1.f)) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeScale(
        const VECTOR3_REG _scale)
    {
        const simd::M128 zero = simd::Zero();
        return MATRIX_REG { simd::Blend<0b0001>(zero, _scale.v), simd::Blend<0b0010>(zero, _scale.v), simd::Blend<0b0100>(zero, _scale.v), simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotationX(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);                                                                                            // s c s c
        const simd::M128 r1 = simd::Blend<0b0110>(simd::Zero(), sc);                                                                          // 0  c s 0
        const simd::M128 r2 = simd::Xor(simd::Blend<0b0110>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc)), simd::Set(0.f, -0.f, 0.f, 0.f));   // 0 -s c 0
        return MATRIX_REG { simd::Set(1.f, 0.f, 0.f, 0.f), r1, r2, simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotationY(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);                                                                                            // s c s c
        const simd::M128 r0 = simd::Xor(simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc)), simd::Set(0.f, 0.f, -0.f, 0.f));   // c 0 -s 0
        const simd::M128 r2 = simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc));                                              // s 0  c 0
        return MATRIX_REG { r0, simd::Set(0.f, 1.f, 0.f, 0.f), r2, simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotationZ(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);                                                                  // s c s c
        const simd::M128 r0 = simd::Blend<0b0011>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc));                     //  c s 0 0
        const simd::M128 r1 = simd::Xor(simd::Blend<0b0011>(simd::Zero(), sc), simd::Set(-0.f, 0.f, 0.f, 0.f));    // -s c 0 0
        return MATRIX_REG { r0, r1, simd::Set(0.f, 0.f, 1.f, 0.f), simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    // pitch(x) -> yaw(y) -> roll(z)
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotation(
        const VECTOR3_REG _pyrRad)
    {
        // A = (sp*sy, cp, sp*cy, 0), B = (cy, 0, -sy, 0) 로 두면 r0 = sr*A + cr*B, r1 = cr*A - sr*B, r2 = (cp*sy, -sp, cp*cy, 0)
        simd::M128 s;
        simd::M128 c;
        simd::SinCos(_pyrRad.v, s, c);
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
        return MATRIX_REG { r0, r1, r2, simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    // 로드리게스. 행 = n[i] * (1 - c) * n + (대각은 c, 나머지는 s * n 의 반대칭 행렬)
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotation(
        const VECTOR3_REG _axis,
        const float       _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const simd::M128 sc = simd::SinCos1(_rad);   // s c s c
        const simd::M128 n  = simd::ZeroW(_axis.v);
        const simd::M128 c  = simd::Splat<1>(sc);
        const simd::M128 tn = simd::Mult(simd::Sub(simd::SetAll(1.f), c), n);
        const simd::M128 sn = simd::Mult(simd::Splat<0>(sc), n);                                                                   // sx sy sz 0
        const simd::M128 e0 = simd::Blend<0b0001>(simd::Xor(simd::Shuffle<3, 2, 1, 3>(sn), simd::Set(0.f, 0.f, -0.f, 0.f)), c);   //   c  sz -sy 0
        const simd::M128 e1 = simd::Blend<0b0010>(simd::Xor(simd::Shuffle<2, 3, 0, 3>(sn), simd::Set(-0.f, 0.f, 0.f, 0.f)), c);   // -sz   c  sx 0
        const simd::M128 e2 = simd::Blend<0b0100>(simd::Xor(simd::Shuffle<1, 0, 3, 3>(sn), simd::Set(0.f, -0.f, 0.f, 0.f)), c);   //  sy -sx   c 0
        return MATRIX_REG { simd::MultAdd(simd::Splat<0>(n), tn, e0), simd::MultAdd(simd::Splat<1>(n), tn, e1), simd::MultAdd(simd::Splat<2>(n), tn, e2), simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    // 단위 사원수에서 1 - 2(yy + zz) = 2xx + 2ww - 1 이므로 행 i = q[i] * 2q + w * P_i - e_i
    //   P_0 = (2w, 2z, -2y), P_1 = (-2z, 2w, 2x), P_2 = (2y, -2x, 2w)
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotation(
        const QUATERNION_REG _q)
    {
        JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");

        const simd::M128 q   = _q.v;
        const simd::M128 q2  = simd::Add(q, q);
        const simd::M128 q2z = simd::ZeroW(q2);   // 2x 2y 2z 0
        const simd::M128 w   = simd::Splat<3>(q);

        const simd::M128 p0 = simd::Xor(simd::Shuffle2<3, 2, 1, 3>(q2, q2z), simd::Set(0.f, 0.f, -0.f, 0.f));   //  2w  2z -2y 0
        const simd::M128 p1 = simd::Xor(simd::Shuffle2<2, 3, 0, 3>(q2, q2z), simd::Set(-0.f, 0.f, 0.f, 0.f));   // -2z  2w  2x 0
        const simd::M128 p2 = simd::Xor(simd::Shuffle<1, 0, 3, 3>(q2), simd::Set(0.f, -0.f, 0.f, 0.f));         //  2y -2x  2w 2w

        const simd::M128 row0 = simd::MultAdd(simd::Splat<0>(q), q2z, simd::MultSub(w, p0, simd::Set(1.f, 0.f, 0.f, 0.f)));
        const simd::M128 row1 = simd::MultAdd(simd::Splat<1>(q), q2z, simd::MultSub(w, p1, simd::Set(0.f, 1.f, 0.f, 0.f)));
        const simd::M128 row2 = simd::ZeroW(simd::MultAdd(simd::Splat<2>(q), q2z, simd::MultSub(w, p2, simd::Set(0.f, 0.f, 1.f, 0.f))));
        return MATRIX_REG { row0, row1, row2, simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeRotationLookTo(
        const VECTOR3_REG _dir,
        const VECTOR3_REG _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");

        const simd::M128 f = _dir.v;
        const simd::M128 r = simd::Normalize3(simd::Cross3(_up.v, f));
        const simd::M128 u = simd::Cross3(f, r);
        return MATRIX_REG { simd::ZeroW(r), simd::ZeroW(u), simd::ZeroW(f), simd::Set(0.f, 0.f, 0.f, 1.f) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeSRT(
        const VECTOR3_REG _scale,
        const VECTOR3_REG _forward,
        const VECTOR3_REG _up,
        const VECTOR3_REG _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");

        const simd::M128 f = _forward.v;
        const simd::M128 r = simd::Normalize3(simd::Cross3(_up.v, f));
        const simd::M128 u = simd::Cross3(f, r);
        const simd::M128 s = _scale.v;
        return MATRIX_REG {
            simd::ZeroW(simd::Mult(r, simd::Splat<0>(s))),
            simd::ZeroW(simd::Mult(u, simd::Splat<1>(s))),
            simd::ZeroW(simd::Mult(f, simd::Splat<2>(s))),
            simd::Blend<0b1000>(_translation.v, simd::SetAll(1.f))
        };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeSRT(
        const VECTOR3_REG _scale,
        const VECTOR3_REG _pyrRad,
        const VECTOR3_REG _translation)
    {
        const MATRIX_REG rot = MakeRotation(_pyrRad);
        const simd::M128 s   = _scale.v;
        return MATRIX_REG {
            simd::Mult(rot.r[0].v, simd::Splat<0>(s)),
            simd::Mult(rot.r[1].v, simd::Splat<1>(s)),
            simd::Mult(rot.r[2].v, simd::Splat<2>(s)),
            simd::Blend<0b1000>(_translation.v, simd::SetAll(1.f))
        };
    }

    // 회전 행렬의 행마다 스케일을 곱하고 이동을 넣는다 (DirectXMath 는 스케일 행렬과 행렬 곱을 한다)
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeSRT(
        const VECTOR3_REG    _scale,
        const QUATERNION_REG _rotation,
        const VECTOR3_REG    _translation)
    {
        const MATRIX_REG rot = MakeRotation(_rotation);
        const simd::M128 s   = _scale.v;
        return MATRIX_REG {
            simd::Mult(rot.r[0].v, simd::Splat<0>(s)),
            simd::Mult(rot.r[1].v, simd::Splat<1>(s)),
            simd::Mult(rot.r[2].v, simd::Splat<2>(s)),
            simd::Blend<0b1000>(_translation.v, simd::SetAll(1.f))
        };
    }

    // 회전의 전치에 스케일의 역수를 곱하고, 이동은 -(t · 축) / s
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvSRT(
        const VECTOR3_REG _scale,
        const VECTOR3_REG _forward,
        const VECTOR3_REG _up,
        const VECTOR3_REG _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");

        const simd::M128 f      = _forward.v;
        const simd::M128 r      = simd::Normalize3(simd::Cross3(_up.v, f));
        const simd::M128 u      = simd::Cross3(f, r);
        const simd::M128 negT   = simd::Negate(_translation.v);
        simd::M128       row0   = simd::Blend<0b1000>(r, simd::Dot3V(r, negT));
        simd::M128       row1   = simd::Blend<0b1000>(u, simd::Dot3V(u, negT));
        simd::M128       row2   = simd::Blend<0b1000>(f, simd::Dot3V(f, negT));
        simd::M128       row3   = simd::Set(0.f, 0.f, 0.f, 1.f);
        const simd::M128 invS   = simd::Blend<0b1000>(simd::Div(simd::SetAll(1.f), _scale.v), simd::SetAll(1.f));
        simd::Transpose4(row0, row1, row2, row3);
        return MATRIX_REG { simd::Mult(row0, invS), simd::Mult(row1, invS), simd::Mult(row2, invS), simd::Mult(row3, invS) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvSRT(
        const VECTOR3_REG _scale,
        const VECTOR3_REG _pyrRad,
        const VECTOR3_REG _translation)
    {
        const MATRIX_REG rot  = MakeRotation(_pyrRad);
        const simd::M128 negT = simd::Negate(_translation.v);
        simd::M128       row0 = simd::Blend<0b1000>(rot.r[0].v, simd::Dot3V(rot.r[0].v, negT));
        simd::M128       row1 = simd::Blend<0b1000>(rot.r[1].v, simd::Dot3V(rot.r[1].v, negT));
        simd::M128       row2 = simd::Blend<0b1000>(rot.r[2].v, simd::Dot3V(rot.r[2].v, negT));
        simd::M128       row3 = simd::Set(0.f, 0.f, 0.f, 1.f);
        const simd::M128 invS = simd::Blend<0b1000>(simd::Div(simd::SetAll(1.f), _scale.v), simd::SetAll(1.f));
        simd::Transpose4(row0, row1, row2, row3);
        return MATRIX_REG { simd::Mult(row0, invS), simd::Mult(row1, invS), simd::Mult(row2, invS), simd::Mult(row3, invS) };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvSRT(
        const VECTOR3_REG    _scale,
        const QUATERNION_REG _rotation,
        const VECTOR3_REG    _translation)
    {
        const MATRIX_REG rot  = MakeRotation(_rotation);
        const simd::M128 negT = simd::Negate(_translation.v);
        simd::M128       row0 = simd::Blend<0b1000>(rot.r[0].v, simd::Dot3V(rot.r[0].v, negT));
        simd::M128       row1 = simd::Blend<0b1000>(rot.r[1].v, simd::Dot3V(rot.r[1].v, negT));
        simd::M128       row2 = simd::Blend<0b1000>(rot.r[2].v, simd::Dot3V(rot.r[2].v, negT));
        simd::M128       row3 = simd::Set(0.f, 0.f, 0.f, 1.f);
        const simd::M128 invS = simd::Blend<0b1000>(simd::Div(simd::SetAll(1.f), _scale.v), simd::SetAll(1.f));
        simd::Transpose4(row0, row1, row2, row3);
        return MATRIX_REG { simd::Mult(row0, invS), simd::Mult(row1, invS), simd::Mult(row2, invS), simd::Mult(row3, invS) };
    }

    // 축 r, u, f 와 -(eye · 축) 을 행으로 두고 전치한다
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeViewLookTo(
        const VECTOR3_REG _eye,
        const VECTOR3_REG _dir,
        const VECTOR3_REG _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");

        const simd::M128 f      = _dir.v;
        const simd::M128 r      = simd::Normalize3(simd::Cross3(_up.v, f));
        const simd::M128 u      = simd::Cross3(f, r);
        const simd::M128 negEye = simd::Negate(_eye.v);
        simd::M128       row0   = simd::Blend<0b1000>(r, simd::Dot3V(r, negEye));
        simd::M128       row1   = simd::Blend<0b1000>(u, simd::Dot3V(u, negEye));
        simd::M128       row2   = simd::Blend<0b1000>(f, simd::Dot3V(f, negEye));
        simd::M128       row3   = simd::Set(0.f, 0.f, 0.f, 1.f);
        simd::Transpose4(row0, row1, row2, row3);
        return MATRIX_REG { row0, row1, row2, row3 };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeViewLookTo(
        const VECTOR3_REG _eye,
        const VECTOR3_REG _pyrRad)
    {
        return MakeInvSRT(MathConstants<VECTOR3_REG>::kOne, _pyrRad, _eye);
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeViewLookTo(
        const VECTOR3_REG    _eye,
        const QUATERNION_REG _rotation)
    {
        return MakeInvSRT(MathConstants<VECTOR3_REG>::kOne, _rotation, _eye);
    }

    // 나눗셈 셋 (cot/aspect, cot, f/(f-n)) 을 벡터 나눗셈 한 번으로
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakePersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const simd::M128 sc   = simd::SinCos1(_fovYRad * 0.5f);   // s c s c
        const simd::M128 num  = simd::Blend<0b0100>(simd::Splat<1>(sc), simd::SetAll(_farZ));
        const simd::M128 den  = simd::Blend<0b0100>(simd::Mult(simd::Splat<0>(sc), simd::Set(_aspectRatio, 1.f, 1.f, 1.f)), simd::SetAll(_farZ - _nearZ));
        const simd::M128 q    = simd::Div(num, den);   // xs ys range -
        const simd::M128 zero = simd::Zero();
        return MATRIX_REG {
            simd::Blend<0b0001>(zero, q),
            simd::Blend<0b0010>(zero, q),
            simd::Blend<0b1000>(simd::Blend<0b0100>(zero, q), simd::SetAll(1.f)),
            simd::Blend<0b0100>(zero, simd::Mult(q, simd::SetAll(-_nearZ)))
        };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvPersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const simd::M128 sc   = simd::SinCos1(_fovYRad * 0.5f);   // s c s c
        const simd::M128 q    = simd::Div(simd::Shuffle<0, 0, 0, 0>(sc), simd::Shuffle<1, 1, 1, 1>(sc));
        const simd::M128 inv  = simd::Div(simd::Set(_nearZ - _farZ, 1.f, 0.f, 0.f), simd::Set(_farZ * _nearZ, _nearZ, 1.f, 1.f));   // -d/(fn) 1/n
        const simd::M128 zero = simd::Zero();
        return MATRIX_REG {
            simd::Blend<0b0001>(zero, simd::Mult(q, simd::SetAll(_aspectRatio))),
            simd::Blend<0b0010>(zero, q),
            simd::Blend<0b1000>(zero, simd::Splat<0>(inv)),
            simd::Blend<0b1100>(zero, simd::Shuffle<0, 0, 3, 1>(simd::Blend<0b1000>(inv, simd::SetAll(1.f))))
        };
    }

    // 역수 셋을 벡터 나눗셈 한 번으로
    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeOrtho(
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

        const simd::M128 rcp  = simd::Div(simd::SetAll(1.f), simd::Set(_right - _left, _top - _bottom, _farZ - _nearZ, 1.f));   // invW invH range 1
        const simd::M128 two  = simd::Add(rcp, rcp);
        const simd::M128 zero = simd::Zero();
        const simd::M128 r3   = simd::Blend<0b1000>(simd::Negate(simd::Mult(simd::Set(_left + _right, _top + _bottom, _nearZ, 0.f), rcp)), simd::SetAll(1.f));
        return MATRIX_REG { simd::Blend<0b0001>(zero, two), simd::Blend<0b0010>(zero, two), simd::Blend<0b0100>(zero, rcp), r3 };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvOrtho(
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

        const simd::M128 half = simd::Mult(simd::Set(_right - _left, _top - _bottom, _right + _left, _top + _bottom), simd::SetAll(0.5f));   // w/2 h/2 cx cy
        const simd::M128 zero = simd::Zero();
        return MATRIX_REG {
            simd::Blend<0b0001>(zero, half),
            simd::Blend<0b0010>(zero, half),
            simd::Set(0.f, 0.f, _farZ - _nearZ, 0.f),
            simd::Shuffle2<2, 3, 0, 1>(half, simd::Set(_nearZ, 1.f, 0.f, 0.f))
        };
    }

    [[nodiscard]] static MATRIX_REG JUG_VECTORCALL MakeInvOrtho(
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

    [[nodiscard]] VECTOR4_REG& operator[](
        const size_t _row)
    {
        return r[_row];
    }

    [[nodiscard]] const VECTOR4_REG& operator[](
        const size_t _row) const
    {
        return r[_row];
    }

    MATRIX_REG& operator+=(const MATRIX_REG& _other);
    MATRIX_REG& operator-=(const MATRIX_REG& _other);
    MATRIX_REG& operator*=(const MATRIX_REG& _other);
    MATRIX_REG& operator*=(float _scalar);
    MATRIX_REG& operator/=(float _scalar);

    const static MATRIX_REG kIdentity;
    const static MATRIX_REG kZero;

    VECTOR4_REG r[4];
};

static_assert(sizeof(MATRIX_REG) == 64 && alignof(MATRIX_REG) == 16, "MATRIX_REG must be four 16-byte registers");

// =======================================================
//  Constants
// =======================================================

inline constexpr MATRIX_REG MATRIX_REG::kIdentity = {
    VECTOR4_REG { 1.f, 0.f, 0.f, 0.f },
    VECTOR4_REG { 0.f, 1.f, 0.f, 0.f },
    VECTOR4_REG { 0.f, 0.f, 1.f, 0.f },
    VECTOR4_REG { 0.f, 0.f, 0.f, 1.f }
};

inline constexpr MATRIX_REG MATRIX_REG::kZero = {
    VECTOR4_REG { 0.f },
    VECTOR4_REG { 0.f },
    VECTOR4_REG { 0.f },
    VECTOR4_REG { 0.f }
};

template<>
struct MathConstants<MATRIX_REG>
{
    constexpr static MATRIX_REG kIdentity = MATRIX_REG::kIdentity;
    constexpr static MATRIX_REG kZero     = MATRIX_REG::kZero;
};

// =======================================================
//  Load / Store
// =======================================================

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL Load(
    const MATRIX& _mtx)
{
    const float* p = _mtx.GetPtr();
    return MATRIX_REG { simd::LoadAligned(p), simd::LoadAligned(p + 4), simd::LoadAligned(p + 8), simd::LoadAligned(p + 12) };
}

[[nodiscard]] inline MATRIX JUG_VECTORCALL Store(
    const MATRIX_REG _mtx)
{
    MATRIX ret;
    float* p = ret.GetPtr();
    simd::StoreAligned(p, _mtx.r[0].v);
    simd::StoreAligned(p + 4, _mtx.r[1].v);
    simd::StoreAligned(p + 8, _mtx.r[2].v);
    simd::StoreAligned(p + 12, _mtx.r[3].v);
    return ret;
}

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator+(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y)
{
    return MATRIX_REG { _x.r[0] + _y.r[0], _x.r[1] + _y.r[1], _x.r[2] + _y.r[2], _x.r[3] + _y.r[3] };
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator-(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y)
{
    return MATRIX_REG { _x.r[0] - _y.r[0], _x.r[1] - _y.r[1], _x.r[2] - _y.r[2], _x.r[3] - _y.r[3] };
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator*(
    const MATRIX_REG _mtx,
    const float      _scalar)
{
    const simd::M128 s = simd::SetAll(_scalar);
    return MATRIX_REG { simd::Mult(_mtx.r[0].v, s), simd::Mult(_mtx.r[1].v, s), simd::Mult(_mtx.r[2].v, s), simd::Mult(_mtx.r[3].v, s) };
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator*(
    const float      _scalar,
    const MATRIX_REG _mtx)
{
    return _mtx * _scalar;
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator/(
    const MATRIX_REG _mtx,
    const float      _scalar)
{
    const simd::M128 s = simd::SetAll(_scalar);
    return MATRIX_REG { simd::Div(_mtx.r[0].v, s), simd::Div(_mtx.r[1].v, s), simd::Div(_mtx.r[2].v, s), simd::Div(_mtx.r[3].v, s) };
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL operator*(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y)
{
#if defined(JUG_MATH_SIMD_FMA)
    // 두 행씩 256비트로. FMA 가 있으면 곱셈 1 + FMA 3 체인이 명령이 가장 적다.
    const __m256 x01 = _mm256_insertf128_ps(_mm256_castps128_ps256(_x.r[0].v), _x.r[1].v, 1);
    const __m256 x23 = _mm256_insertf128_ps(_mm256_castps128_ps256(_x.r[2].v), _x.r[3].v, 1);
    const __m256 b0  = _mm256_insertf128_ps(_mm256_castps128_ps256(_y.r[0].v), _y.r[0].v, 1);
    const __m256 b1  = _mm256_insertf128_ps(_mm256_castps128_ps256(_y.r[1].v), _y.r[1].v, 1);
    const __m256 b2  = _mm256_insertf128_ps(_mm256_castps128_ps256(_y.r[2].v), _y.r[2].v, 1);
    const __m256 b3  = _mm256_insertf128_ps(_mm256_castps128_ps256(_y.r[3].v), _y.r[3].v, 1);

    __m256 r01 = _mm256_mul_ps(_mm256_shuffle_ps(x01, x01, 0x00), b0);
    __m256 r23 = _mm256_mul_ps(_mm256_shuffle_ps(x23, x23, 0x00), b0);
    r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0x55), b1, r01);
    r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0x55), b1, r23);
    r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xaa), b2, r01);
    r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xaa), b2, r23);
    r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xff), b3, r01);
    r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xff), b3, r23);
    return MATRIX_REG { _mm256_castps256_ps128(r01), _mm256_extractf128_ps(r01, 1), _mm256_castps256_ps128(r23), _mm256_extractf128_ps(r23, 1) };
#else
    // (x*b0 + z*b2) + (y*b1 + w*b3) 트리
    const simd::M128 b0 = _y.r[0].v;
    const simd::M128 b1 = _y.r[1].v;
    const simd::M128 b2 = _y.r[2].v;
    const simd::M128 b3 = _y.r[3].v;

    const simd::M128 a0   = _x.r[0].v;
    const simd::M128 row0 = simd::Add(simd::MultAdd(simd::Splat<2>(a0), b2, simd::Mult(simd::Splat<0>(a0), b0)), simd::MultAdd(simd::Splat<3>(a0), b3, simd::Mult(simd::Splat<1>(a0), b1)));
    const simd::M128 a1   = _x.r[1].v;
    const simd::M128 row1 = simd::Add(simd::MultAdd(simd::Splat<2>(a1), b2, simd::Mult(simd::Splat<0>(a1), b0)), simd::MultAdd(simd::Splat<3>(a1), b3, simd::Mult(simd::Splat<1>(a1), b1)));
    const simd::M128 a2   = _x.r[2].v;
    const simd::M128 row2 = simd::Add(simd::MultAdd(simd::Splat<2>(a2), b2, simd::Mult(simd::Splat<0>(a2), b0)), simd::MultAdd(simd::Splat<3>(a2), b3, simd::Mult(simd::Splat<1>(a2), b1)));
    const simd::M128 a3   = _x.r[3].v;
    const simd::M128 row3 = simd::Add(simd::MultAdd(simd::Splat<2>(a3), b2, simd::Mult(simd::Splat<0>(a3), b0)), simd::MultAdd(simd::Splat<3>(a3), b3, simd::Mult(simd::Splat<1>(a3), b1)));
    return MATRIX_REG { row0, row1, row2, row3 };
#endif
}

[[nodiscard]] inline bool JUG_VECTORCALL operator==(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y)
{
    return _x.r[0] == _y.r[0] && _x.r[1] == _y.r[1] && _x.r[2] == _y.r[2] && _x.r[3] == _y.r[3];
}

[[nodiscard]] inline bool JUG_VECTORCALL operator!=(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y)
{
    return !(_x == _y);
}

inline MATRIX_REG& MATRIX_REG::operator+=(
    const MATRIX_REG& _other)
{
    *this = *this + _other;
    return *this;
}

inline MATRIX_REG& MATRIX_REG::operator-=(
    const MATRIX_REG& _other)
{
    *this = *this - _other;
    return *this;
}

inline MATRIX_REG& MATRIX_REG::operator*=(
    const MATRIX_REG& _other)
{
    *this = *this * _other;
    return *this;
}

inline MATRIX_REG& MATRIX_REG::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

inline MATRIX_REG& MATRIX_REG::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

// =======================================================
//  Method
// =======================================================

[[nodiscard]] inline bool JUG_VECTORCALL IsEqualApprox(
    const MATRIX_REG  _x,
    const MATRIX_REG& _y,
    const float       _epsilon = kEpsilon)
{
    return IsEqualApprox(_x.r[0], _y.r[0], _epsilon) && IsEqualApprox(_x.r[1], _y.r[1], _epsilon) && IsEqualApprox(_x.r[2], _y.r[2], _epsilon) && IsEqualApprox(_x.r[3], _y.r[3], _epsilon);
}

[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL Transpose(
    const MATRIX_REG _mtx)
{
    simd::M128 r0 = _mtx.r[0].v;
    simd::M128 r1 = _mtx.r[1].v;
    simd::M128 r2 = _mtx.r[2].v;
    simd::M128 r3 = _mtx.r[3].v;
    simd::Transpose4(r0, r1, r2, r3);
    return MATRIX_REG { r0, r1, r2, r3 };
}

// DirectXMath XMMatrixDeterminant 와 같은 식: 아래 두 행의 2x2 소행렬식 6개를 두 번에 나눠 구한다
[[nodiscard]] inline float JUG_VECTORCALL Determinant(
    const MATRIX_REG _mtx)
{
    const simd::M128 r0 = _mtx.r[0].v;
    const simd::M128 r1 = _mtx.r[1].v;
    const simd::M128 r2 = _mtx.r[2].v;
    const simd::M128 r3 = _mtx.r[3].v;

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
    return simd::GetX(simd::Dot4V(s, ret));
}

// 2x2 블록 방법 (Eric Zhang, "Fast 4x4 Matrix Inverse with SSE SIMD"). 역행렬이 없으면 (행렬식이 정확히 0) 영행렬.
//     | A B |    2x2 블록은 레지스터 하나에 (m00 m01 m10 m11) 로 담는다. X# 는 수반 행렬.
// M = | C D |    M^-1 = 1/|M| * | X Y |  의 각 블록을 수반 행렬 꼴로 구한 뒤 마지막 셔플에서 되돌린다.
//                               | Z W |
[[nodiscard]] inline MATRIX_REG JUG_VECTORCALL Inverse(
    const MATRIX_REG _mtx,
    float*           _outDetOrNull = nullptr)
{
    const simd::M128 r0 = _mtx.r[0].v;
    const simd::M128 r1 = _mtx.r[1].v;
    const simd::M128 r2 = _mtx.r[2].v;
    const simd::M128 r3 = _mtx.r[3].v;

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
        return MATRIX_REG::kZero;
    }

    // (1/|M|, -1/|M|, -1/|M|, 1/|M|) 를 곱하고, 수반 행렬 셔플과 행 배치 셔플을 한 번에
    const simd::M128 rDetM = simd::Div(simd::Set(1.f, -1.f, -1.f, 1.f), detM);
    x                      = simd::Mult(x, rDetM);
    y                      = simd::Mult(y, rDetM);
    z                      = simd::Mult(z, rDetM);
    w                      = simd::Mult(w, rDetM);
    return MATRIX_REG { simd::Shuffle2<3, 1, 3, 1>(x, y), simd::Shuffle2<2, 0, 2, 0>(x, y), simd::Shuffle2<3, 1, 3, 1>(z, w), simd::Shuffle2<2, 0, 2, 0>(z, w) };
}

[[nodiscard]] inline VECTOR4_REG JUG_VECTORCALL Xform(
    const VECTOR4_REG _v,
    const MATRIX_REG& _mtx)
{
    const simd::M128 v = _v.v;
#if defined(JUG_MATH_SIMD_FMA)
    simd::M128 ret = simd::Mult(simd::Splat<0>(v), _mtx.r[0].v);
    ret            = simd::MultAdd(simd::Splat<1>(v), _mtx.r[1].v, ret);
    ret            = simd::MultAdd(simd::Splat<2>(v), _mtx.r[2].v, ret);
    ret            = simd::MultAdd(simd::Splat<3>(v), _mtx.r[3].v, ret);
    return VECTOR4_REG { ret };
#else
    const simd::M128 xz = simd::MultAdd(simd::Splat<2>(v), _mtx.r[2].v, simd::Mult(simd::Splat<0>(v), _mtx.r[0].v));
    const simd::M128 yw = simd::MultAdd(simd::Splat<3>(v), _mtx.r[3].v, simd::Mult(simd::Splat<1>(v), _mtx.r[1].v));
    return VECTOR4_REG { simd::Add(xz, yw) };
#endif
}

// (x, y, z, 1) 을 변환하고 w 로 나눈다
[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL XformPoint(
    const VECTOR3_REG _point,
    const MATRIX_REG& _mtx)
{
    const simd::M128 p = _point.v;
#if defined(JUG_MATH_SIMD_FMA)
    simd::M128 ret = simd::MultAdd(simd::Splat<0>(p), _mtx.r[0].v, _mtx.r[3].v);
    ret            = simd::MultAdd(simd::Splat<1>(p), _mtx.r[1].v, ret);
    ret            = simd::MultAdd(simd::Splat<2>(p), _mtx.r[2].v, ret);
#else
    const simd::M128 xw  = simd::MultAdd(simd::Splat<0>(p), _mtx.r[0].v, _mtx.r[3].v);
    const simd::M128 yz  = simd::MultAdd(simd::Splat<2>(p), _mtx.r[2].v, simd::Mult(simd::Splat<1>(p), _mtx.r[1].v));
    const simd::M128 ret = simd::Add(xw, yz);
#endif
    return VECTOR3_REG { simd::Div(ret, simd::Splat<3>(ret)) };
}

// (x, y, z, 0) 을 변환한다 (이동 무시)
[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL XformVector(
    const VECTOR3_REG _v,
    const MATRIX_REG& _mtx)
{
    const simd::M128 v  = _v.v;
    const simd::M128 xz = simd::MultAdd(simd::Splat<2>(v), _mtx.r[2].v, simd::Mult(simd::Splat<0>(v), _mtx.r[0].v));
    return VECTOR3_REG { simd::MultAdd(simd::Splat<1>(v), _mtx.r[1].v, xz) };
}

inline void JUG_VECTORCALL Decompose(
    const MATRIX_REG _mtx,
    VECTOR3_REG*     _pOutScaleOrNull,
    QUATERNION_REG*  _pOutRotationOrNull,
    VECTOR3_REG*     _pOutTranslationOrNull)
{
    if (_pOutRotationOrNull || _pOutScaleOrNull)
    {
        // 행 길이 셋을 전치해서 한 번에: (|r0|, |r1|, |r2|, 0)
        const simd::M128 r0 = _mtx.r[0].v;
        const simd::M128 r1 = _mtx.r[1].v;
        const simd::M128 r2 = _mtx.r[2].v;
        simd::M128       x  = simd::Mult(r0, r0);
        simd::M128       y  = simd::Mult(r1, r1);
        simd::M128       z  = simd::Mult(r2, r2);
        simd::M128       w  = simd::Zero();
        simd::Transpose4(x, y, z, w);
        const simd::M128 scale = simd::Sqrt(simd::Add(simd::Add(x, y), simd::Add(z, w)));
        if (_pOutScaleOrNull)
        {
            *_pOutScaleOrNull = VECTOR3_REG { scale };
        }

        if (_pOutRotationOrNull)
        {
            const simd::M128 inv = simd::Div(simd::SetAll(1.f), scale);
            *_pOutRotationOrNull = QUATERNION_REG::MakeFromMatrix(MATRIX_REG {
                simd::Mult(r0, simd::Splat<0>(inv)),
                simd::Mult(r1, simd::Splat<1>(inv)),
                simd::Mult(r2, simd::Splat<2>(inv)),
                simd::Set(0.f, 0.f, 0.f, 1.f) });
        }
    }

    if (_pOutTranslationOrNull)
    {
        *_pOutTranslationOrNull = VECTOR3_REG { _mtx.r[3].v };
    }
}

// Shepperd. 분기 없는 SIMD 판 (DirectXMath 방식) 은 셔플이 많아 이 스칼라 분기판보다 느렸다.
inline QUATERNION_REG JUG_VECTORCALL QUATERNION_REG::MakeFromMatrix(
    const MATRIX_REG& _mtx)
{
    const float m00 = _mtx.r[0].GetX(), m01 = _mtx.r[0].GetY(), m02 = _mtx.r[0].GetZ();
    const float m10 = _mtx.r[1].GetX(), m11 = _mtx.r[1].GetY(), m12 = _mtx.r[1].GetZ();
    const float m20 = _mtx.r[2].GetX(), m21 = _mtx.r[2].GetY(), m22 = _mtx.r[2].GetZ();

    const float trace = m00 + m11 + m22;
    if (trace > 0.f)
    {
        const float s    = Sqrt(trace + 1.f) * 2.f;   // 4w
        const float invS = 1.f / s;
        return QUATERNION_REG { (m12 - m21) * invS, (m20 - m02) * invS, (m01 - m10) * invS, s * 0.25f };
    }

    if (m00 > m11 && m00 > m22)
    {
        const float s    = Sqrt(1.f + m00 - m11 - m22) * 2.f;   // 4x
        const float invS = 1.f / s;
        return QUATERNION_REG { s * 0.25f, (m01 + m10) * invS, (m02 + m20) * invS, (m12 - m21) * invS };
    }

    if (m11 > m22)
    {
        const float s    = Sqrt(1.f + m11 - m00 - m22) * 2.f;   // 4y
        const float invS = 1.f / s;
        return QUATERNION_REG { (m01 + m10) * invS, s * 0.25f, (m12 + m21) * invS, (m20 - m02) * invS };
    }

    const float s    = Sqrt(1.f + m22 - m00 - m11) * 2.f;   // 4z
    const float invS = 1.f / s;
    return QUATERNION_REG { (m02 + m20) * invS, (m12 + m21) * invS, s * 0.25f, (m01 - m10) * invS };
}

}   // namespace jug
