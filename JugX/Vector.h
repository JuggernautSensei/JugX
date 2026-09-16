#pragma once
#include "Vector4.h"
#include "Vector4I.h"

namespace jug
{

// =======================================================
//  Type Traits
// =======================================================

template<typename T>
concept VectorT = AnyTypeOfV<T, VECTOR2, VECTOR3, VECTOR4, VECTOR2I, VECTOR3I, VECTOR4I>;

template<typename T>
concept IntegerVectorT = VectorT<T> && std::is_integral_v<typename T::ValueT>;

template<typename T>
concept FloatingVectorT = VectorT<T> && std::is_floating_point_v<typename T::ValueT>;

// =======================================================
//  Basic
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator*(
    const typename V::ValueT _scalar,
    const V                  _v)
{
    return _v * _scalar;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator/(
    const typename V::ValueT _scalar,
    const V                  _v)
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

template<FloatingVectorT V>
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

template<FloatingVectorT V>
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

template<FloatingVectorT V>
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

template<FloatingVectorT V>
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

template<FloatingVectorT V>
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
//  V operators
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr typename V::ValueT Dot(
    const V _x,
    const V _y)
{
    typename V::ValueT ret = _x.e[0] * _y.e[0];
    for (size_t i = 1; i < V::kDim; ++i)
    {
        ret += _x.e[i] * _y.e[i];
    }
    return ret;
}

template<VectorT V>
    requires(V::kDim == 2)
[[nodiscard]] JUG_MATH_API constexpr typename V::ValueT Cross(
    const V _x,
    const V _y)
{
    return _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0];
}

template<VectorT V>
    requires(V::kDim == 3)
[[nodiscard]] JUG_MATH_API constexpr V Cross(
    const V _x,
    const V _y)
{
    return {
        _x.e[1] * _y.e[2] - _x.e[2] * _y.e[1],
        _x.e[2] * _y.e[0] - _x.e[0] * _y.e[2],
        _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0]
    };
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr typename V::ValueT LengthSq(
    const V _v)
{
    return Dot(_v, _v);
}

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Length(
    const V _v)
{
    return Sqrt(LengthSq(_v));
}

template<FloatingVectorT V>
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

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const V _v)
{
    return IsEqualApprox(LengthSq(_v), 1.f);
}

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsOrthogonal(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Dot(_x, _y));
}

template<FloatingVectorT V>
    requires(V::kDim == 2 || V::kDim == 3)
[[nodiscard]] JUG_MATH_API constexpr bool IsParallel(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Cross(_x, _y));
}

// =======================================================
//  Distance
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr typename V::ValueT DistanceSq(
    const V _x,
    const V _y)
{
    return LengthSq(_x - _y);
}

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const V _x,
    const V _y)
{
    return Length(_x - _y);
}

// =======================================================
//  Interpolation
// =======================================================

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Lerp(
    const V     _x,
    const V     _y,
    const float _t)
{
    return _x + (_y - _x) * _t;
}

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Angle(
    const V _x,
    const V _y)
{
    return ACos(Clamp(Dot(Normalize(_x), Normalize(_y)), -1.f, 1.f));
}

template<FloatingVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Reflect(
    const V _v,
    const V _normal)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    return _v - 2.f * Dot(_v, _normal) * _normal;
}

template<FloatingVectorT V>
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

template<FloatingVectorT V>
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
//  VECTOR3 SIMD optimization
// =======================================================

[[nodiscard]] constexpr VECTOR3 Normalize(
    const VECTOR3 _v)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 value = _v.ToSIMD();
        const simd::M128 lenSq = simd::Dot4V(value, value);
        const simd::M128 valid = simd::CmpGe(lenSq, simd::SetAll(kEpsilon));
        return VECTOR3::MakeFromSIMD(simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid));
    }
    return Normalize<VECTOR3>(_v);
}

// =======================================================
//  VECTOR4 SIMD optimization
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
        return VECTOR4::MakeFromSIMD(simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid));
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
        return VECTOR4::MakeFromSIMD(simd::Min(_x.ToSIMD(), _y.ToSIMD()));
    }
    return Min<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr VECTOR4 Max(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return VECTOR4::MakeFromSIMD(simd::Max(_x.ToSIMD(), _y.ToSIMD()));
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
        return VECTOR4::MakeFromSIMD(simd::Clamp(_v.ToSIMD(), _min.ToSIMD(), _max.ToSIMD()));
    }
    return Clamp<VECTOR4>(_v, _min, _max);
}

[[nodiscard]] constexpr VECTOR4 Abs(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return VECTOR4::MakeFromSIMD(simd::Abs(_v.ToSIMD()));
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
        return VECTOR4::MakeFromSIMD(simd::Lerp(_x.ToSIMD(), _y.ToSIMD(), _t));
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
