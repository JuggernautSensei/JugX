#pragma once
#include "Quaternion.h"
#include "VectorReg.h"

namespace jug
{

struct MATRIX_REG;

// 연산 타입. QUATERNION 과는 Load / Store 로 오간다.
struct alignas(16) QUATERNION_REG
{
    QUATERNION_REG() = default;

    constexpr QUATERNION_REG(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        : v(simd::Set(_x, _y, _z, _w))
    {
    }

    QUATERNION_REG(
        const VECTOR3_REG _imaginary,
        const float       _real)
        : v(simd::Insert<3>(_imaginary.v, _real))
    {
    }

    explicit constexpr QUATERNION_REG(
        const simd::M128 _value)
        : v(_value)
    {
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] static QUATERNION_REG JUG_VECTORCALL MakeFromTo(
        const VECTOR3_REG _dirFrom,
        const VECTOR3_REG _dirTo)
    {
        JUG_ASSERT(IsNormalized(_dirFrom) && IsNormalized(_dirTo), "From and To must be normalized vectors");

        const float d = Dot(_dirFrom, _dirTo);
        if (Abs(d) > kAlmostOne)
        {
            if (d >= 0)
            {
                return QUATERNION_REG { 0.f, 0.f, 0.f, 1.f };
            }

            // 반대 방향이면 수직인 아무 축으로 180도
            const VECTOR3_REG axis = Normalize(Cross(Abs(_dirFrom.GetX()) <= Abs(_dirFrom.GetY()) ? MathConstants<VECTOR3_REG>::kRight : MathConstants<VECTOR3_REG>::kUp, _dirFrom));
            return QUATERNION_REG { axis, 0.f };
        }

        const float s = Sqrt((1.f + d) * 2.f);
        return QUATERNION_REG { Cross(_dirFrom, _dirTo) * (1.f / s), s * 0.5f };
    }

    [[nodiscard]] static QUATERNION_REG JUG_VECTORCALL MakeFromAxisAngle(
        const VECTOR3_REG _axis,
        const float       _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const simd::M128 sc = simd::SinCos1(_rad * 0.5f);   // s c s c
        return QUATERNION_REG { simd::Mult(simd::Shuffle<0, 0, 0, 1>(sc), simd::Blend<0b1000>(_axis.v, simd::SetAll(1.f))) };
    }

    // pitch(x) -> yaw(y) -> roll(z) 순서의 오일러 각 (라디안)
    [[nodiscard]] static QUATERNION_REG JUG_VECTORCALL MakeFromEuler(
        const VECTOR3_REG _pyrRad)
    {
        // (sp, cp, cp, cp) * (cy, sy, cy, cy) * (cr, cr, sr, cr) + (cp, -sp, -sp, sp) * (sy, cy, sy, sy) * (sr, sr, cr, sr)
        simd::M128 s;
        simd::M128 c;
        simd::SinCos(simd::Mult(_pyrRad.v, simd::SetAll(0.5f)), s, c);

        const simd::M128 p0 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(s, c));
        const simd::M128 y0 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(c, s));
        const simd::M128 r0 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(c, s));
        const simd::M128 p1 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(c, s));
        const simd::M128 y1 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(s, c));
        const simd::M128 r1 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(s, c));

        const simd::M128 q0 = simd::Mult(simd::Mult(p0, y0), r0);
        const simd::M128 q1 = simd::Mult(simd::Mult(p1, simd::Set(1.f, -1.f, -1.f, 1.f)), y1);
        return QUATERNION_REG { simd::MultAdd(q1, r1, q0) };
    }

    [[nodiscard]] static QUATERNION_REG JUG_VECTORCALL MakeFromMatrix(
        const MATRIX_REG& _mtx);

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] float GetX() const
    {
        return simd::Get<0>(v);
    }

    [[nodiscard]] float GetY() const
    {
        return simd::Get<1>(v);
    }

    [[nodiscard]] float GetZ() const
    {
        return simd::Get<2>(v);
    }

    [[nodiscard]] float GetW() const
    {
        return simd::Get<3>(v);
    }

    [[nodiscard]] VECTOR3_REG GetImaginary() const
    {
        return VECTOR3_REG { v };
    }

    QUATERNION_REG& operator+=(QUATERNION_REG _other);
    QUATERNION_REG& operator-=(QUATERNION_REG _other);
    QUATERNION_REG& operator*=(QUATERNION_REG _other);
    QUATERNION_REG& operator*=(float _scalar);
    QUATERNION_REG& operator/=(float _scalar);

    const static QUATERNION_REG kZero;
    const static QUATERNION_REG kIdentity;

    simd::M128 v;
};

static_assert(sizeof(QUATERNION_REG) == 16 && alignof(QUATERNION_REG) == 16, "QUATERNION_REG must be a single 16-byte register");

// =======================================================
//  Constants
// =======================================================

inline constexpr QUATERNION_REG QUATERNION_REG::kZero     = { 0.f, 0.f, 0.f, 0.f };
inline constexpr QUATERNION_REG QUATERNION_REG::kIdentity = { 0.f, 0.f, 0.f, 1.f };

template<>
struct MathConstants<QUATERNION_REG>
{
    constexpr static QUATERNION_REG kZero     = QUATERNION_REG::kZero;
    constexpr static QUATERNION_REG kIdentity = QUATERNION_REG::kIdentity;
};

// =======================================================
//  Load / Store
// =======================================================

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Load(
    const QUATERNION& _q)
{
    return QUATERNION_REG { simd::LoadAligned(_q.GetPtr()) };
}

[[nodiscard]] inline QUATERNION JUG_VECTORCALL Store(
    const QUATERNION_REG _q)
{
    QUATERNION ret;
    simd::StoreAligned(ret.GetPtr(), _q.v);
    return ret;
}

// ======================================================
//  Operators
// ======================================================

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator-(
    const QUATERNION_REG _q)
{
    return QUATERNION_REG { simd::Negate(_q.v) };
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator+(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    return QUATERNION_REG { simd::Add(_x.v, _y.v) };
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator-(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    return QUATERNION_REG { simd::Sub(_x.v, _y.v) };
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator*(
    const QUATERNION_REG _q,
    const float          _scalar)
{
    return QUATERNION_REG { simd::Mult(_q.v, simd::SetAll(_scalar)) };
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator*(
    const float          _scalar,
    const QUATERNION_REG _q)
{
    return QUATERNION_REG { simd::Mult(_q.v, simd::SetAll(_scalar)) };
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator/(
    const QUATERNION_REG _q,
    const float          _scalar)
{
    return QUATERNION_REG { simd::Div(_q.v, simd::SetAll(_scalar)) };
}

// _x * _y 는 _x 회전을 먼저 적용하고 _y 회전을 적용 (DirectXMath XMQuaternionMultiply 와 같음)
[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL operator*(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    const simd::M128 q1  = _x.v;
    const simd::M128 q2  = _y.v;
    simd::M128       ret = simd::Mult(simd::Splat<3>(q2), q1);
    ret                  = simd::MultAdd(simd::Mult(simd::Splat<0>(q2), simd::Set(1.f, -1.f, 1.f, -1.f)), simd::Shuffle<3, 2, 1, 0>(q1), ret);
    ret                  = simd::MultAdd(simd::Mult(simd::Splat<1>(q2), simd::Set(1.f, 1.f, -1.f, -1.f)), simd::Shuffle<2, 3, 0, 1>(q1), ret);
    ret                  = simd::MultAdd(simd::Mult(simd::Splat<2>(q2), simd::Set(-1.f, 1.f, 1.f, -1.f)), simd::Shuffle<1, 0, 3, 2>(q1), ret);
    return QUATERNION_REG { ret };
}

[[nodiscard]] inline bool JUG_VECTORCALL operator==(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    return simd::MoveMask(simd::CmpEq(_x.v, _y.v)) == 0xf;
}

[[nodiscard]] inline bool JUG_VECTORCALL operator!=(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    return !(_x == _y);
}

inline QUATERNION_REG& QUATERNION_REG::operator+=(
    const QUATERNION_REG _other)
{
    *this = *this + _other;
    return *this;
}

inline QUATERNION_REG& QUATERNION_REG::operator-=(
    const QUATERNION_REG _other)
{
    *this = *this - _other;
    return *this;
}

inline QUATERNION_REG& QUATERNION_REG::operator*=(
    const QUATERNION_REG _other)
{
    *this = *this * _other;
    return *this;
}

inline QUATERNION_REG& QUATERNION_REG::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

inline QUATERNION_REG& QUATERNION_REG::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

// ======================================================
//  Basic
// ======================================================

[[nodiscard]] inline bool JUG_VECTORCALL IsEqualApprox(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y,
    const float          _epsilon = kEpsilon)
{
    return simd::MoveMask(simd::CmpLt(simd::Abs(simd::Sub(_x.v, _y.v)), simd::SetAll(_epsilon))) == 0xf;
}

[[nodiscard]] inline float JUG_VECTORCALL Dot(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y)
{
    return simd::GetX(simd::Dot4V(_x.v, _y.v));
}

[[nodiscard]] inline float JUG_VECTORCALL LengthSq(
    const QUATERNION_REG _q)
{
    return Dot(_q, _q);
}

[[nodiscard]] inline float JUG_VECTORCALL Length(
    const QUATERNION_REG _q)
{
    return simd::GetX(simd::Sqrt(simd::Dot4V(_q.v, _q.v)));
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Normalize(
    const QUATERNION_REG _q)
{
    return QUATERNION_REG { simd::NormalizeBy(_q.v, simd::Dot4V(_q.v, _q.v)) };
}

[[nodiscard]] inline bool JUG_VECTORCALL IsNormalized(
    const QUATERNION_REG _q)
{
    return IsEqualApprox(LengthSq(_q), 1.f);
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Conjugate(
    const QUATERNION_REG _q)
{
    return QUATERNION_REG { simd::Xor(_q.v, simd::Set(-0.f, -0.f, -0.f, 0.f)) };
}

// 영 사원수의 역원은 영 사원수
[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Inverse(
    const QUATERNION_REG _q)
{
    const simd::M128 lenSq = simd::Dot4V(_q.v, _q.v);
    if (IsZeroApprox(simd::GetX(lenSq)))
    {
        return QUATERNION_REG::kZero;
    }
    return QUATERNION_REG { simd::Div(simd::Xor(_q.v, simd::Set(-0.f, -0.f, -0.f, 0.f)), lenSq) };
}

// =======================================================
//  Vector
// =======================================================

// t = 2 (q.xyz x v), v' = v + w t + q.xyz x t
[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL Rotate(
    const VECTOR3_REG    _v,
    const QUATERNION_REG _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const simd::M128 t = simd::Cross3(_q.v, simd::Add(_v.v, _v.v));
    return VECTOR3_REG { simd::Add(simd::MultAdd(simd::Splat<3>(_q.v), t, _v.v), simd::Cross3(_q.v, t)) };
}

[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL RotateInv(
    const VECTOR3_REG    _v,
    const QUATERNION_REG _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const simd::M128 t = simd::Cross3(_q.v, simd::Add(_v.v, _v.v));
    return VECTOR3_REG { simd::Add(simd::NegMultAdd(simd::Splat<3>(_q.v), t, _v.v), simd::Cross3(_q.v, t)) };
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Nlerp(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y,
    const float          _t)
{
    const simd::M128 dot = simd::Dot4V(_x.v, _y.v);
    const simd::M128 y   = simd::Xor(_y.v, simd::And(dot, simd::SetAll(-0.f)));   // 최단 경로
    return Normalize(QUATERNION_REG { simd::Lerp(_x.v, y, simd::SetAll(_t)) });
}

[[nodiscard]] inline QUATERNION_REG JUG_VECTORCALL Slerp(
    const QUATERNION_REG _x,
    const QUATERNION_REG _y,
    const float          _t)
{
    // 최단 경로 선택을 분기 없이: dot 의 부호 비트를 _y 에 뒤집어 씌운다
    const simd::M128 x   = _x.v;
    const simd::M128 dot = simd::Dot4V(x, _y.v);
    const simd::M128 y   = simd::Xor(_y.v, simd::And(dot, simd::SetAll(-0.f)));
    const simd::M128 c   = simd::Abs(dot);   // cos(omega)
    if (simd::GetX(c) > kAlmostOne)          // 거의 같은 회전 - sin(omega) 로 나누면 불안정
    {
        return Normalize(QUATERNION_REG { simd::Lerp(x, y, simd::SetAll(_t)) });
    }

    // omega = acos(c) = sqrt(1 - c) * P(c) (ACos 와 같은 7차 다항식, c >= 0 이라 부호 처리 없음)
    const simd::M128 c2    = simd::Mult(c, c);
    const simd::M128 c4    = simd::Mult(c2, c2);
    const simd::M128 p01   = simd::MultAdd(simd::SetAll(-0.2145988016f), c, simd::SetAll(1.5707963050f));
    const simd::M128 p23   = simd::MultAdd(simd::SetAll(-0.0501743046f), c, simd::SetAll(0.0889789874f));
    const simd::M128 p45   = simd::MultAdd(simd::SetAll(-0.0170881256f), c, simd::SetAll(0.0308918810f));
    const simd::M128 p67   = simd::MultAdd(simd::SetAll(-0.0012624911f), c, simd::SetAll(0.0066700901f));
    const simd::M128 poly  = simd::MultAdd(simd::MultAdd(p67, c2, p45), c4, simd::MultAdd(p23, c2, p01));
    const simd::M128 omega = simd::Mult(simd::Sqrt(simd::Sub(simd::SetAll(1.f), c)), poly);

    // sin(omega), sin((1 - t) omega), sin(t omega)
    const simd::M128 s     = simd::Sin(simd::Mult(omega, simd::Set(1.f, 1.f - _t, _t, 0.f)));
    const simd::M128 scale = simd::Div(s, simd::Splat<0>(s));
    return QUATERNION_REG { simd::MultAdd(x, simd::Splat<1>(scale), simd::Mult(y, simd::Splat<2>(scale))) };
}

}   // namespace jug
