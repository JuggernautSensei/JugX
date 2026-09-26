#pragma once
#include "AxisAngle.h"
#include "Vector.h"

namespace jug
{

struct MATRIX;

struct alignas(16) QUATERNION
{
    using ValueT = float;

    JUG_MATH_API QUATERNION() = default;

    JUG_MATH_API constexpr QUATERNION(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        : v { _x, _y, _z, _w }
    {
    }

    JUG_MATH_API constexpr QUATERNION(
        const VECTOR3 _imaginary,
        const float   _real)
        : v { _imaginary, _real }
    {
    }

    explicit JUG_MATH_API constexpr QUATERNION(
        const VECTOR4 _xyzw)
        : v { _xyzw }
    {
    }

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API static constexpr QUATERNION MakeFromTo(
        const VECTOR3 _dirFrom,
        const VECTOR3 _dirTo)
    {
        JUG_ASSERT(IsNormalized(_dirFrom) && IsNormalized(_dirTo), "From and To must be normalized vectors");

        const float d = Dot(_dirFrom, _dirTo);
        if (Abs(d) > kAlmostOne)
        {
            if (d >= 0)
            {
                return QUATERNION { 0.f, 0.f, 0.f, 1.f };
            }

            // 반대 방향이면 수직인 아무 축으로 180도
            const VECTOR3 axis = Normalize(Cross(Abs(_dirFrom[0]) <= Abs(_dirFrom[1]) ? Right<VECTOR3>() : Up<VECTOR3>(), _dirFrom));
            return QUATERNION { axis, 0.f };
        }

        const VECTOR3 c    = Cross(_dirFrom, _dirTo);
        const float   s    = Sqrt((1.f + d) * 2.f);
        const float   invS = 1.f / s;
        return QUATERNION { c * invS, s * 0.5f };
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromAxisAngle(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        float s = 0.f;
        float c = 0.f;
        SinCos(_rad * 0.5f, s, c);
        return QUATERNION { _axis * s, c };
    }

    // pitch(x) -> yaw(y) -> roll(z) 순서의 오일러 각 (라디안)
    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromEuler(
        const VECTOR3 _pyrRad)
    {
        float sp = 0.f, cp = 0.f;
        float sy = 0.f, cy = 0.f;
        float sr = 0.f, cr = 0.f;
        SinCos(_pyrRad[0] * 0.5f, sp, cp);
        SinCos(_pyrRad[1] * 0.5f, sy, cy);
        SinCos(_pyrRad[2] * 0.5f, sr, cr);
        return QUATERNION {
            cy * sp * cr + sy * cp * sr,
            sy * cp * cr - cy * sp * sr,
            cy * cp * sr - sy * sp * cr,
            cy * cp * cr + sy * sp * sr
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromMatrix(
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

    JUG_MATH_API constexpr QUATERNION& operator+=(QUATERNION _other);
    JUG_MATH_API constexpr QUATERNION& operator-=(QUATERNION _other);
    JUG_MATH_API constexpr QUATERNION& operator*=(QUATERNION _other);
    JUG_MATH_API constexpr QUATERNION& operator*=(float _scalar);
    JUG_MATH_API constexpr QUATERNION& operator/=(float _scalar);

    // =======================================================
    //  Conversion
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr AXIS_ANGLE ToAxisAngle() const;
    [[nodiscard]] JUG_MATH_API constexpr VECTOR3    ToEuler() const;

    const static QUATERNION kZero;
    const static QUATERNION kIdentity;

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

static_assert(sizeof(QUATERNION) == 16, "QUATERNION must be tightly packed");
static_assert(alignof(QUATERNION) == 16, "QUATERNION must be 16-byte aligned");
static_assert(PodT<QUATERNION>, "QUATERNION must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr QUATERNION QUATERNION::kZero     = { 0.f, 0.f, 0.f, 0.f };
inline constexpr QUATERNION QUATERNION::kIdentity = { 0.f, 0.f, 0.f, 1.f };

template<>
struct MathConstants<QUATERNION>
{
    constexpr static QUATERNION kZero     = QUATERNION::kZero;
    constexpr static QUATERNION kIdentity = QUATERNION::kIdentity;
};

// ======================================================
//  Operators
// ======================================================

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator-(
    const QUATERNION _q)
{
    return QUATERNION { -_q.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator+(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return QUATERNION { _x.v + _y.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator-(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return QUATERNION { _x.v - _y.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
    const QUATERNION _q,
    const float      _scalar)
{
    return QUATERNION { _q.v * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
    const float      _scalar,
    const QUATERNION _q)
{
    return QUATERNION { _q.v * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator/(
    const QUATERNION _q,
    const float      _scalar)
{
    return QUATERNION { _q.v / _scalar };
}

// _x * _y 는 _x 회전을 먼저 적용하고 _y 회전을 적용 (Hamilton 곱으로는 _y ⊗ _x, DirectXMath XMQuaternionMultiply 와 같음)
[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
    const QUATERNION _x,
    const QUATERNION _y)
{
    const float x1 = _x[0], y1 = _x[1], z1 = _x[2], w1 = _x[3];
    const float x2 = _y[0], y2 = _y[1], z2 = _y[2], w2 = _y[3];
    return QUATERNION {
        w2 * x1 + x2 * w1 + y2 * z1 - z2 * y1,
        w2 * y1 - x2 * z1 + y2 * w1 + z2 * x1,
        w2 * z1 + x2 * y1 - y2 * x1 + z2 * w1,
        w2 * w1 - x2 * x1 - y2 * y1 - z2 * z1
    };
}

[[nodiscard]] JUG_MATH_API constexpr bool operator==(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return _x.v == _y.v;
}

[[nodiscard]] JUG_MATH_API constexpr bool operator!=(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return !(_x.v == _y.v);
}

JUG_MATH_API constexpr QUATERNION& QUATERNION::operator+=(
    const QUATERNION _other)
{
    *this = *this + _other;
    return *this;
}

JUG_MATH_API constexpr QUATERNION& QUATERNION::operator-=(
    const QUATERNION _other)
{
    *this = *this - _other;
    return *this;
}

JUG_MATH_API constexpr QUATERNION& QUATERNION::operator*=(
    const QUATERNION _other)
{
    *this = *this * _other;
    return *this;
}

JUG_MATH_API constexpr QUATERNION& QUATERNION::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

JUG_MATH_API constexpr QUATERNION& QUATERNION::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

// ======================================================
//  Basic
// ======================================================

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _epsilon = kEpsilon)
{
    return IsEqualApprox(_x.v, _y.v, _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr float Dot(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return Dot(_x.v, _y.v);
}

[[nodiscard]] JUG_MATH_API constexpr float LengthSq(
    const QUATERNION _q)
{
    return LengthSq(_q.v);
}

[[nodiscard]] JUG_MATH_API constexpr float Length(
    const QUATERNION _q)
{
    return Length(_q.v);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION Normalize(
    const QUATERNION _q)
{
    return QUATERNION { Normalize(_q.v) };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const QUATERNION _q)
{
    return IsEqualApprox(LengthSq(_q), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsSameRotationApprox(
    const QUATERNION _x,
    const QUATERNION _y)
{
    JUG_ASSERT(IsNormalized(_x) && IsNormalized(_y), "QUATERNION must be normalized");
    return IsEqualApprox(Abs(Dot(_x, _y)), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION Conjugate(
    const QUATERNION _q)
{
    return QUATERNION { -_q[0], -_q[1], -_q[2], _q[3] };
}

// 영 사원수의 역원은 영 사원수
[[nodiscard]] JUG_MATH_API constexpr QUATERNION Inverse(
    const QUATERNION _q)
{
    const float lenSq = LengthSq(_q);
    if (IsZeroApprox(lenSq))
    {
        return MathConstants<QUATERNION>::kZero;
    }
    return Conjugate(_q) * (1.f / lenSq);
}

// =======================================================
//  Vector
// =======================================================

// t = 2 (q.xyz x v), v' = v + w t + q.xyz x t
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Rotate(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = _q.GetImaginary();
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v + t * _q[3] + Cross(img, t);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 RotateInv(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = _q.GetImaginary();
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v - t * _q[3] + Cross(img, t);
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr QUATERNION Nlerp(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _t)
{
    const QUATERNION end = Dot(_x, _y) < 0.f ? -_y : _y;
    return Normalize(_x + (end - _x) * _t);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION Slerp(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _t)
{
    float      cosOmega = Dot(_x, _y);
    QUATERNION end      = _y;
    if (cosOmega < 0.f)   // 최단 경로
    {
        cosOmega = -cosOmega;
        end      = -_y;
    }

    if (cosOmega > kAlmostOne)   // 거의 같은 회전 - sin(omega) 로 나누면 불안정
    {
        return Nlerp(_x, end, _t);
    }

    const float omega       = ACos(cosOmega);
    const float invSinOmega = 1.f / Sin(omega);
    return _x * (Sin((1.f - _t) * omega) * invSinOmega) + end * (Sin(_t * omega) * invSinOmega);
}

// =======================================================
//  Conversion
// =======================================================

JUG_MATH_API constexpr AXIS_ANGLE QUATERNION::ToAxisAngle() const
{
    JUG_ASSERT(IsNormalized(*this), "QUATERNION must be normalized");

    const QUATERNION shortest = v[3] < 0.f ? -(*this) : *this;

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

JUG_MATH_API constexpr VECTOR3 QUATERNION::ToEuler() const
{
    JUG_ASSERT(IsNormalized(*this), "QUATERNION must be normalized");

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
