#pragma once
#include "Vector2.h"

namespace jug
{

template<VectorScalarT T>
struct VEC<T, 3>
{
    JUG_MATH_API VEC() = default;

    JUG_MATH_API constexpr VEC(
        const T _x,
        const T _y,
        const T _z)
        : e { _x, _y, _z }
    {
    }

    JUG_MATH_API constexpr VEC(
        const VEC<T, 2> _xy,
        const T         _z)
        : e { _xy[0], _xy[1], _z }
    {
    }

    explicit JUG_MATH_API constexpr VEC(
        const T _value)
        : e { _value, _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator-() const
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] = -v[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator+(
        const VEC _other) const
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] += _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator-(
        const VEC _other) const
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] -= _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator*(
        const T _scalar) const
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator/(
        const T _scalar) const
        requires std::is_integral_v<T>
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] /= _scalar;
        }
        return v;
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
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] *= _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VEC operator/(
        const VEC _other) const
    {
        VEC v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] /= _other[i];
        }
        return v;
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

    [[nodiscard]] JUG_MATH_API constexpr VEC<T, 2> ToVector2() const
    {
        return VEC<T, 2> { e[0], e[1] };
    }

    // w = 0
    [[nodiscard]] JUG_MATH_API constexpr simd::M128 JUG_VECTORCALL ToM128() const
        requires std::is_same_v<T, float>
    {
        return simd::Set(e[0], e[1], e[2], 0.f);
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC JUG_VECTORCALL FromM128(
        const simd::M128 _v)
        requires std::is_same_v<T, float>
    {
        if (std::is_constant_evaluated())
        {
            return simd::NoInlineConstruct<VEC>(simd::Get<0>(_v), simd::Get<1>(_v), simd::Get<2>(_v));
        }
        else
        {
            VEC ret;
            simd::StoreFloat3(ret.e.data(), _v);
            return ret;
        }
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
        return VEC { T { 1 }, T { 0 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Up()
    {
        return VEC { T { 0 }, T { 1 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Forward()
    {
        return VEC { T { 0 }, T { 0 }, T { 1 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC UnitX()
    {
        return VEC { T { 1 }, T { 0 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC UnitY()
    {
        return VEC { T { 0 }, T { 1 }, T { 0 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC UnitZ()
    {
        return VEC { T { 0 }, T { 0 }, T { 1 } };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Max()
    {
        return VEC { std::numeric_limits<T>::max() };
    }

    [[nodiscard]] JUG_MATH_API constexpr static VEC Min()
    {
        return VEC { std::numeric_limits<T>::lowest() };
    }

    constexpr static size_t kDim = 3;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            T x, y, z;
        };
        struct
        {
            T width, height, depth;
        };
        struct
        {
            T pitch, yaw, roll;
        };
        struct
        {
            T r, g, b;
        };
        struct
        {
            T u, v, w;
        };
        ARRAY<T, 3> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

using VEC3  = VEC<float, 3>;
using VEC3I = VEC<int, 3>;
using VEC3U = VEC<uint32_t, 3>;

static_assert(PodT<VEC3>, "VEC3 must be POD type.");

}   // namespace jug
