#pragma once
#include "Vector.h"

namespace jug
{

struct QUAT;

namespace matrix_detail
{
    // 행 벡터 _a 에 행렬 _m 을 곱한다
    [[nodiscard]] JUG_MATH_API constexpr simd::M128 JUG_VECTORCALL MultRow(
        const simd::M128    _a,
        const simd::M128X4& _m)
    {
#if defined(JUG_SIMD_FMA)
        simd::M128 ret = simd::Mult(simd::Splat<0>(_a), _m.r[0]);
        ret            = simd::MultAdd(simd::Splat<1>(_a), _m.r[1], ret);
        ret            = simd::MultAdd(simd::Splat<2>(_a), _m.r[2], ret);
        return simd::MultAdd(simd::Splat<3>(_a), _m.r[3], ret);
#else
        const simd::M128 xz = simd::MultAdd(simd::Splat<2>(_a), _m.r[2], simd::Mult(simd::Splat<0>(_a), _m.r[0]));
        const simd::M128 yw = simd::MultAdd(simd::Splat<3>(_a), _m.r[3], simd::Mult(simd::Splat<1>(_a), _m.r[1]));
        return simd::Add(xz, yw);
#endif
    }

    // pitch(x) -> yaw(y) -> roll(z)
    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL RotationPYR(
        const simd::M128 _pyrRad)
    {
        const auto [s, c]     = simd::SinCos(_pyrRad);
        const simd::M128 spcp = simd::Shuffle2<0, 0, 0, 0>(s, c);
        const simd::M128 sycy = simd::Shuffle2<1, 1, 1, 1>(s, c);
        const simd::M128 t    = simd::Blend<0b1010>(simd::Shuffle<0, 0, 2, 2>(sycy), simd::Set(0.f, 1.f, 0.f, 0.f));
        const simd::M128 a    = simd::Mult(simd::Shuffle<0, 2, 0, 2>(spcp), t);
        const simd::M128 b    = simd::Mult(simd::Shuffle<2, 0, 0, 0>(sycy), simd::Set(1.f, 0.f, -1.f, 0.f));
        const simd::M128 sr   = simd::Splat<2>(s);
        const simd::M128 cr   = simd::Splat<2>(c);
        return simd::M128X4 {
            {
             simd::MultAdd(sr, a, simd::Mult(cr, b)),
             simd::NegMultAdd(sr, b, simd::Mult(cr, a)),
             simd::Mult(simd::Mult(simd::Shuffle<2, 0, 2, 2>(spcp), simd::Set(1.f, -1.f, 1.f, 0.f)), t),
             simd::Set(0.f, 0.f, 0.f, 1.f),
             }
        };
    }

    // 단위 사원수
    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL RotationQuat(
        const simd::M128 _q)
    {
        const simd::M128 q2  = simd::Add(_q, _q);
        const simd::M128 q2z = simd::ZeroW(q2);
        const simd::M128 w   = simd::Splat<3>(_q);
        const simd::M128 p0  = simd::Xor(simd::Shuffle2<3, 2, 1, 3>(q2, q2z), simd::Set(0.f, 0.f, -0.f, 0.f));
        const simd::M128 p1  = simd::Xor(simd::Shuffle2<2, 3, 0, 3>(q2, q2z), simd::Set(-0.f, 0.f, 0.f, 0.f));
        const simd::M128 p2  = simd::Xor(simd::Shuffle<1, 0, 3, 3>(q2), simd::Set(0.f, -0.f, 0.f, 0.f));
        return simd::M128X4 {
            {
             simd::MultAdd(simd::Splat<0>(_q), q2z, simd::MultSub(w, p0, simd::Set(1.f, 0.f, 0.f, 0.f))),
             simd::MultAdd(simd::Splat<1>(_q), q2z, simd::MultSub(w, p1, simd::Set(0.f, 1.f, 0.f, 0.f))),
             simd::ZeroW(simd::MultAdd(simd::Splat<2>(_q), q2z, simd::MultSub(w, p2, simd::Set(0.f, 0.f, 1.f, 0.f)))),
             simd::Set(0.f, 0.f, 0.f, 1.f),
             }
        };
    }

    // 행 = (right, up, forward, (0, 0, 0, 1)). _dir 의 w 는 0 이어야 한다
    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL RotationLookTo(
        const simd::M128 _dir,
        const simd::M128 _up)
    {
        const simd::M128 r = simd::Normalize3(simd::Cross3(_up, _dir));
        const simd::M128 u = simd::Cross3(_dir, r);
        return simd::M128X4 {
            { simd::ZeroW(r), simd::ZeroW(u), _dir, simd::Set(0.f, 0.f, 0.f, 1.f) }
        };
    }

    // 회전 _rot 뒤에 이동 _t 를 한 변환의 역행렬
    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL InvRT(
        const simd::M128X4 _rot,
        const simd::M128   _t)
    {
        const simd::M128 negT = simd::Negate(_t);
        simd::M128       r0   = simd::Blend<0b1000>(_rot.r[0], simd::Dot3V(_rot.r[0], negT));
        simd::M128       r1   = simd::Blend<0b1000>(_rot.r[1], simd::Dot3V(_rot.r[1], negT));
        simd::M128       r2   = simd::Blend<0b1000>(_rot.r[2], simd::Dot3V(_rot.r[2], negT));
        simd::M128       r3   = simd::Set(0.f, 0.f, 0.f, 1.f);
        simd::Transpose4(r0, r1, r2, r3);
        return simd::M128X4 {
            { r0, r1, r2, r3 }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL InvSRT(
        const simd::M128X4 _rot,
        const simd::M128   _scale,
        const simd::M128   _t)
    {
        const simd::M128X4 m    = InvRT(_rot, _t);
        const simd::M128   invS = simd::Div(simd::SetAll(1.f), simd::Blend<0b1000>(_scale, simd::SetAll(1.f)));
        return simd::M128X4 {
            { simd::Mult(m.r[0], invS), simd::Mult(m.r[1], invS), simd::Mult(m.r[2], invS), simd::Mult(m.r[3], invS) }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL ScaleTranslate(
        const simd::M128X4 _rot,
        const simd::M128   _scale,
        const simd::M128   _t)
    {
        return simd::M128X4 {
            {
             simd::Mult(_rot.r[0], simd::Splat<0>(_scale)),
             simd::Mult(_rot.r[1], simd::Splat<1>(_scale)),
             simd::Mult(_rot.r[2], simd::Splat<2>(_scale)),
             simd::Blend<0b1000>(_t, simd::SetAll(1.f)),
             }
        };
    }
}   // namespace matrix_detail

// 1. 4x4 행렬
// 2. 행 우선
// 3. 행 벡터
// 4. 왼손 좌표계

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

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX JUG_VECTORCALL FromM128X4(
        const simd::M128X4 _m)
    {
        if (std::is_constant_evaluated())
        {
            return simd::NoInlineConstruct<MATRIX>(VECTOR4::FromM128(_m.r[0]), VECTOR4::FromM128(_m.r[1]), VECTOR4::FromM128(_m.r[2]), VECTOR4::FromM128(_m.r[3]));
        }
        else
        {
            return std::bit_cast<MATRIX>(_m);
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr simd::M128X4 JUG_VECTORCALL ToM128X4() const
    {
        return simd::M128X4 {
            { simd::LoadAligned(r[0].GetPtr()), simd::LoadAligned(r[1].GetPtr()), simd::LoadAligned(r[2].GetPtr()), simd::LoadAligned(r[3].GetPtr()) }
        };
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeTranslation(
        const VECTOR3 _translation)
    {
        return FromM128X4(simd::M128X4 {
            {
             simd::Set(1.f, 0.f, 0.f, 0.f),
             simd::Set(0.f, 1.f, 0.f, 0.f),
             simd::Set(0.f, 0.f, 1.f, 0.f),
             simd::Blend<0b1000>(_translation.ToM128(), simd::SetAll(1.f)),
             }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeScale(
        const VECTOR3 _scale)
    {
        const simd::M128 zero = simd::Zero();
        const simd::M128 s    = _scale.ToM128();
        return FromM128X4(simd::M128X4 {
            { simd::Blend<0b0001>(zero, s), simd::Blend<0b0010>(zero, s), simd::Blend<0b0100>(zero, s), simd::Set(0.f, 0.f, 0.f, 1.f) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationX(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);
        const simd::M128 r1 = simd::Blend<0b0110>(simd::Zero(), sc);
        const simd::M128 r2 = simd::Xor(simd::Blend<0b0110>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc)), simd::Set(0.f, -0.f, 0.f, 0.f));
        return FromM128X4(simd::M128X4 {
            { simd::Set(1.f, 0.f, 0.f, 0.f), r1, r2, simd::Set(0.f, 0.f, 0.f, 1.f) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationY(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);
        const simd::M128 r0 = simd::Xor(simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc)), simd::Set(0.f, 0.f, -0.f, 0.f));
        const simd::M128 r2 = simd::Blend<0b0101>(simd::Zero(), simd::Shuffle<0, 0, 1, 0>(sc));
        return FromM128X4(simd::M128X4 {
            { r0, simd::Set(0.f, 1.f, 0.f, 0.f), r2, simd::Set(0.f, 0.f, 0.f, 1.f) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationZ(
        const float _rad)
    {
        const simd::M128 sc = simd::SinCos1(_rad);
        const simd::M128 r0 = simd::Blend<0b0011>(simd::Zero(), simd::Shuffle<1, 0, 0, 0>(sc));
        const simd::M128 r1 = simd::Xor(simd::Blend<0b0011>(simd::Zero(), sc), simd::Set(-0.f, 0.f, 0.f, 0.f));
        return FromM128X4(simd::M128X4 {
            { r0, r1, simd::Set(0.f, 0.f, 1.f, 0.f), simd::Set(0.f, 0.f, 0.f, 1.f) }
        });
    }

    // pitch(x) -> yaw(y) -> roll(z)
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const VECTOR3 _pyrRad)
    {
        return FromM128X4(matrix_detail::RotationPYR(_pyrRad.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const simd::M128 sc = simd::SinCos1(_rad);
        const simd::M128 n  = _axis.ToM128();
        const simd::M128 c  = simd::Splat<1>(sc);
        const simd::M128 tn = simd::Mult(simd::Sub(simd::SetAll(1.f), c), n);
        const simd::M128 sn = simd::Mult(simd::Splat<0>(sc), n);
        const simd::M128 e0 = simd::Blend<0b0001>(simd::Xor(simd::Shuffle<3, 2, 1, 3>(sn), simd::Set(0.f, 0.f, -0.f, 0.f)), c);
        const simd::M128 e1 = simd::Blend<0b0010>(simd::Xor(simd::Shuffle<2, 3, 0, 3>(sn), simd::Set(-0.f, 0.f, 0.f, 0.f)), c);
        const simd::M128 e2 = simd::Blend<0b0100>(simd::Xor(simd::Shuffle<1, 0, 3, 3>(sn), simd::Set(0.f, -0.f, 0.f, 0.f)), c);
        return FromM128X4(simd::M128X4 {
            {
             simd::MultAdd(simd::Splat<0>(n), tn, e0),
             simd::MultAdd(simd::Splat<1>(n), tn, e1),
             simd::MultAdd(simd::Splat<2>(n), tn, e2),
             simd::Set(0.f, 0.f, 0.f, 1.f),
             }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const QUAT& _q);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationLookTo(
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_dir, _up), "Forward and up dirs cannot be parallel");

        return FromM128X4(matrix_detail::RotationLookTo(_dir.ToM128(), _up.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        return FromM128X4(matrix_detail::ScaleTranslate(matrix_detail::RotationLookTo(_forward.ToM128(), _up.ToM128()), _scale.ToM128(), _translation.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        return FromM128X4(matrix_detail::ScaleTranslate(matrix_detail::RotationPYR(_pyrRad.ToM128()), _scale.ToM128(), _translation.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        VECTOR3     _scale,
        const QUAT& _rotation,
        VECTOR3     _translation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        return FromM128X4(matrix_detail::InvSRT(matrix_detail::RotationLookTo(_forward.ToM128(), _up.ToM128()), _scale.ToM128(), _translation.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

        return FromM128X4(matrix_detail::InvSRT(matrix_detail::RotationPYR(_pyrRad.ToM128()), _scale.ToM128(), _translation.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvSRT(
        VECTOR3 _scale,
        QUAT    _rotation,
        VECTOR3 _translation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");

        return FromM128X4(matrix_detail::InvRT(matrix_detail::RotationLookTo(_dir.ToM128(), _up.ToM128()), _eye.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _pyrRad)
    {
        return FromM128X4(matrix_detail::InvRT(matrix_detail::RotationPYR(_pyrRad.ToM128()), _eye.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        VECTOR3 _eye,
        QUAT    _rotation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakePersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const simd::M128 sc   = simd::SinCos1(_fovYRad * 0.5f);
        const simd::M128 num  = simd::Blend<0b0100>(simd::Splat<1>(sc), simd::SetAll(_farZ));
        const simd::M128 den  = simd::Blend<0b0100>(simd::Mult(simd::Splat<0>(sc), simd::Set(_aspectRatio, 1.f, 1.f, 1.f)), simd::SetAll(_farZ - _nearZ));
        const simd::M128 q    = simd::Div(num, den);
        const simd::M128 zero = simd::Zero();
        return FromM128X4(simd::M128X4 {
            {
             simd::Blend<0b0001>(zero, q),
             simd::Blend<0b0010>(zero, q),
             simd::Blend<0b1000>(simd::Blend<0b0100>(zero, q), simd::SetAll(1.f)),
             simd::Blend<0b0100>(zero, simd::Mult(q, simd::SetAll(-_nearZ))),
             }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvPersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const simd::M128 sc   = simd::SinCos1(_fovYRad * 0.5f);
        const simd::M128 q    = simd::Div(simd::Shuffle<0, 0, 0, 0>(sc), simd::Shuffle<1, 1, 1, 1>(sc));
        const simd::M128 inv  = simd::Div(simd::Set(_nearZ - _farZ, 1.f, 0.f, 0.f), simd::Set(_farZ * _nearZ, _nearZ, 1.f, 1.f));
        const simd::M128 zero = simd::Zero();
        return FromM128X4(simd::M128X4 {
            {
             simd::Blend<0b0001>(zero, simd::Mult(q, simd::SetAll(_aspectRatio))),
             simd::Blend<0b0010>(zero, q),
             simd::Blend<0b1000>(zero, simd::Splat<0>(inv)),
             simd::Blend<0b1100>(zero, simd::Shuffle<0, 0, 3, 1>(simd::Blend<0b1000>(inv, simd::SetAll(1.f)))),
             }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeOrtho(
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

        const simd::M128 rcp  = simd::Div(simd::SetAll(1.f), simd::Set(_right - _left, _top - _bottom, _farZ - _nearZ, 1.f));
        const simd::M128 two  = simd::Add(rcp, rcp);
        const simd::M128 zero = simd::Zero();
        const simd::M128 r3   = simd::Blend<0b1000>(simd::Negate(simd::Mult(simd::Set(_left + _right, _top + _bottom, _nearZ, 0.f), rcp)), simd::SetAll(1.f));
        return FromM128X4(simd::M128X4 {
            { simd::Blend<0b0001>(zero, two), simd::Blend<0b0010>(zero, two), simd::Blend<0b0100>(zero, rcp), r3 }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvOrtho(
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

        const simd::M128 half = simd::Mult(simd::Set(_right - _left, _top - _bottom, _right + _left, _top + _bottom), simd::SetAll(0.5f));
        const simd::M128 zero = simd::Zero();
        return FromM128X4(simd::M128X4 {
            {
             simd::Blend<0b0001>(zero, half),
             simd::Blend<0b0010>(zero, half),
             simd::Set(0.f, 0.f, _farZ - _nearZ, 0.f),
             simd::Shuffle2<2, 3, 0, 1>(half, simd::Set(_nearZ, 1.f, 0.f, 0.f)),
             }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvOrtho(
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

    [[nodiscard]] JUG_MATH_API constexpr float& operator()(
        const size_t _row,
        const size_t _col)
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr float operator()(
        const size_t _row,
        const size_t _col) const
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return r[0].GetPtr();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return r[0].GetPtr();
    }

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator+(
        const MATRIX& _other) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128X4 y = _other.ToM128X4();
        return FromM128X4(simd::M128X4 {
            { simd::Add(x.r[0], y.r[0]), simd::Add(x.r[1], y.r[1]), simd::Add(x.r[2], y.r[2]), simd::Add(x.r[3], y.r[3]) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator-(
        const MATRIX& _other) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128X4 y = _other.ToM128X4();
        return FromM128X4(simd::M128X4 {
            { simd::Sub(x.r[0], y.r[0]), simd::Sub(x.r[1], y.r[1]), simd::Sub(x.r[2], y.r[2]), simd::Sub(x.r[3], y.r[3]) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
        const float _scalar) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128   s = simd::SetAll(_scalar);
        return FromM128X4(simd::M128X4 {
            { simd::Mult(x.r[0], s), simd::Mult(x.r[1], s), simd::Mult(x.r[2], s), simd::Mult(x.r[3], s) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator/(
        const float _scalar) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128   s = simd::SetAll(_scalar);
        return FromM128X4(simd::M128X4 {
            { simd::Div(x.r[0], s), simd::Div(x.r[1], s), simd::Div(x.r[2], s), simd::Div(x.r[3], s) }
        });
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
        const MATRIX& _other) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128X4 y = _other.ToM128X4();
#if defined(JUG_SIMD_FMA)
        if (!std::is_constant_evaluated())
        {
            const __m256 x01 = _mm256_insertf128_ps(_mm256_castps128_ps256(x.r[0]), x.r[1], 1);
            const __m256 x23 = _mm256_insertf128_ps(_mm256_castps128_ps256(x.r[2]), x.r[3], 1);
            const __m256 b0  = _mm256_insertf128_ps(_mm256_castps128_ps256(y.r[0]), y.r[0], 1);
            const __m256 b1  = _mm256_insertf128_ps(_mm256_castps128_ps256(y.r[1]), y.r[1], 1);
            const __m256 b2  = _mm256_insertf128_ps(_mm256_castps128_ps256(y.r[2]), y.r[2], 1);
            const __m256 b3  = _mm256_insertf128_ps(_mm256_castps128_ps256(y.r[3]), y.r[3], 1);

            __m256 r01 = _mm256_mul_ps(_mm256_shuffle_ps(x01, x01, 0x00), b0);
            __m256 r23 = _mm256_mul_ps(_mm256_shuffle_ps(x23, x23, 0x00), b0);
            r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0x55), b1, r01);
            r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0x55), b1, r23);
            r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xaa), b2, r01);
            r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xaa), b2, r23);
            r01        = _mm256_fmadd_ps(_mm256_shuffle_ps(x01, x01, 0xff), b3, r01);
            r23        = _mm256_fmadd_ps(_mm256_shuffle_ps(x23, x23, 0xff), b3, r23);
            return FromM128X4(simd::M128X4 {
                { _mm256_castps256_ps128(r01), _mm256_extractf128_ps(r01, 1), _mm256_castps256_ps128(r23), _mm256_extractf128_ps(r23, 1) }
            });
        }
#endif
        return FromM128X4(simd::M128X4 {
            { matrix_detail::MultRow(x.r[0], y), matrix_detail::MultRow(x.r[1], y), matrix_detail::MultRow(x.r[2], y), matrix_detail::MultRow(x.r[3], y) }
        });
    }

    [[nodiscard]] JUG_MATH_API friend constexpr MATRIX operator*(
        const float   _scalar,
        const MATRIX& _mtx)
    {
        return _mtx * _scalar;
    }

    [[nodiscard]] JUG_MATH_API friend constexpr MATRIX operator/(
        const float   _scalar,
        const MATRIX& _mtx)
    {
        const simd::M128X4 x = _mtx.ToM128X4();
        const simd::M128   s = simd::SetAll(_scalar);
        return FromM128X4(simd::M128X4 {
            { simd::Div(s, x.r[0]), simd::Div(s, x.r[1]), simd::Div(s, x.r[2]), simd::Div(s, x.r[3]) }
        });
    }

    JUG_MATH_API constexpr MATRIX& operator+=(
        const MATRIX& _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator-=(
        const MATRIX& _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator*=(
        const MATRIX& _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const MATRIX& _other) const
    {
        const simd::M128X4 x = ToM128X4();
        const simd::M128X4 y = _other.ToM128X4();
        const simd::M128   e = simd::And(simd::And(simd::CmpEq(x.r[0], y.r[0]), simd::CmpEq(x.r[1], y.r[1])), simd::And(simd::CmpEq(x.r[2], y.r[2]), simd::CmpEq(x.r[3], y.r[3])));
        return simd::MoveMask(e) == 0xf;
    }

    const static MATRIX kIdentity;
    const static MATRIX kZero;

    constexpr static size_t kRow = 4;
    constexpr static size_t kCol = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };

        ARRAY<VECTOR4, 4>         r;
        ARRAY<float, 16>          e;
        ARRAY<ARRAY<float, 4>, 4> m;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(MATRIX) == 64, "MATRIX must be tightly packed");
static_assert(alignof(MATRIX) == 16, "MATRIX must be 16-byte aligned");
static_assert(PodT<MATRIX>, "MATRIX must be POD type.");

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
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr MATRIX Abs(
    const MATRIX& _mtx)
{
    const simd::M128X4 x = _mtx.ToM128X4();
    return MATRIX::FromM128X4(simd::M128X4 {
        { simd::Abs(x.r[0]), simd::Abs(x.r[1]), simd::Abs(x.r[2]), simd::Abs(x.r[3]) }
    });
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const MATRIX& _x,
    const MATRIX& _y,
    const float   _epsilon = kEpsilon)
{
    const simd::M128X4 x   = _x.ToM128X4();
    const simd::M128X4 y   = _y.ToM128X4();
    const simd::M128   eps = simd::SetAll(_epsilon);
    const simd::M128   d01 = simd::And(simd::CmpLt(simd::Abs(simd::Sub(x.r[0], y.r[0])), eps), simd::CmpLt(simd::Abs(simd::Sub(x.r[1], y.r[1])), eps));
    const simd::M128   d23 = simd::And(simd::CmpLt(simd::Abs(simd::Sub(x.r[2], y.r[2])), eps), simd::CmpLt(simd::Abs(simd::Sub(x.r[3], y.r[3])), eps));
    return simd::MoveMask(simd::And(d01, d23)) == 0xf;
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX Transpose(
    const MATRIX& _mtx)
{
    simd::M128X4 x = _mtx.ToM128X4();
    simd::Transpose4(x.r[0], x.r[1], x.r[2], x.r[3]);
    return MATRIX::FromM128X4(x);
}

[[nodiscard]] JUG_MATH_API constexpr float Determinant(
    const MATRIX& _mtx)
{
    const simd::M128X4 x  = _mtx.ToM128X4();
    const simd::M128   r0 = x.r[0];
    const simd::M128   r1 = x.r[1];
    const simd::M128   r2 = x.r[2];
    const simd::M128   r3 = x.r[3];

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
    return simd::Get<0>(simd::Dot4V(s, ret));
}

// 역행렬이 없으면 (행렬식이 정확히 0) 영행렬
[[nodiscard]] JUG_MATH_API constexpr MATRIX Inverse(
    const MATRIX& _mtx,
    float*        _outDetOrNull = nullptr)
{
    const simd::M128X4 m  = _mtx.ToM128X4();
    const simd::M128   r0 = m.r[0];
    const simd::M128   r1 = m.r[1];
    const simd::M128   r2 = m.r[2];
    const simd::M128   r3 = m.r[3];

    const simd::M128 a = simd::Shuffle2<0, 1, 0, 1>(r0, r1);
    const simd::M128 b = simd::Shuffle2<2, 3, 2, 3>(r0, r1);
    const simd::M128 c = simd::Shuffle2<0, 1, 0, 1>(r2, r3);
    const simd::M128 d = simd::Shuffle2<2, 3, 2, 3>(r2, r3);

    const simd::M128 detSub = simd::NegMultAdd(simd::Shuffle2<1, 3, 1, 3>(r0, r2), simd::Shuffle2<0, 2, 0, 2>(r1, r3), simd::Mult(simd::Shuffle2<0, 2, 0, 2>(r0, r2), simd::Shuffle2<1, 3, 1, 3>(r1, r3)));
    const simd::M128 detA   = simd::Splat<0>(detSub);
    const simd::M128 detB   = simd::Splat<1>(detSub);
    const simd::M128 detC   = simd::Splat<2>(detSub);
    const simd::M128 detD   = simd::Splat<3>(detSub);

    const simd::M128 dc = simd::NegMultAdd(simd::Shuffle<1, 1, 2, 2>(d), simd::Shuffle<2, 3, 0, 1>(c), simd::Mult(simd::Shuffle<3, 3, 0, 0>(d), c));
    const simd::M128 ab = simd::NegMultAdd(simd::Shuffle<1, 1, 2, 2>(a), simd::Shuffle<2, 3, 0, 1>(b), simd::Mult(simd::Shuffle<3, 3, 0, 0>(a), b));

    simd::M128 x = simd::NegMultAdd(simd::Shuffle<1, 0, 3, 2>(b), simd::Shuffle<2, 1, 2, 1>(dc), simd::MultSub(detD, a, simd::Mult(b, simd::Shuffle<0, 3, 0, 3>(dc))));
    simd::M128 w = simd::NegMultAdd(simd::Shuffle<1, 0, 3, 2>(c), simd::Shuffle<2, 1, 2, 1>(ab), simd::MultSub(detA, d, simd::Mult(c, simd::Shuffle<0, 3, 0, 3>(ab))));
    simd::M128 y = simd::MultAdd(simd::Shuffle<1, 0, 3, 2>(d), simd::Shuffle<2, 1, 2, 1>(ab), simd::MultSub(detB, c, simd::Mult(d, simd::Shuffle<3, 0, 3, 0>(ab))));
    simd::M128 z = simd::MultAdd(simd::Shuffle<1, 0, 3, 2>(a), simd::Shuffle<2, 1, 2, 1>(dc), simd::MultSub(detC, b, simd::Mult(a, simd::Shuffle<3, 0, 3, 0>(dc))));

    const simd::M128 tr   = simd::SumAll(simd::Mult(ab, simd::Shuffle<0, 2, 1, 3>(dc)));
    const simd::M128 detM = simd::Sub(simd::MultAdd(detB, detC, simd::Mult(detA, detD)), tr);
    const float      det  = simd::Get<0>(detM);
    if (_outDetOrNull)
    {
        *_outDetOrNull = det;
    }
    if (det == 0.f)
    {
        return MATRIX::kZero;
    }

    const simd::M128 rDetM = simd::Div(simd::Set(1.f, -1.f, -1.f, 1.f), detM);
    x                      = simd::Mult(x, rDetM);
    y                      = simd::Mult(y, rDetM);
    z                      = simd::Mult(z, rDetM);
    w                      = simd::Mult(w, rDetM);
    return MATRIX::FromM128X4(simd::M128X4 {
        { simd::Shuffle2<3, 1, 3, 1>(x, y), simd::Shuffle2<2, 0, 2, 0>(x, y), simd::Shuffle2<3, 1, 3, 1>(z, w), simd::Shuffle2<2, 0, 2, 0>(z, w) }
    });
}

JUG_MATH_API constexpr void Decompose(
    const MATRIX& _mtx,
    VECTOR3*      _pOutScaleOrNull,
    QUAT*         _pOutRotationOrNull,
    VECTOR3*      _pOutTranslationOrNull);

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Xform(
    const VECTOR4 _v,
    const MATRIX& _mtx)
{
    return VECTOR4::FromM128(matrix_detail::MultRow(_v.ToM128(), _mtx.ToM128X4()));
}

// (x, y, z, 1) 을 변환하고 w 로 나눈다
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformPoint(
    const VECTOR3 _point,
    const MATRIX& _mtx)
{
    const simd::M128X4 m = _mtx.ToM128X4();
    const simd::M128   p = _point.ToM128();
#if defined(JUG_SIMD_FMA)
    simd::M128 ret = simd::MultAdd(simd::Splat<0>(p), m.r[0], m.r[3]);
    ret            = simd::MultAdd(simd::Splat<1>(p), m.r[1], ret);
    ret            = simd::MultAdd(simd::Splat<2>(p), m.r[2], ret);
#else
    const simd::M128 xw  = simd::MultAdd(simd::Splat<0>(p), m.r[0], m.r[3]);
    const simd::M128 yz  = simd::MultAdd(simd::Splat<2>(p), m.r[2], simd::Mult(simd::Splat<1>(p), m.r[1]));
    const simd::M128 ret = simd::Add(xw, yz);
#endif
    return VECTOR3::FromM128(simd::Div(ret, simd::Splat<3>(ret)));
}

// (x, y, z, 0) 을 변환한다 (이동 무시)
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformVector(
    const VECTOR3 _v,
    const MATRIX& _mtx)
{
    const simd::M128X4 m  = _mtx.ToM128X4();
    const simd::M128   v  = _v.ToM128();
    const simd::M128   xz = simd::MultAdd(simd::Splat<2>(v), m.r[2], simd::Mult(simd::Splat<0>(v), m.r[0]));
    return VECTOR3::FromM128(simd::MultAdd(simd::Splat<1>(v), m.r[1], xz));
}

}   // namespace jug

#include "Matrix.inl"
