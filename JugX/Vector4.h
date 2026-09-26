#pragma once
#include <concepts>

#include "Vector3.h"

namespace jug
{

template<VectorScalarT T>
struct alignas(16) VECTOR<T, 4>
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

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-() const
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] = -ret[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator+(
        const VECTOR _other) const
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] += _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-(
        const VECTOR _other) const
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] -= _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator*(
        const T _scalar) const
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] *= _scalar;
        }
        return ret;
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
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] *= _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const VECTOR _other) const
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] /= _other[i];
        }
        return ret;
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
        for (size_t i = 0; i < kDim; ++i)
        {
            if (e[i] != _other[i])   // NOLINT
            {
                return false;
            }
        }
        return true;
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

static_assert(sizeof(VECTOR4) == 16, "VECTOR4 must be tightly packed");
static_assert(alignof(VECTOR4) == 16, "VECTOR4 must be 16-byte aligned");
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

}   // namespace jug
