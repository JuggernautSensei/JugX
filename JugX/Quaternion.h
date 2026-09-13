#pragma once
#include <array>

#include "AxisAngle.h"
#include "Math.h"
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
        : v { _imaginary.e[0], _imaginary.e[1], _imaginary.e[2], _real }
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

        // shortest arc
        const float d = Dot(_dirFrom, _dirTo);
        if (Abs(d) > kAlmostOne)   // 거의 같은 방향
        {
            if (d >= 0)
            {
                return QUATERNION::kIdentity;   // vector are same
            }

            // 임의의 축을 선택하여 180도 회전
            const VECTOR3 axis = Cross(Abs(_dirFrom.x) <= Abs(_dirFrom.y) && Abs(_dirFrom.x) <= Abs(_dirFrom.z) ? Right<VECTOR3>() : Up<VECTOR3>(), _dirFrom);

            QUATERNION q;
            q.v.e[0] = axis.x;
            q.v.e[1] = axis.y;
            q.v.e[2] = axis.z;
            q.v.e[3] = 0.f;
            return q;   // NOLINT
        }
        else
        {
            const VECTOR3 c    = Cross(_dirFrom, _dirTo);
            const float   s    = Sqrt((1.f + d) * 2.f);
            const float   invS = 1.f / s;

            QUATERNION q;
            q.x = c.x * invS;
            q.y = c.y * invS;
            q.z = c.z * invS;
            q.w = s * 0.5f;
            return q;   // NOLINT
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromAxisAngle(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const float halfRad = _rad * 0.5f;
        const float s       = Sin(halfRad);
        const float c       = Cos(halfRad);
        return QUATERNION { _axis * s, c };
    }

    // 회전의 순서는 Y - X - Z
    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromEuler(
        const VECTOR3 _pyrRad)
    {
        const float hp = _pyrRad.e[0] * 0.5f;
        const float hy = _pyrRad.e[1] * 0.5f;
        const float hr = _pyrRad.e[2] * 0.5f;
        const float sp = Sin(hp), cp = Cos(hp);
        const float sy = Sin(hy), cy = Cos(hy);
        const float sr = Sin(hr), cr = Cos(hr);
        return QUATERNION {
            cy * sp * cr + sy * cp * sr,
            sy * cp * cr - cy * sp * sr,
            cy * cp * sr - sy * sp * cr,
            cy * cp * cr + sy * sp * sr
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromMatrix(
        const MATRIX& _mtx);

#ifdef JUG_SIMD_AVAILABLE
    // =======================================================
    //  Simd
    // =======================================================

    [[nodiscard]] simd::M128 ToSimd() const
    {
        return simd::LoadAligned(v.e.data());
    }

    [[nodiscard]] static QUATERNION MakeFromSimd(
        const simd::M128 _value)
    {
        QUATERNION q;
        simd::StoreAligned(q.v.e.data(), _value);
        return q;   // NOLINT
    }
#endif

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr float& operator[](
        const size_t _index)
    {
        return v[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr const float& operator[](
        const size_t _index) const
    {
        return v[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return v.e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return v.e.data();
    }

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator-() const
    {
        return QUATERNION { -v };
    }

    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator+(
        const QUATERNION _other) const
    {
        return QUATERNION { v + _other.v };
    }

    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator-(
        const QUATERNION _other) const
    {
        return QUATERNION { v - _other.v };
    }

    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
        const float _scalar) const
    {
        return QUATERNION { v * _scalar };
    }

    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator/(
        const float _scalar) const
    {
        const float inv = 1.f / _scalar;
        return *this * inv;
    }

    // 회전 합성. this 를 적용한 뒤 _other 를 적용한 회전을 돌려준다.
    // 수학적으로는 q = _other * this 이다.
    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
        const QUATERNION _other) const
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 q1  = ToSimd();
            const simd::M128 q2  = _other.ToSimd();
            simd::M128       ret = simd::Mul(simd::Splat<3>(q2), q1);
            ret                  = simd::MulAdd(simd::Splat<0>(q2), simd::Mul(simd::Shuffle<3, 2, 1, 0>(q1), simd::Set(1.f, -1.f, 1.f, -1.f)), ret);
            ret                  = simd::MulAdd(simd::Splat<1>(q2), simd::Mul(simd::Shuffle<2, 3, 0, 1>(q1), simd::Set(1.f, 1.f, -1.f, -1.f)), ret);
            ret                  = simd::MulAdd(simd::Splat<2>(q2), simd::Mul(simd::Shuffle<1, 0, 3, 2>(q1), simd::Set(-1.f, 1.f, 1.f, -1.f)), ret);
            return MakeFromSimd(ret);
        }
#endif
        const float x1 = v[0], y1 = v[1], z1 = v[2], w1 = v[3];
        const float x2 = _other.v[0], y2 = _other.v[1], z2 = _other.v[2], w2 = _other.v[3];
        return QUATERNION {
            w2 * x1 + x2 * w1 + y2 * z1 - z2 * y1,
            w2 * y1 - x2 * z1 + y2 * w1 + z2 * x1,
            w2 * z1 + x2 * y1 - y2 * x1 + z2 * w1,
            w2 * w1 - x2 * x1 - y2 * y1 - z2 * z1
        };
    }

    JUG_MATH_API constexpr QUATERNION& operator+=(
        const QUATERNION _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr QUATERNION& operator-=(
        const QUATERNION _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr QUATERNION& operator*=(
        const QUATERNION _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr QUATERNION& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const QUATERNION _other) const
    {
        return v == _other.v;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator!=(
        const QUATERNION _other) const
    {
        return v != _other.v;
    }

    // =======================================================
    //  Conversion
    // =======================================================

    JUG_MATH_API constexpr AXIS_ANGLE ToAxisAngle() const
    {
        JUG_ASSERT(IsNormalized(v), "QUATERNION must be normalized");

        // 예각을 선택
        const QUATERNION shortest = v[3] < 0.f ? -(*this) : *this;

        AXIS_ANGLE ret;
        ret.rad = 2.f * ACos(Clamp(shortest.v[3], -1.f, 1.f));

        // 정규화를 통해 ASin 연산 제거
        const VECTOR3 img   = { shortest.v[0], shortest.v[1], shortest.v[2] };
        const float   lenSq = LengthSq(img);
        if (IsZeroApprox(lenSq))   // 회전이 없으면 축이 정의되지 않는다. 임의의 단위축(+X)으로 대체
        {
            ret.axis = MathConstants<VECTOR3>::kUnitX;
            return ret;
        }

        ret.axis = img * RSqrt(lenSq);
        return ret;
    }

    JUG_MATH_API constexpr VECTOR3 ToEuler() const
    {
        JUG_ASSERT(IsNormalized(v), "QUATERNION must be normalized");

        const float qx = v[0], qy = v[1], qz = v[2], qw = v[3];
        const float m21      = 2.f * (qy * qz - qx * qw);
        const float sinPitch = Clamp(-m21, -1.f, 1.f);
        const float pitchRad = ASin(sinPitch);

        if (Abs(sinPitch) > kAlmostOne)
        {
            // 짐벌락 발생. roll은 0으로 고정하고 yaw만 계산
            const float m00    = 1.f - 2.f * (qy * qy + qz * qz);
            const float m02    = 2.f * (qx * qz - qy * qw);
            const float yawRad = ATan2(-m02, m00);
            return VECTOR3 { yawRad, pitchRad, 0.f };
        }

        const float m20     = 2.f * (qx * qz + qy * qw);
        const float m22     = 1.f - 2.f * (qx * qx + qy * qy);
        const float m01     = 2.f * (qx * qy + qz * qw);
        const float m11     = 1.f - 2.f * (qx * qx + qz * qz);
        const float yawRad  = ATan2(m20, m22);
        const float rollRad = ATan2(m01, m11);
        return VECTOR3 { yawRad, pitchRad, rollRad };
    }

    // =======================================================
    //  Fields
    // =======================================================

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

        struct
        {
            VECTOR3 imaginary;
            float   real;
        };

        VECTOR4         v;
        ARRAY<float, 4> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(QUATERNION) == 16, "QUATERNION must be tightly packed");
static_assert(alignof(QUATERNION) == 16, "QUATERNION must be 16-byte aligned for SIMD");
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
//  Basic
// ======================================================

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
    const float      _scalar,
    const QUATERNION _q)
{
    return _q * _scalar;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return IsEqualApprox(_x.v, _y.v);
}

// =======================================================
//  Vector operators
// =======================================================

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
    // 영 사원수 정규화는 정의되지 않는 연산이라 영 사원수를 그대로 리턴한다.
    return QUATERNION { Normalize(_q.v) };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const QUATERNION _q)
{
    return IsNormalized(_q.v);
}

// q 와 -q 는 같은 회전을 나타내므로 부호를 무시하고 비교한다.
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
    return QUATERNION { -_q.v[0], -_q.v[1], -_q.v[2], _q.v[3] };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION Inverse(
    const QUATERNION _q)
{
    const float lenSq = LengthSq(_q);
    if (IsZeroApprox(lenSq))   // 영 사원수에 대한 역원은 영 사원수로 정의
    {
        return Zero<QUATERNION>();
    }
    return Conjugate(_q) * (1.f / lenSq);
}

// =======================================================
//  Rotation
// =======================================================

// v' = v + 2w(q_v x v) + 2(q_v x (q_v x v))
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Mul(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");  
    const VECTOR3 img = { _q.v[0], _q.v[1], _q.v[2] };
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v + t * _q.v[3] + Cross(img, t);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 MulInverse(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    // Mul(_v, Inverse(_q)) 보다 효율적
    return Mul(_v, Conjugate(_q));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator*(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    return Mul(_v, _q);
}

// =======================================================
//  Interpolation
// =======================================================

// 선형 보간 후 정규화. Slerp 보다 빠르지만 각속도가 일정하지 않다.
[[nodiscard]] JUG_MATH_API constexpr QUATERNION Nlerp(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _t)
{
    // 최단 경로로 보간하도록 부호를 맞춘다.
    const QUATERNION end = Dot(_x, _y) < 0.f ? -_y : _y;
    return Normalize(_x + (end - _x) * _t);
}

// 구면 선형 보간. 각속도가 일정하다.
[[nodiscard]] JUG_MATH_API constexpr QUATERNION Slerp(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _t)
{
    float cosOmega = Dot(_x, _y);

    // 최단 경로 선택
    QUATERNION end = _y;
    if (cosOmega < 0.f)
    {
        cosOmega = -cosOmega;
        end      = -_y;
    }

    // 두 회전이 거의 같으면 sin(omega) 가 0 에 수렴해 나눗셈이 불안정해진다.
    if (cosOmega > kAlmostOne)
    {
        return Nlerp(_x, end, _t);
    }

    const float omega    = ACos(Clamp(cosOmega, -1.f, 1.f));
    const float sinOmega = Sin(omega);

    // 위 분기로 걸러지지만, 나눗셈 전에 한 번 더 확인해 0 으로 나누는 경로를 없앤다.
    if (IsZeroApprox(sinOmega))
    {
        return Nlerp(_x, end, _t);
    }

    const float invSinOmega = 1.f / sinOmega;
    const float scaleX      = Sin((1.f - _t) * omega) * invSinOmega;
    const float scaleY      = Sin(_t * omega) * invSinOmega;
    return _x * scaleX + end * scaleY;
}
}   // namespace jug

#include "Quaternion.inl"
