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

// VECTOR4 는 아래에 레지스터 전용 오버로드가 따로 있다 (요소를 쓰는 함수는 VECTOR4 로 인스턴스화되지 않는다).

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
    return _v * kDeg2Rad;
}

template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V ToDeg(
    const V _v)
{
    return _v * kRad2Deg;
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

// 영벡터의 정규화는 영벡터로 정의한다 (DirectXMath 와 같은 의미 - 길이가 정확히 0 일 때만).
template<FloatingPointVectorT V>
[[nodiscard]] JUG_MATH_API constexpr V Normalize(
    const V _v)
{
    const float lenSq = LengthSq(_v);
    if (lenSq == 0.f)
    {
        return Zero<V>();
    }

    const float len = Sqrt(lenSq);
    V           ret = _v;
    for (size_t i = 0; i < V::kDim; ++i)
    {
        ret[i] = _v[i] / len;
    }
    return ret;
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

// =======================================================
//  VECTOR3 (SIMD)
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 Normalize(
    const VECTOR3 _v)
{
    JUG_SIMD_PATH(VECTOR3::MakeFromM128(simd::Normalize3(_v.ToM128())));
    return Normalize<VECTOR3>(_v);
}

// =======================================================
//  VECTOR4
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Dot(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(simd::Dot4(_x.v, _y.v));
    return _x[0] * _y[0] + _x[1] * _y[1] + _x[2] * _y[2] + _x[3] * _y[3];
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL LengthSq(
    const VECTOR4 _v)
{
    return Dot(_v, _v);
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Length(
    const VECTOR4 _v)
{
    JUG_SIMD_PATH(simd::Length4(_v.v));
    return Sqrt(LengthSq(_v));
}

// 영벡터의 정규화는 영벡터 (DirectXMath 와 같은 의미)
[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Normalize(
    const VECTOR4 _v)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Normalize4(_v.v)));
    const float lenSq = LengthSq(_v);
    if (lenSq == 0.f)
    {
        return MathConstants<VECTOR4>::kZero;
    }
    const float len = Sqrt(lenSq);
    return VECTOR4 { _v[0] / len, _v[1] / len, _v[2] / len, _v[3] / len };
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL DistanceSq(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return LengthSq(_x - _y);
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Distance(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return Length(_x - _y);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Min(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Min(_x.v, _y.v)));
    return VECTOR4 { Min(_x[0], _y[0]), Min(_x[1], _y[1]), Min(_x[2], _y[2]), Min(_x[3], _y[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Max(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Max(_x.v, _y.v)));
    return VECTOR4 { Max(_x[0], _y[0]), Max(_x[1], _y[1]), Max(_x[2], _y[2]), Max(_x[3], _y[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Clamp(
    const VECTOR4 _v,
    const VECTOR4 _min,
    const VECTOR4 _max)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Clamp(_v.v, _min.v, _max.v)));
    return VECTOR4 { Clamp(_v[0], _min[0], _max[0]), Clamp(_v[1], _min[1], _max[1]), Clamp(_v[2], _min[2], _max[2]), Clamp(_v[3], _min[3], _max[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Saturate(
    const VECTOR4 _v)
{
    return Clamp(_v, MathConstants<VECTOR4>::kZero, MathConstants<VECTOR4>::kOne);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Abs(
    const VECTOR4 _v)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Abs(_v.v)));
    return VECTOR4 { Abs(_v[0]), Abs(_v[1]), Abs(_v[2]), Abs(_v[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Rcp(
    const VECTOR4 _v)
{
    return 1.f / _v;
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL RcpSafe(
    const VECTOR4 _v)
{
    return VECTOR4 { RcpSafe(_v[0]), RcpSafe(_v[1]), RcpSafe(_v[2]), RcpSafe(_v[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL Lerp(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _t)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Lerp(_x.v, _y.v, _t)));
    return _x + (_y - _x) * _t;
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsEqualApprox(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _epsilon = kEpsilon)
{
    JUG_SIMD_PATH(simd::AllTrue(simd::CmpLt(simd::Abs(simd::Sub(_x.v, _y.v)), simd::SetAll(_epsilon))));
    return IsEqualApprox(_x[0], _y[0], _epsilon) && IsEqualApprox(_x[1], _y[1], _epsilon) && IsEqualApprox(_x[2], _y[2], _epsilon) && IsEqualApprox(_x[3], _y[3], _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL IsZeroApprox(
    const VECTOR4 _v,
    const float   _epsilon = kEpsilon)
{
    JUG_SIMD_PATH(simd::AllTrue(simd::CmpLt(simd::Abs(_v.v), simd::SetAll(_epsilon))));
    return IsZeroApprox(_v[0], _epsilon) && IsZeroApprox(_v[1], _epsilon) && IsZeroApprox(_v[2], _epsilon) && IsZeroApprox(_v[3], _epsilon);
}

}   // namespace jug
