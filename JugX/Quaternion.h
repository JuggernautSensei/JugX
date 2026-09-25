#pragma once
#include "AxisAngle.h"
#include "Vector.h"

namespace jug
{

struct MATRIX;

// 레지스터 타입 (VECTOR4 하나). 요소는 GetX()/SetX()/operator[] 로 접근한다.
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

    [[nodiscard]] JUG_MATH_API static constexpr QUATERNION JUG_VECTORCALL MakeFromTo(
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
                return QUATERNION { 0.f, 0.f, 0.f, 1.f };   // vector are same
            }

            // 임의의 축을 선택하여 180도 회전
            const VECTOR3 axis = Normalize(Cross(Abs(_dirFrom[0]) <= Abs(_dirFrom[1]) ? Right<VECTOR3>() : Up<VECTOR3>(), _dirFrom));
            return QUATERNION { axis, 0.f };
        }

        const VECTOR3 c    = Cross(_dirFrom, _dirTo);
        const float   s    = Sqrt((1.f + d) * 2.f);
        const float   invS = 1.f / s;
        return QUATERNION { c * invS, s * 0.5f };
    }

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION JUG_VECTORCALL MakeFromAxisAngle(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            simd::M128 s;
            simd::M128 c;
            simd::SinCos(simd::SetAll(_rad * 0.5f), s, c);
            return MakeFromM128(simd::Blend<0b1000>(simd::Mult(_axis.ToM128(), s), c));   // (axis * sin, cos)
        }
#endif
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad * 0.5f, s, c);
        return QUATERNION { _axis * s, c };
    }

    // pitch(x) -> yaw(y) -> roll(z) 순서의 오일러 각 (라디안)
    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION JUG_VECTORCALL MakeFromEuler(
        const VECTOR3 _pyrRad)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            // 세 반각의 sin/cos 를 한 번에 구한 뒤 (sp, cp, cp, cp) * (cy, sy, cy, cy) * (cr, cr, sr, cr)
            //                                    + (cp, -sp, -sp, sp) * (sy, cy, sy, sy) * (sr, sr, cr, sr)
            simd::M128 s;
            simd::M128 c;
            simd::SinCos(simd::Scale(_pyrRad.ToM128(), 0.5f), s, c);

            const simd::M128 p0 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(s, c));
            const simd::M128 y0 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(c, s));
            const simd::M128 r0 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(c, s));
            const simd::M128 p1 = simd::Shuffle<0, 2, 2, 2>(simd::Shuffle2<0, 0, 0, 0>(c, s));
            const simd::M128 y1 = simd::Shuffle<0, 2, 0, 0>(simd::Shuffle2<1, 1, 1, 1>(s, c));
            const simd::M128 r1 = simd::Shuffle<0, 0, 2, 0>(simd::Shuffle2<2, 2, 2, 2>(s, c));

            const simd::M128 q0 = simd::Mult(simd::Mult(p0, y0), r0);
            const simd::M128 q1 = simd::Mult(simd::Mult(p1, simd::Set(1.f, -1.f, -1.f, 1.f)), y1);
            return MakeFromM128(simd::MultAdd(q1, r1, q0));
        }
#endif
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

    [[nodiscard]] JUG_MATH_API constexpr static QUATERNION JUG_VECTORCALL MakeFromMatrix(
        MATRIX _mtx);

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static constexpr QUATERNION JUG_VECTORCALL MakeFromM128(
        const simd::M128 _value)
    {
        return QUATERNION { VECTOR4::MakeFromM128(_value) };
    }

    [[nodiscard]] constexpr simd::M128 ToM128() const
    {
        return v.ToM128();
    }
#endif

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr float GetX() const
    {
        return v.GetX();
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetY() const
    {
        return v.GetY();
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetZ() const
    {
        return v.GetZ();
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetW() const
    {
        return v.GetW();
    }

    JUG_MATH_API constexpr void SetX(
        const float _value)
    {
        v.SetX(_value);
    }

    JUG_MATH_API constexpr void SetY(
        const float _value)
    {
        v.SetY(_value);
    }

    JUG_MATH_API constexpr void SetZ(
        const float _value)
    {
        v.SetZ(_value);
    }

    JUG_MATH_API constexpr void SetW(
        const float _value)
    {
        v.SetW(_value);
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

    [[nodiscard]] JUG_MATH_API float* GetPtr()
    {
        return v.GetPtr();
    }

    [[nodiscard]] JUG_MATH_API const float* GetPtr() const
    {
        return v.GetPtr();
    }

    // =======================================================
    //  Compound Assignment (정의는 자유 연산자 뒤)
    // =======================================================

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

    VECTOR4 v;
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
//  Operators
// ======================================================

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator-(
    const QUATERNION _q)
{
    return QUATERNION { -_q.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator+(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return QUATERNION { _x.v + _y.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator-(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return QUATERNION { _x.v - _y.v };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator*(
    const QUATERNION _q,
    const float      _scalar)
{
    return QUATERNION { _q.v * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator*(
    const float      _scalar,
    const QUATERNION _q)
{
    return QUATERNION { _q.v * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator/(
    const QUATERNION _q,
    const float      _scalar)
{
    return QUATERNION { _q.v * (1.f / _scalar) };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator/(
    const float      _scalar,
    const QUATERNION _q)
{
    return QUATERNION { _scalar / _q.v };
}

// _x * _y 는 _x 회전을 먼저 적용하고 _y 회전을 적용. Hamilton 곱으로는 _y ⊗ _x (DirectXMath XMQuaternionMultiply 와 같음).
[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL operator*(
    const QUATERNION _x,
    const QUATERNION _y)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 q1  = _x.ToM128();
        const simd::M128 q2  = _y.ToM128();
        simd::M128       ret = simd::Mult(simd::Splat<3>(q2), q1);
        ret                  = simd::MultAdd(simd::Mult(simd::Splat<0>(q2), simd::Set(1.f, -1.f, 1.f, -1.f)), simd::Shuffle<3, 2, 1, 0>(q1), ret);
        ret                  = simd::MultAdd(simd::Mult(simd::Splat<1>(q2), simd::Set(1.f, 1.f, -1.f, -1.f)), simd::Shuffle<2, 3, 0, 1>(q1), ret);
        ret                  = simd::MultAdd(simd::Mult(simd::Splat<2>(q2), simd::Set(-1.f, 1.f, 1.f, -1.f)), simd::Shuffle<1, 0, 3, 2>(q1), ret);
        return QUATERNION::MakeFromM128(ret);
    }
#endif
    const float x1 = _x[0], y1 = _x[1], z1 = _x[2], w1 = _x[3];
    const float x2 = _y[0], y2 = _y[1], z2 = _y[2], w2 = _y[3];
    return QUATERNION {
        w2 * x1 + x2 * w1 + y2 * z1 - z2 * y1,
        w2 * y1 - x2 * z1 + y2 * w1 + z2 * x1,
        w2 * z1 + x2 * y1 - y2 * x1 + z2 * w1,
        w2 * w1 - x2 * x1 - y2 * y1 - z2 * z1
    };
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator==(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return _x.v == _y.v;
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator!=(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return _x.v != _y.v;
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

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsEqualApprox(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _epsilon = kEpsilon)
{
    return IsEqualApprox(_x.v, _y.v, _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Dot(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return Dot(_x.v, _y.v);
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL LengthSq(
    const QUATERNION _q)
{
    return LengthSq(_q.v);
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Length(
    const QUATERNION _q)
{
    return Length(_q.v);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL Normalize(
    const QUATERNION _q)
{
    return QUATERNION { Normalize(_q.v) };
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsNormalized(
    const QUATERNION _q)
{
    return IsEqualApprox(LengthSq(_q), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsSameRotationApprox(
    const QUATERNION _x,
    const QUATERNION _y)
{
    JUG_ASSERT(IsNormalized(_x) && IsNormalized(_y), "QUATERNION must be normalized");
    return IsEqualApprox(Abs(Dot(_x, _y)), 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL Conjugate(
    const QUATERNION _q)
{
    JUG_SIMD_PATH(QUATERNION::MakeFromM128(simd::Xor(_q.ToM128(), simd::Set(-0.f, -0.f, -0.f, 0.f))));
    return QUATERNION { -_q[0], -_q[1], -_q[2], _q[3] };
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL Inverse(
    const QUATERNION _q)
{
    const float lenSq = LengthSq(_q);
    if (IsZeroApprox(lenSq))   // 영 사원수에 대한 역원은 영 사원수로 정의
    {
        return MathConstants<QUATERNION>::kZero;
    }
    return Conjugate(_q) * (1.f / lenSq);
}

// =======================================================
//  Vector
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 JUG_VECTORCALL Rotate(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = _q.GetImaginary();
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v + t * _q.GetW() + Cross(img, t);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 JUG_VECTORCALL RotateInv(
    const VECTOR3    _v,
    const QUATERNION _q)
{
    JUG_ASSERT(IsNormalized(_q), "QUATERNION must be normalized");
    const VECTOR3 img = _q.GetImaginary();
    const VECTOR3 t   = Cross(img, _v) * 2.f;
    return _v - t * _q.GetW() + Cross(img, t);
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL Nlerp(
    const QUATERNION _x,
    const QUATERNION _y,
    const float      _t)
{
    const QUATERNION end = Dot(_x, _y) < 0.f ? -_y : _y;
    return Normalize(_x + (end - _x) * _t);
}

[[nodiscard]] JUG_MATH_API constexpr QUATERNION JUG_VECTORCALL Slerp(
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

// =======================================================
//  Conversion
// =======================================================

JUG_MATH_API constexpr AXIS_ANGLE QUATERNION::ToAxisAngle() const
{
    JUG_ASSERT(IsNormalized(*this), "QUATERNION must be normalized");

    // 예각을 선택
    const QUATERNION shortest = GetW() < 0.f ? -(*this) : *this;

    AXIS_ANGLE ret;
    ret.rad             = 2.f * ACos(Clamp(shortest.GetW(), -1.f, 1.f));
    const VECTOR3 img   = shortest.GetImaginary();
    const float   lenSq = LengthSq(img);
    if (IsZeroApprox(lenSq))   // 회전이 없으면 축이 정의되지 않는다. 임의의 단위축(+X)으로 대체
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

    const float qx = GetX(), qy = GetY(), qz = GetZ(), qw = GetW();
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

}   // namespace jug

#include "Quaternion.inl"
