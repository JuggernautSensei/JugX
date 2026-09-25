#pragma once
#include "Vector4.h"

namespace jug
{

// =======================================================
//  Type Traits
// =======================================================

template<typename V>
struct VectorTraits
{
    static constexpr bool kIsVector = false;
};

template<typename T, size_t N>
struct VectorTraits<VECTOR<T, N>>
{
    static constexpr bool kIsVector = true;

    using Scalar                 = T;
    static constexpr size_t kDim = N;
};

template<typename V>
concept VectorT = VectorTraits<V>::kIsVector;

template<typename V>
concept FloatingPointVectorT = VectorT<V> && std::floating_point<typename VectorTraits<V>::Scalar>;


// =======================================================
//  Method
// =======================================================

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator*(
    const typename VectorTraits<V>::Scalar _scalar,
    const V                                _v)
{
    return _v * _scalar;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr V operator/(
    const typename VectorTraits<V>::Scalar _scalar,
    const V                                _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = _scalar / _v[i];
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(
    const V     _v,
    const float _epsilon = kEpsilon)
{
    for (size_t i = 0; i < V::kDim; ++i)
    {
        if (!IsZeroApprox(_v[i], _epsilon))
        {
            return false;
        }
    }
    return true;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const V     _x,
    const V     _y,
    const float _epsilon = kEpsilon)
{
    for (size_t i = 0; i < V::kDim; ++i)
    {
        if (!IsEqualApprox(_x[i], _y[i], _epsilon))
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
        ret[i] = Clamp(_v[i], _min[i], _max[i]);
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
        ret[i] = Abs(_v[i]);
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
        ret[i] = Max(_x[i], _y[i]);
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
        ret[i] = Min(_x[i], _y[i]);
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Rcp(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = 1.f / _v[i];
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V RcpSafe(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = RcpSafe(_v[i]);
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V ToRad(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = ToRad(_v[i]);
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V ToDeg(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = ToDeg(_v[i]);
    }
    return ret;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Saturate(
    const V _v)
{
    V ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = Saturate(_v[i]);
    }
    return ret;
}

template<VectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Dot(
    const V _x,
    const V _y)
{
    float ret = _x[0] * _y[0];
    for (size_t i = 1; i < V::kDim; ++i)
    {
        ret += _x[i] * _y[i];
    }
    return ret;
}

[[nodiscard]] JUG_MATH_API constexpr float Cross(
    const VECTOR2 _x,
    const VECTOR2 _y)
{
    return _x[0] * _y[1] - _x[1] * _y[0];
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Cross(
    const VECTOR3 _x,
    const VECTOR3 _y)
{
    return {
        _x[1] * _y[2] - _x[2] * _y[1],
        _x[2] * _y[0] - _x[0] * _y[2],
        _x[0] * _y[1] - _x[1] * _y[0]
    };
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float LengthSq(
    const V _v)
{
    return Dot(_v, _v);
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Length(
    const V _v)
{
    return Sqrt(LengthSq(_v));
}

template<FloatingPointVectorT V>
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

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsNormalized(
    const V _v)
{
    return IsEqualApprox(LengthSq(_v), 1.f);
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr bool IsOrthogonal(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Dot(_x, _y));
}

template<FloatingPointVectorT V>
    requires(V::kDim < 4)
[[nodiscard]] JUG_MATH_API constexpr bool IsParallel(
    const V _x,
    const V _y)
{
    return IsZeroApprox(Cross(_x, _y));
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float DistanceSq(
    const V _x,
    const V _y)
{
    return LengthSq(_x - _y);
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const V _x,
    const V _y)
{
    return Length(_x - _y);
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Lerp(
    const V     _x,
    const V     _y,
    const float _t)
{
    return _x + (_y - _x) * _t;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr float Angle(
    const V _x,
    const V _y)
{
    return ACos(Clamp(Dot(Normalize(_x), Normalize(_y)), -1.f, 1.f));
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Reflect(
    const V _v,
    const V _normal)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    return _v - 2.f * Dot(_v, _normal) * _normal;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Refract(
    const V     _v,
    const V     _normal,
    const float _eta)
{
    JUG_ASSERT(IsNormalized(_normal), "Normal must be normalized");
    const float dot  = -Dot(_v, _normal);
    const V     perp = (_v + dot * _normal) * _eta;
    const float k    = 1.f - LengthSq(perp);
    if (k < 0.f)   // 전반사. HLSL refract() 와 같이 영벡터로 정의한다.
    {
        return Zero<V>();
    }
    const V para = _normal * -Sqrt(k);
    return perp + para;
}

template<FloatingPointVectorT V>
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

[[nodiscard]] constexpr VECTOR4 operator/(
    const float   _scalar,
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return VECTOR4::MakeFromM128(simd::Div(simd::SetAll(_scalar), _v.ToM128()));
    }
    return operator/ <VECTOR4>(_scalar, _v);
}

[[nodiscard]] constexpr VECTOR3 Normalize(
    const VECTOR3 _v)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 value = _v.ToM128();
        const simd::M128 lenSq = simd::Dot4V(value, value);
        const simd::M128 valid = simd::CmpGe(lenSq, simd::SetAll(kEpsilon));
        return VECTOR3::MakeFromM128(simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid));
    }
    return Normalize<VECTOR3>(_v);
}

[[nodiscard]] constexpr float Dot(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return simd::Dot4(_x.ToM128(), _y.ToM128());
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
        return simd::Length4(_v.ToM128());
    }
    return Sqrt(LengthSq<VECTOR4>(_v));
}

[[nodiscard]] constexpr VECTOR4 Normalize(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 value = _v.ToM128();
        const simd::M128 lenSq = simd::Dot4V(value, value);
        const simd::M128 valid = simd::CmpGe(lenSq, simd::SetAll(kEpsilon));
        return VECTOR4::MakeFromM128(simd::And(simd::Div(value, simd::Sqrt(lenSq)), valid));
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
        return VECTOR4::MakeFromM128(simd::Min(_x.ToM128(), _y.ToM128()));
    }
    return Min<VECTOR4>(_x, _y);
}

[[nodiscard]] constexpr VECTOR4 Max(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    if (!std::is_constant_evaluated())
    {
        return VECTOR4::MakeFromM128(simd::Max(_x.ToM128(), _y.ToM128()));
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
        return VECTOR4::MakeFromM128(simd::Clamp(_v.ToM128(), _min.ToM128(), _max.ToM128()));
    }
    return Clamp<VECTOR4>(_v, _min, _max);
}

[[nodiscard]] constexpr VECTOR4 Abs(
    const VECTOR4 _v)
{
    if (!std::is_constant_evaluated())
    {
        return VECTOR4::MakeFromM128(simd::Abs(_v.ToM128()));
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
        return VECTOR4::MakeFromM128(simd::Lerp(_x.ToM128(), _y.ToM128(), _t));
    }
    return Lerp<VECTOR4>(_x, _y, _t);
}

[[nodiscard]] constexpr bool IsEqualApprox(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _epsilon = kEpsilon)
{
    if (!std::is_constant_evaluated())
    {
        const simd::M128 diff = simd::Abs(simd::Sub(_x.ToM128(), _y.ToM128()));
        return simd::AllTrue(simd::CmpLt(diff, simd::SetAll(_epsilon)));
    }
    return IsEqualApprox<VECTOR4>(_x, _y, _epsilon);
}

[[nodiscard]] constexpr bool IsZeroApprox(
    const VECTOR4 _v,
    const float   _epsilon = kEpsilon)
{
    if (!std::is_constant_evaluated())
    {
        return simd::AllTrue(simd::CmpLt(simd::Abs(_v.ToM128()), simd::SetAll(_epsilon)));
    }
    return IsZeroApprox<VECTOR4>(_v, _epsilon);
}

#endif   // JUG_SIMD_AVAILABLE

}   // namespace jug