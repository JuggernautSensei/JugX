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
        : v { _imaginary[0], _imaginary[1], _imaginary[2], _real }
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
            const VECTOR3 axis = Normalize(Cross(Abs(_dirFrom.x) <= Abs(_dirFrom.y) ? Right<VECTOR3>() : Up<VECTOR3>(), _dirFrom));

            QUATERNION q;
            q[0] = axis.x;
            q[1] = axis.y;
            q[2] = axis.z;
            q[3] = 0.f;
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

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION MakeFromEuler(
        const VECTOR3 _pyrRad)
    {
        const float hp = _pyrRad[0] * 0.5f;
        const float hy = _pyrRad[1] * 0.5f;
        const float hr = _pyrRad[2] * 0.5f;
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
    [[nodiscard]] simd::M128 ToM128() const
    {
        return simd::LoadAligned(GetPtr());
    }

    [[nodiscard]] static QUATERNION MakeFromM128(
        const simd::M128 _value)
    {
        QUATERNION q;
        simd::StoreAligned(q.GetPtr(), _value);
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
        return v.GetPtr();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return v.GetPtr();
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

    // q1 * q2는 q1 회전 먼저 적용하고 q2 회전 적용. Hamilton 곱에 따르면 q2 ⊗ q1 임.
    [[nodiscard]] JUG_MATH_API constexpr QUATERNION operator*(
        const QUATERNION _other) const
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 q1  = ToM128();
            const simd::M128 q2  = _other.ToM128();
            simd::M128       ret = simd::Mult(simd::Splat<3>(q2), q1);
            ret                  = simd::MultAdd(simd::Splat<0>(q2), simd::Mult(simd::Shuffle<3, 2, 1, 0>(q1), simd::Set(1.f, -1.f, 1.f, -1.f)), ret);
            ret                  = simd::MultAdd(simd::Splat<1>(q2), simd::Mult(simd::Shuffle<2, 3, 0, 1>(q1), simd::Set(1.f, 1.f, -1.f, -1.f)), ret);
            ret                  = simd::MultAdd(simd::Splat<2>(q2), simd::Mult(simd::Shuffle<1, 0, 3, 2>(q1), simd::Set(-1.f, 1.f, 1.f, -1.f)), ret);
            return MakeFromM128(ret);
        }
#endif
        const float x1 = v[0], y1 = v[1], z1 = v[2], w1 = v[3];
        const float x2 = _other[0], y2 = _other[1], z2 = _other[2], w2 = _other[3];
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

    JUG_MATH_API constexpr QUATERNION& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
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
        ret.rad             = 2.f * ACos(Clamp(shortest[3], -1.f, 1.f));
        const VECTOR3 img   = { shortest[0], shortest[1], shortest[2] };
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

[[nodiscard]] JUG_MATH_API constexpr QUATERNION operator/(
    const float      _scalar,
    const QUATERNION _q)
{
    return QUATERNION { _scalar / _q.v };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _epsilon = kEpsilon)
{
    return IsEqualApprox(_x.v, _y.v, _epsilon);
}

// =======================================================
//  Vector
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
    return QUATERNION { Normalize(_q.v) };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const QUATERNION _q)
{
    return IsNormalized(_q.v);
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

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Rotate(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = { _q[0], _q[1], _q[2] };
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v + t * _q[3] + Cross(img, t);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 RotateInv(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = { _q[0], _q[1], _q[2] };
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
    float cosOmega = Dot(_x, _y);

    // 최단 경로 선택
    QUATERNION end = _y;
    if (cosOmega < 0.f)
    {
        cosOmega = -cosOmega;
        end      = -_y;
    }

    if (cosOmega > kAlmostOne)
    {
        return Nlerp(_x, end, _t);
    }

    const float omega    = ACos(Clamp(cosOmega, -1.f, 1.f));
    const float sinOmega = Sin(omega);
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
