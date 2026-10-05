#pragma once
#include "Math.h"

namespace jug
{

template<typename T>
concept VectorScalarT = std::is_integral_v<T> || std::is_same_v<T, float>;

template<VectorScalarT T, size_t N>
struct VEC;

template<VectorScalarT T>
struct VEC<T, 2>
{
    JUG_MATH_API VEC() = default;

    JUG_MATH_API constexpr VEC(
        const T _x,
        const T _y)
        : e { _x, _y }
    {
    }

    explicit JUG_MATH_API constexpr VEC(
        const T _value)
        : e { _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator-() const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] = -ret[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator+(
        const VEC _other) const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] += _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator-(
        const VEC _other) const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] -= _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator*(
        const T _scalar) const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] *= _scalar;
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator/(
        const T _scalar) const
        requires std::is_integral_v<T>
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] /= _scalar;
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator/(
        const T _scalar) const
        requires std::is_floating_point_v<T>
    {
        return *this * (1.f / _scalar);
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator*(
        const VEC _other) const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] *= _other[i];
        }
        return ret;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator/(
        const VEC _other) const
    {
        VEC ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] /= _other[i];
        }
        return ret;
    }

    JUG_MATH_API constexpr VEC& operator+=(
        const VEC _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VEC& operator-=(
        const VEC _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VEC& operator*=(
        const T _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VEC& operator/=(
        const T _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VEC& operator*=(
        const VEC _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VEC& operator/=(
        const VEC _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VEC _other) const
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

    [[nodiscard]] JUG_MATH_API constexpr static VEC Zero()
    {
        return VEC { T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC One()
    {
        return VEC { T { 1 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Right()
    {
        return VEC { T { 1 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Up()
    {
        return VEC { T { 0 }, T { 1 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC UnitX()
    {
        return VEC { T { 1 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC UnitY()
    {
        return VEC { T { 0 }, T { 1 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Max()
    {
        return VEC { std::numeric_limits<T>::max() };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Min()
    {
        return VEC { std::numeric_limits<T>::lowest() };
    }

    constexpr static size_t kDim = 2;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            T x;
            T y;
        };
        struct
        {
            T width;
            T height;
        };
        struct
        {
            T u;
            T v;
        };
        ARRAY<T, 2> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

using VEC2  = VEC<float, 2>;
using VEC2I = VEC<int, 2>;
using VEC2U = VEC<uint32_t, 2>;

static_assert(PodT<VEC2>, "VEC2 must be POD type.");

}   // namespace jug
