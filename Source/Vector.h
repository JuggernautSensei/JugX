#pragma once
#include "Config.h"  
#include "Math.h"
#include "SIMD.h"
//
#include "MathMacros.h"
#include "Vector2.h"   
#include "Vector2I.h"  
#include "Vector3.h"   
#include "Vector3I.h"  
#include "Vector4.h"   
#include "Vector4I.h"  

namespace jug
{

// =======================================================
//  Type Traits
// =======================================================

template<typename TVector>
concept VectorT = std::is_same_v<TVector, VECTOR2> || std::is_same_v<TVector, VECTOR3> || std::is_same_v<TVector, VECTOR4>
               || std::is_same_v<TVector, VECTOR2I> || std::is_same_v<TVector, VECTOR3I> || std::is_same_v<TVector, VECTOR4I>;

template<typename TVector>
concept FloatVectorT = VectorT<TVector> && std::is_floating_point_v<typename TVector::ValueT>;

// =======================================================
//  Basic
// =======================================================

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector operator*(
    const typename TVector::ValueT _scalar,
    const TVector                  _v)
{
    return _v * _scalar;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector operator/(
    const typename TVector::ValueT _scalar,
    TVector                        _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = _scalar / _v.e[i];
    }
    return _v;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(
    const TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        if (!IsZeroApprox(_v.e[i]))
        {
            return false;
        }
    }
    return true;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const TVector _x,
    const TVector _y)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        if (!IsEqualApprox(_x.e[i], _y.e[i]))
        {
            return false;
        }
    }
    return true;
}

// IsZeroApprox 는 모든 성분이 0 에 가까워야 참이지만, 이건 성분 중 하나라도 0 에 가까우면 참이다.
template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsAnyApproxZero(
    const TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        if (IsZeroApprox(_v.e[i]))
        {
            return true;
        }
    }
    return false;
}

// IsEqualApprox 는 모든 성분이 같아야 참이지만, 이건 성분 중 하나라도 같으면 참이다.
template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsAnyApproxEqual(
    const TVector _x,
    const TVector _y)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        if (IsEqualApprox(_x.e[i], _y.e[i]))
        {
            return true;
        }
    }
    return false;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Clamp(
    TVector       _v,
    const TVector _min,
    const TVector _max)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = Clamp(_v.e[i], _min.e[i], _max.e[i]);
    }
    return _v;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Abs(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = Abs(_v.e[i]);
    }
    return _v;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Max(
    TVector       _x,
    const TVector _y)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _x.e[i] = Max(_x.e[i], _y.e[i]);
    }
    return _x;
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Min(
    TVector       _x,
    const TVector _y)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _x.e[i] = Min(_x.e[i], _y.e[i]);
    }
    return _x;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Rcp(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = 1.f / _v.e[i];
    }
    return _v;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector RcpSafe(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = RcpSafe(_v.e[i]);
    }
    return _v;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector ToRad(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = ToRad(_v.e[i]);
    }
    return _v;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector ToDeg(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = ToDeg(_v.e[i]);
    }
    return _v;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Saturate(
    TVector _v)
{
    for (size_t i = 0; i < TVector::kDim; ++i)
    {
        _v.e[i] = Saturate(_v.e[i]);
    }
    return _v;
}

// =======================================================
//  Vector operators
// =======================================================

template<VectorT TVector>
    requires(TVector::kDim >= 2)
[[nodiscard]] JUG_MATH_API constexpr typename TVector::ValueT Dot(
    const TVector _x,
    const TVector _y)
{
    typename TVector::ValueT ret = _x.e[0] * _y.e[0];
    for (size_t i = 1; i < TVector::kDim; ++i)
    {
        ret += _x.e[i] * _y.e[i];
    }
    return ret;
}

template<VectorT TVector>
    requires(TVector::kDim == 2)
[[nodiscard]] JUG_MATH_API constexpr typename TVector::ValueT Cross(
    const TVector _x,
    const TVector _y)
{
    return _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0];
}

template<VectorT TVector>
    requires(TVector::kDim == 3)
[[nodiscard]] JUG_MATH_API constexpr TVector Cross(
    const TVector _x,
    const TVector _y)
{
    return {
        _x.e[1] * _y.e[2] - _x.e[2] * _y.e[1],
        _x.e[2] * _y.e[0] - _x.e[0] * _y.e[2],
        _x.e[0] * _y.e[1] - _x.e[1] * _y.e[0]
    };
}

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr typename TVector::ValueT LengthSq(
    const TVector _v)
{
    return Dot(_v, _v);
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr float Length(
    const TVector _v)
{
    return Sqrt(LengthSq(_v));
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Normalize(
    const TVector _v)
{
    const float lenSq = LengthSq(_v);
    if (IsZeroApprox(lenSq))   // fallback. 영벡터에 대한 정규화는 영벡터로 정의한다.
    {
        return Zero<TVector>();
    }
    return _v * RSqrt(lenSq);
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const TVector _v)
{
    return IsEqualApprox(LengthSq(_v), 1.f);
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr bool IsOrthogonal(
    const TVector _x,
    const TVector _y)
{
    return IsZeroApprox(Dot(_x, _y));
}

template<FloatVectorT TVector>
    requires(TVector::kDim == 2 || TVector::kDim == 3)
[[nodiscard]] JUG_MATH_API constexpr bool IsParallel(
    const TVector _x,
    const TVector _y)
{
    return IsZeroApprox(Cross(_x, _y));
}

// =======================================================
//  Distance
// =======================================================

template<VectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr typename TVector::ValueT DistanceSq(
    const TVector _x,
    const TVector _y)
{
    return LengthSq(_x - _y);
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const TVector _x,
    const TVector _y)
{
    return Length(_x - _y);
}

// =======================================================
//  Interpolation
// =======================================================

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Lerp(
    const TVector _x,
    const TVector _y,
    const float   _t)
{
    return _x + (_y - _x) * _t;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr float Angle(
    const TVector _x,
    const TVector _y)
{
    return ACos(Clamp(Dot(Normalize(_x), Normalize(_y)), -1.f, 1.f));
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Reflect(
    const TVector _v,
    const TVector _normal)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    return _v - 2.f * Dot(_v, _normal) * _normal;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Refract(
    const TVector _v,
    const TVector _normal,
    const float   _eta)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    const float   dot  = -Dot(_v, _normal);
    const TVector perp = (_v + dot * _normal) * _eta;
    const TVector para = _normal * -Sqrt(Abs(1.f - LengthSq(perp)));
    return perp + para;
}

template<FloatVectorT TVector>
[[nodiscard]] JUG_MATH_API constexpr TVector Project(
    const TVector _v,
    const TVector _onto)
{
    const float lenSq = LengthSq(_onto);
    if (IsZeroApprox(lenSq))   // fallback. 영벡터로의 투영은 영벡터로 정의한다.
    {
        return Zero<TVector>();
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
