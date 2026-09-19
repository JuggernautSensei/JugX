#pragma once
#include "Vector4.h"

namespace jug
{

// =======================================================
//  Type Traits
// =======================================================

template<typename T>
concept VectorT = AnyTypeOfV<T, VECTOR2, VECTOR3, VECTOR4>;

// =======================================================
//  Basic
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator*(
    const float _scalar,
    const V     _v)
{
    return _v * _scalar;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator/(
    const float _scalar,
    const V     _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = _scalar / _v.e[i];
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(
    const V _v)
{
    for (size_t i = 0; i < V::kDim; ++i)
    {
        if (!IsZeroApprox(_v.e[i]))
        {
            return false;
        }
    }
    return true;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const V _x,
    const V _y)
{
    for (size_t i = 0; i < V::kDim; ++i)
    {
        if (!IsEqualApprox(_x.e[i], _y.e[i]))
        {
            return false;
        }
    }
    return true;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Clamp(
    const V _v,
    const V _min,
    const V _max)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = Clamp(_v.e[i], _min.e[i], _max.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Abs(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = Abs(_v.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Max(
    const V _x,
    const V _y)
{
    V ret = _x;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = Max(_x.e[i], _y.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Min(
    const V _x,
    const V _y)
{
    V ret = _x;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = Min(_x.e[i], _y.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Rcp(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = 1.f / _v.e[i];
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V RcpSafe(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = RcpSafe(_v.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V ToRad(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = ToRad(_v.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V ToDeg(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = ToDeg(_v.e[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Saturate(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret.e[i] = Saturate(_v.e[i]);
    }
    return ret;
}

// =======================================================
//  Operators
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Dot(
    const V _x,
    const V _y)
{
    float ret = _x.e[0] * _y.e[0];
    for (size_t i = 1; i < V::kDim; ++i)
    {
        ret += _x.e[i] * _y.e[i];
    }
    return ret;
}

[[nodiscard]] JUG_MATH_API constexpr float Cross(
    const VECTOR2 _x,
    const VECTOR2 _y)
{
    return _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0];
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Cross(
    const VECTOR3 _x,
    const VECTOR3 _y)
{
    return {
        _x.e[1] * _y.e[2] - _x.e[2] * _y.e[1],
        _x.e[2] * _y.e[0] - _x.e[0] * _y.e[2],
        _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0]
    };
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float LengthSq(
    const V _v)
{
    return Dot(_v, _v);
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Length(
    const V _v)
{
    return Sqrt(LengthSq(_v));
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Normalize(
    const V _v)
{
    const float lenSq = LengthSq(_v);
    if (IsZeroApprox(lenSq))   // fallback. 영벡터에 대한 정규화는 영벡터로 정의한다.
    {
        return Zero<V>();
    }
    return _v * RSqrt(lenSq);
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const V _v)
{
    return IsEqualApprox(LengthSq(_v), 1.f);
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsOrthogonal(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Dot(_x, _y));
}

template<VectorT V>
    requires(V::kDim < 4)
[[nodiscard]] JUG_MATH_API constexpr bool IsParallel(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Cross(_x, _y));
}   // namespace jug

// =======================================================
//  Distance
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float DistanceSq(
    const V _x,
    const V _y)
{
    return LengthSq(_x - _y);
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const V _x,
    const V _y)
{
    return Length(_x - _y);
}

// =======================================================
//  Interpolation
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Lerp(
    const V     _x,
    const V     _y,
    const float _t)
{
    return _x + (_y - _x) * _t;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Angle(
    const V _x,
    const V _y)
{
    return ACos(Clamp(Dot(Normalize(_x), Normalize(_y)), -1.f, 1.f));
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Reflect(
    const V _v,
    const V _normal)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    return _v - 2.f * Dot(_v, _normal) * _normal;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Refract(
    const V     _v,
    const V     _normal,
    const float _eta)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    const float dot  = -Dot(_v, _normal);
    const V     perp = (_v + dot * _normal) * _eta;
    const V     para = _normal * -Sqrt(Abs(1.f - LengthSq(perp)));
    return perp + para;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Project(
    const V _v,
    const V _onto)
{
    const float lenSq = LengthSq(_onto);
    if (IsZeroApprox(lenSq))   // fallback. 영벡터로의 투영은 영벡터로 정의한다.
    {
        return Zero<V>();
    }
    return _onto * (Dot(_v, _onto) * RcpSafe(lenSq));
}

#ifdef JUG_SIMD_AVAILABLE

// =======================================================
//  VECTOR3
// =======================================================

[[nodiscard]] constexpr VECTOR3 Normalize(
    const VECTOR3 _v)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 value = simd::LoadAligned(_v.e.data());
        const simd::M128 lenSq = simd::Dot4V(value, value);
        const simd::M128 valid = simd::CmpGe(lenSq, simd::SetAll(kEpsilon));
        return simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid);
    }
    return Normalize<VECTOR3>(_v);
}

// =======================================================
//  VECTOR4
// =======================================================

[[nodiscard]] constexpr float Dot(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Dot4(_x.ToSIMD(), _y.ToSIMD());
    }
    return Dot<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr float LengthSq(
    const VECTOR4 _v)
{
    return Dot(_v, _v);
}

[[nodiscard]] constexpr float Length(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Length4(_v.ToSIMD());
    }
    return Sqrt(LengthSq<VECTOR4>(_v));
}

[[nodiscard]] constexpr VECTOR4 Normalize(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 value = _v.ToSIMD();
        const simd::M128 lenSq = simd::Dot4V(value, value);
        const simd::M128 valid = simd::CmpGe(lenSq, simd::SetAll(kEpsilon));
        return simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid);
    }
    return Normalize<VECTOR4>(_v);
}

[[nodiscard]] constexpr float DistanceSq(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return LengthSq(_x - _y);
}

[[nodiscard]] constexpr float Distance(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return Length(_x - _y);
}

[[nodiscard]] constexpr VECTOR4 Min(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Min(_x.ToSIMD(), _y.ToSIMD());
    }
    return Min<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr VECTOR4 Max(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Max(_x.ToSIMD(), _y.ToSIMD());
    }
    return Max<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr VECTOR4 Clamp(
    const VECTOR4 _v,
    const VECTOR4 _min,
    const VECTOR4 _max)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Clamp(_v.ToSIMD(), _min.ToSIMD(), _max.ToSIMD());
    }
    return Clamp<VECTOR4>(_v, _min, _max);
}

[[nodiscard]] constexpr VECTOR4 Abs(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Abs(_v.ToSIMD());
    }
    return Abs<VECTOR4>(_v);
}

[[nodiscard]] constexpr VECTOR4 Lerp(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _t)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Lerp(_x.ToSIMD(), _y.ToSIMD(), _t);
    }
    return Lerp<VECTOR4>(_x, _y, _t);
}

[[nodiscard]] constexpr bool IsEqualApprox(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 diff = simd::Abs(simd::Sub(_x.ToSIMD(), _y.ToSIMD()));
        return simd::AllTrue(simd::CmpLt(diff, simd::SetAll(kEpsilon)));
    }
    return IsEqualApprox<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr bool IsZeroApprox(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return simd::AllTrue(simd::CmpLt(simd::Abs(_v.ToSIMD()), simd::SetAll(kEpsilon)));
    }
    return IsZeroApprox<VECTOR4>(_v);
}

#endif   // JUG_MATH_SIMD

}   // namespace jug
