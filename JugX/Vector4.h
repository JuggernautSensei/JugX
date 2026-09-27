#pragma once
#include "Vector3.h"

namespace jug
{

template<VectorScalarT T>
struct VECTOR<T, 4>
{
    JUG_MATH_API VECTOR() = default;

    JUG_MATH_API constexpr VECTOR(
        const T _x,
        const T _y,
        const T _z,
        const T _w)
        : e { _x, _y, _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR(
        const VECTOR<T, 2> _xy,
        const T            _z,
        const T            _w)
        : e { _xy[0], _xy[1], _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR(
        const VECTOR<T, 3> _xyz,
        const T            _w)
        : e { _xyz[0], _xyz[1], _xyz[2], _w }
    {
    }

    explicit JUG_MATH_API constexpr VECTOR(
        const T _value)
        : e { _value, _value, _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr static VECTOR JUG_VECTORCALL FromM128(
        const simd::M128 _v)
        requires std::is_same_v<T, float>
    {
        if (std::is_constant_evaluated())
        {
            return simd::NoInlineConstruct<VECTOR>(simd::Get<0>(_v), simd::Get<1>(_v), simd::Get<2>(_v), simd::Get<3>(_v));
        }
        return std::bit_cast<VECTOR>(_v);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-() const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Negate(ToM128()));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] = -ret[i];
            }
            return ret;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator+(
        const VECTOR _other) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Add(ToM128(), _other.ToM128()));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] += _other[i];
            }
            return ret;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-(
        const VECTOR _other) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Sub(ToM128(), _other.ToM128()));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] -= _other[i];
            }
            return ret;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator*(
        const T _scalar) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Mult(ToM128(), simd::SetAll(_scalar)));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] *= _scalar;
            }
            return ret;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const T _scalar) const
        requires std::is_integral_v<T>
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] /= _scalar;
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const T _scalar) const
        requires std::is_floating_point_v<T>
    {
        return *this * (1.f / _scalar);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator*(
        const VECTOR _other) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Mult(ToM128(), _other.ToM128()));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] *= _other[i];
            }
            return ret;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const VECTOR _other) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return FromM128(simd::Div(ToM128(), _other.ToM128()));
        }
        else
        {
            VECTOR ret = *this;
            for (size_t i = 0; i < kDim; ++i)
            {
                ret[i] /= _other[i];
            }
            return ret;
        }
    }

    JUG_MATH_API constexpr VECTOR& operator+=(
        const VECTOR _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR& operator-=(
        const VECTOR _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR& operator*=(
        const T _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR& operator/=(
        const T _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR& operator*=(
        const VECTOR _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR& operator/=(
        const VECTOR _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR _other) const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return simd::MoveMask(simd::CmpEq(ToM128(), _other.ToM128())) == 0xf;
        }
        else
        {
            for (size_t i = 0; i < kDim; ++i)
            {
                if (e[i] != _other[i])   // NOLINT
                {
                    return false;
                }
            }
            return true;
        }
    }

    [[nodiscard]] JUG_MATH_API constexpr T& operator[](
        const size_t _index)
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr const T& operator[](
        const size_t _index) const
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr T* GetPtr()
    {
        return e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr const T* GetPtr() const
    {
        return e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR<T, 2> ToVector2() const
    {
        return VECTOR<T, 2> { e[0], e[1] };
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR<T, 3> ToVector3() const
    {
        return VECTOR<T, 3> { e[0], e[1], e[2] };
    }


     [[nodiscard]] JUG_MATH_API constexpr simd::M128 JUG_VECTORCALL ToM128() const
        requires std::is_same_v<T, float>
    {
        return simd::Load(e.data());
    }

    constexpr static size_t kDim = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
        ARRAY<T, 4> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

using VECTOR4  = VECTOR<float, 4>;
using VECTOR4S = VECTOR<int, 4>;
using VECTOR4U = VECTOR<uint32_t, 4>;

static_assert(PodT<VECTOR4>, "VECTOR4 must be POD type.");

// =======================================================
//  Constants
// =======================================================

template<VectorScalarT T>
struct MathConstants<VECTOR<T, 4>>
{
    constexpr static VECTOR<T, 4> kZero    = VECTOR<T, 4> { T { 0 } };
    constexpr static VECTOR<T, 4> kOne     = VECTOR<T, 4> { T { 1 } };
    constexpr static VECTOR<T, 4> kRight   = VECTOR<T, 4> { T { 1 }, T { 0 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 4> kUp      = VECTOR<T, 4> { T { 0 }, T { 1 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 4> kForward = VECTOR<T, 4> { T { 0 }, T { 0 }, T { 1 }, T { 0 } };
    constexpr static VECTOR<T, 4> kUnitX   = VECTOR<T, 4> { T { 1 }, T { 0 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 4> kUnitY   = VECTOR<T, 4> { T { 0 }, T { 1 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 4> kUnitZ   = VECTOR<T, 4> { T { 0 }, T { 0 }, T { 1 }, T { 0 } };
    constexpr static VECTOR<T, 4> kUnitW   = VECTOR<T, 4> { T { 0 }, T { 0 }, T { 0 }, T { 1 } };
    constexpr static VECTOR<T, 4> kMax     = VECTOR<T, 4> { MathConstants<T>::kMax };
    constexpr static VECTOR<T, 4> kMin     = VECTOR<T, 4> { MathConstants<T>::kMin };
};

// =======================================================
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(
    const VECTOR4 _v,
    const float   _epsilon = kEpsilon)
{
    return simd::MoveMask(simd::CmpLt(simd::Abs(_v.ToM128()), simd::SetAll(_epsilon))) == 0xf;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _epsilon = kEpsilon)
{
    return simd::MoveMask(simd::CmpLt(simd::Abs(simd::Sub(_x.ToM128(), _y.ToM128())), simd::SetAll(_epsilon))) == 0xf;
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Clamp(
    const VECTOR4 _v,
    const VECTOR4 _min,
    const VECTOR4 _max)
{
    return VECTOR4::FromM128(simd::Max(_min.ToM128(), simd::Min(_max.ToM128(), _v.ToM128())));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Abs(
    const VECTOR4 _v)
{
    return VECTOR4::FromM128(simd::Abs(_v.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Max(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return VECTOR4::FromM128(simd::Max(_x.ToM128(), _y.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Min(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return VECTOR4::FromM128(simd::Min(_x.ToM128(), _y.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Rcp(
    const VECTOR4 _v)
{
    return VECTOR4::FromM128(simd::Div(simd::SetAll(1.f), _v.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Saturate(
    const VECTOR4 _v)
{
    return VECTOR4::FromM128(simd::Max(simd::Zero(), simd::Min(simd::SetAll(1.f), _v.ToM128())));
}

[[nodiscard]] JUG_MATH_API constexpr float Dot(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return simd::Get<0>(simd::Dot4V(_x.ToM128(), _y.ToM128()));
}

[[nodiscard]] JUG_MATH_API constexpr float LengthSq(
    const VECTOR4 _v)
{
    return Dot(_v, _v);
}

[[nodiscard]] JUG_MATH_API constexpr float Length(
    const VECTOR4 _v)
{
    const simd::M128 v = _v.ToM128();
    return simd::Get<0>(simd::Sqrt(simd::Dot4V(v, v)));
}

// 길이가 0 이면 0
[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Normalize(
    const VECTOR4 _v)
{
    const simd::M128 v = _v.ToM128();
    return VECTOR4::FromM128(simd::NormalizeBy(v, simd::Dot4V(v, v)));
}

[[nodiscard]] JUG_MATH_API constexpr float DistanceSq(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    const simd::M128 d = simd::Sub(_x.ToM128(), _y.ToM128());
    return simd::Get<0>(simd::Dot4V(d, d));
}

[[nodiscard]] JUG_MATH_API constexpr float Distance(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    const simd::M128 d = simd::Sub(_x.ToM128(), _y.ToM128());
    return simd::Get<0>(simd::Sqrt(simd::Dot4V(d, d)));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Lerp(
    const VECTOR4 _x,
    const VECTOR4 _y,
    const float   _t)
{
    return VECTOR4::FromM128(simd::Lerp(_x.ToM128(), _y.ToM128(), simd::SetAll(_t)));
}

}   // namespace jug
