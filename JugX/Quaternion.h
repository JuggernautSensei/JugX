#pragma once
#include "AxisAngle.h"
#include "Vector.h"

namespace jug
{

struct MATRIX;

struct alignas(16) QUAT
{
    using ValueT = float;

    JUG_MATH_API QUAT() = default;

    JUG_MATH_API constexpr QUAT(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        : v { _x, _y, _z, _w }
    {
    }

    JUG_MATH_API constexpr QUAT(
        const VECTOR3 _imaginary,
        const float   _real)
        : v { _imaginary, _real }
    {
    }

    explicit JUG_MATH_API constexpr QUAT(
        const VECTOR4 _xyzw)
        : v { _xyzw }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr simd::M128 JUG_VECTORCALL ToM128() const
    {
        return simd::LoadAligned(v.GetPtr());
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUAT JUG_VECTORCALL FromM128(
        const simd::M128 _q)
    {
        if (std::is_constant_evaluated())
        {
            return simd::NoInlineConstruct<QUAT>(simd::Get<0>(_q), simd::Get<1>(_q), simd::Get<2>(_q), simd::Get<3>(_q));
        }
        else
        {
            return std::bit_cast<QUAT>(_q);
        }
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API static constexpr QUAT MakeFromTo(
        const VECTOR3 _dirFrom,
        const VECTOR3 _dirTo)
    {
        JUG_ASSERT(IsNormalized(_dirFrom) && IsNormalized(_dirTo), "From and To must be normalized vectors");

        const simd::M128 from = _dirFrom.ToM128();
        const simd::M128 to   = _dirTo.ToM128();
        const float      d    = simd::Get<0>(simd::Dot3V(from, to));
        if (Abs(d) > kAlmostOne)
        {
            if (d >= 0)
            {
                return QUAT { 0.f, 0.f, 0.f, 1.f };
            }

            const simd::M128 other = Abs(_dirFrom[0]) <= Abs(_dirFrom[1]) ? simd::Set(1.f, 0.f, 0.f, 0.f) : simd::Set(0.f, 1.f, 0.f, 0.f);
            return FromM128(simd::ZeroW(simd::Normalize3(simd::Cross3(other, from))));
        }

        const float s = Sqrt((1.f + d) * 2.f);
        return FromM128(simd::Insert<3>(simd::Mult(simd::Cross3(from, to), simd::SetAll(1.f / s)), s * 0.5f));
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUAT MakeFromAxisAngle(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const simd::M128 sc = simd::SinCos1(_rad * 0.5f);
        return FromM128(simd::Mult(simd::Shuffle<0, 0, 0, 1>(sc), simd::Blend<0b1000>(_axis.ToM128(), simd::SetAll(1.f))));
    }

    // pitch(x) -> yaw(y) -> roll(z)
    [[nodiscard]] JUG_MATH_API constexpr static QUAT MakeFromEuler(
        const VECTOR3 _pyrRad)
    {
        const auto [s, c]   = simd::SinCos(simd::Mult(_pyrRad.ToM128(), simd::SetAll(0.5f)));
        const simd::M128 p0 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(s, c));
        const simd::M128 y0 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(c, s));
        const simd::M128 r0 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(c, s));
        const simd::M128 p1 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(c, s));
        const simd::M128 y1 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(s, c));
        const simd::M128 r1 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(s, c));

        const simd::M128 q0 = simd::Mult(simd::Mult(p0, y0), r0);
        const simd::M128 q1 = simd::Mult(simd::Mult(p1, simd::Set(1.f, -1.f, -1.f, 1.f)), y1);
        return FromM128(simd::MultAdd(q1, r1, q0));
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUAT MakeFromMatrix(
        const MATRIX& _mtx);

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr float& operator[](
        const size_t _index)
    {
        return v[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr float operator[](
        const size_t _index) const
    {
        return v[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 GetImaginary() const
    {
        return v.ToVector3();
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return v.GetPtr();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return v.GetPtr();
    }

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr QUAT operator-() const
    {
        return FromM128(simd::Negate(ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr QUAT operator+(
        const QUAT _other) const
    {
        return FromM128(simd::Add(ToM128(), _other.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr QUAT operator-(
        const QUAT _other) const
    {
        return FromM128(simd::Sub(ToM128(), _other.ToM128()));
    }

    [[nodiscard]] JUG_MATH_API constexpr QUAT operator*(
        const float _scalar) const
    {
        return FromM128(simd::Mult(ToM128(), simd::SetAll(_scalar)));
    }

    [[nodiscard]] JUG_MATH_API constexpr QUAT operator/(
        const float _scalar) const
    {
        return FromM128(simd::Div(ToM128(), simd::SetAll(_scalar)));
    }

    // a * b 는 a 회전을 먼저 적용하고 b 회전을 적용
    [[nodiscard]] JUG_MATH_API constexpr QUAT operator*(
        const QUAT _other) const
    {
        const simd::M128 q1  = ToM128();
        const simd::M128 q2  = _other.ToM128();
        // 부호 상수를 마지막에 곱해야 SSE2 에서 상수 레지스터 복사(movaps)가 안 생김
        const simd::M128 wx = simd::MultAdd(simd::Mult(simd::Splat<0>(q2), simd::Shuffle<3, 2, 1, 0>(q1)), simd::Set(1.f, -1.f, 1.f, -1.f), simd::Mult(simd::Splat<3>(q2), q1));
        const simd::M128 yz = simd::MultAdd(simd::Mult(simd::Splat<2>(q2), simd::Shuffle<1, 0, 3, 2>(q1)), simd::Set(-1.f, 1.f, 1.f, -1.f), simd::Mult(simd::Mult(simd::Splat<1>(q2), simd::Shuffle<2, 3, 0, 1>(q1)), simd::Set(1.f, 1.f, -1.f, -1.f)));
        return FromM128(simd::Add(wx, yz));
    }

    [[nodiscard]] JUG_MATH_API friend constexpr QUAT operator*(
        const float _scalar,
        const QUAT  _q)
    {
        return _q * _scalar;
    }

    JUG_MATH_API constexpr QUAT& operator+=(
        const QUAT _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr QUAT& operator-=(
        const QUAT _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr QUAT& operator*=(
        const QUAT _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr QUAT& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr QUAT& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const QUAT _other) const
    {
        return simd::MoveMask(simd::CmpEq(ToM128(), _other.ToM128())) == 0xf;
    }

    // =======================================================
    //  Conversion
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr AXIS_ANGLE ToAxisAngle() const;
    [[nodiscard]] JUG_MATH_API constexpr VECTOR3    ToEuler() const;

    const static QUAT kZero;
    const static QUAT kIdentity;

    constexpr static size_t kDim = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        VECTOR4         v;
        ARRAY<float, 4> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(QUAT) == 16, "QUAT must be tightly packed");
static_assert(alignof(QUAT) == 16, "QUAT must be 16-byte aligned");
static_assert(PodT<QUAT>, "QUAT must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr QUAT QUAT::kZero     = { 0.f, 0.f, 0.f, 0.f };
inline constexpr QUAT QUAT::kIdentity = { 0.f, 0.f, 0.f, 1.f };

template<>
struct MathConstants<QUAT>
{
    constexpr static QUAT kZero     = QUAT::kZero;
    constexpr static QUAT kIdentity = QUAT::kIdentity;
};

// ======================================================
//  Basic
// ======================================================

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const QUAT  _x,
    const QUAT  _y,
    const float _epsilon = kEpsilon)
{
    return simd::MoveMask(simd::CmpLt(simd::Abs(simd::Sub(_x.ToM128(), _y.ToM128())), simd::SetAll(_epsilon))) == 0xf;
}

[[nodiscard]] JUG_MATH_API constexpr float Dot(
    const QUAT _x,
    const QUAT _y)
{
    return simd::Get<0>(simd::Dot4V(_x.ToM128(), _y.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr float LengthSq(
    const QUAT _q)
{
    return Dot(_q, _q);
}

[[nodiscard]] JUG_MATH_API constexpr float Length(
    const QUAT _q)
{
    const simd::M128 q = _q.ToM128();
    return simd::Get<0>(simd::Sqrt(simd::Dot4V(q, q)));
}

[[nodiscard]] JUG_MATH_API constexpr QUAT Normalize(
    const QUAT _q)
{
    const simd::M128 q = _q.ToM128();
    return QUAT::FromM128(simd::NormalizeBy(q, simd::Dot4V(q, q)));
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const QUAT _q)
{
    return IsEqualApprox(LengthSq(_q), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsSameRotationApprox(
    const QUAT _x,
    const QUAT _y)
{
    JUG_ASSERT(IsNormalized(_x) && IsNormalized(_y), "QUAT must be normalized");
    return IsEqualApprox(Abs(Dot(_x, _y)), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr QUAT Conjugate(
    const QUAT _q)
{
    return QUAT::FromM128(simd::Xor(_q.ToM128(), simd::Set(-0.f, -0.f, -0.f, 0.f)));
}

// 영 사원수의 역원은 영 사원수
[[nodiscard]] JUG_MATH_API constexpr QUAT Inverse(
    const QUAT _q)
{
    const simd::M128 q     = _q.ToM128();
    const simd::M128 lenSq = simd::Dot4V(q, q);
    if (IsZeroApprox(simd::Get<0>(lenSq)))
    {
        return QUAT::kZero;
    }
    return QUAT::FromM128(simd::Div(simd::Xor(q, simd::Set(-0.f, -0.f, -0.f, 0.f)), lenSq));
}

// =======================================================
//  Vector
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Rotate(
    const VECTOR3 _v,
    const QUAT    _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUAT must be normalized");
    const simd::M128 q = _q.ToM128();
    const simd::M128 v = _v.ToM128();
    const simd::M128 t = simd::Cross3(q, simd::Add(v, v));
    return VECTOR3::FromM128(simd::Add(simd::MultAdd(simd::Splat<3>(q), t, v), simd::Cross3(q, t)));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 RotateInv(
    const VECTOR3 _v,
    const QUAT    _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUAT must be normalized");
    const simd::M128 q = _q.ToM128();
    const simd::M128 v = _v.ToM128();
    const simd::M128 t = simd::Cross3(q, simd::Add(v, v));
    return VECTOR3::FromM128(simd::Add(simd::NegMultAdd(simd::Splat<3>(q), t, v), simd::Cross3(q, t)));
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr QUAT Nlerp(
    const QUAT  _x,
    const QUAT  _y,
    const float _t)
{
    const simd::M128 x   = _x.ToM128();
    const simd::M128 dot = simd::Dot4V(x, _y.ToM128());
    const simd::M128 y   = simd::Xor(_y.ToM128(), simd::And(dot, simd::SetAll(-0.f)));
    const simd::M128 q   = simd::Lerp(x, y, simd::SetAll(_t));
    return QUAT::FromM128(simd::NormalizeBy(q, simd::Dot4V(q, q)));
}

[[nodiscard]] JUG_MATH_API constexpr QUAT Slerp(
    const QUAT  _x,
    const QUAT  _y,
    const float _t)
{
    const simd::M128 x   = _x.ToM128();
    const simd::M128 dot = simd::Dot4V(x, _y.ToM128());
    const simd::M128 y   = simd::Xor(_y.ToM128(), simd::And(dot, simd::SetAll(-0.f)));
    const simd::M128 c   = simd::Abs(dot);
    if (simd::Get<0>(c) > kAlmostOne)
    {
        const simd::M128 q = simd::Lerp(x, y, simd::SetAll(_t));
        return QUAT::FromM128(simd::NormalizeBy(q, simd::Dot4V(q, q)));
    }

    const simd::M128 c2    = simd::Mult(c, c);
    const simd::M128 c4    = simd::Mult(c2, c2);
    const simd::M128 p01   = simd::MultAdd(simd::SetAll(-0.2145988016f), c, simd::SetAll(1.5707963050f));
    const simd::M128 p23   = simd::MultAdd(simd::SetAll(-0.0501743046f), c, simd::SetAll(0.0889789874f));
    const simd::M128 p45   = simd::MultAdd(simd::SetAll(-0.0170881256f), c, simd::SetAll(0.0308918810f));
    const simd::M128 p67   = simd::MultAdd(simd::SetAll(-0.0012624911f), c, simd::SetAll(0.0066700901f));
    const simd::M128 poly  = simd::MultAdd(simd::MultAdd(p67, c2, p45), c4, simd::MultAdd(p23, c2, p01));
    const simd::M128 omega = simd::Mult(simd::Sqrt(simd::Sub(simd::SetAll(1.f), c)), poly);

    const simd::M128 s     = simd::Sin(simd::Mult(omega, simd::Set(1.f, 1.f - _t, _t, 0.f)));
    const simd::M128 scale = simd::Div(s, simd::Splat<0>(s));
    return QUAT::FromM128(simd::MultAdd(x, simd::Splat<1>(scale), simd::Mult(y, simd::Splat<2>(scale))));
}

// =======================================================
//  Conversion
// =======================================================

JUG_MATH_API constexpr AXIS_ANGLE QUAT::ToAxisAngle() const
{
    JUG_ASSERT(IsNormalized(*this), "QUAT must be normalized");

    const QUAT shortest = v[3] < 0.f ? -(*this) : *this;

    AXIS_ANGLE ret;
    ret.rad             = 2.f * ACos(Clamp(shortest[3], -1.f, 1.f));
    const VECTOR3 img   = shortest.GetImaginary();
    const float   lenSq = LengthSq(img);
    if (IsZeroApprox(lenSq))   // 회전이 없으면 축이 정의되지 않는다 - +X 로 대체
    {
        ret.axis = MathConstants<VECTOR3>::kUnitX;
        return ret;
    }

    ret.axis = img * RSqrt(lenSq);
    return ret;
}

JUG_MATH_API constexpr VECTOR3 QUAT::ToEuler() const
{
    JUG_ASSERT(IsNormalized(*this), "QUAT must be normalized");

    const float qx = v[0], qy = v[1], qz = v[2], qw = v[3];
    const float m21      = 2.f * (qy * qz - qx * qw);
    const float sinPitch = Clamp(-m21, -1.f, 1.f);
    const float pitchRad = ASin(sinPitch);

    if (Abs(sinPitch) > kAlmostOne)   // 짐벌락 - roll 은 0 으로 두고 yaw 만
    {
        const float m00    = 1.f - 2.f * (qy * qy + qz * qz);
        const float m02    = 2.f * (qx * qz - qy * qw);
        const float yawRad = ATan2(-m02, m00);
        return VECTOR3 { pitchRad, yawRad, 0.f };
    }

    const float m20     = 2.f * (qx * qz + qy * qw);
    const float m22     = 1.f - 2.f * (qx * qx + qy * qy);
    const float m01     = 2.f * (qx * qy + qz * qw);
    const float m11     = 1.f - 2.f * (qx * qx + qz * qz);
    const float yawRad  = ATan2(m20, m22);
    const float rollRad = ATan2(m01, m11);
    return VECTOR3 { pitchRad, yawRad, rollRad };
}

}   // namespace jug

#include "Quaternion.inl"
