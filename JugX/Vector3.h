#pragma once
#include <concepts>

#include "SIMD.h"
#include "Vector2.h"

namespace jug
{

template<VectorScalarT T>
struct VECTOR<T, 3>
{
    JUG_MATH_API VECTOR() = default;

    JUG_MATH_API constexpr VECTOR(
        const T _x,
        const T _y,
        const T _z)
        : e { _x, _y, _z }
    {
    }

    JUG_MATH_API constexpr VECTOR(
        const VECTOR<T, 2> _xy,
        const T            _z)
        : e { _xy[0], _xy[1], _z }
    {
    }

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR(
        const T _value)
        : e { _value, _value, _value }
    {
    }

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static VECTOR MakeFromM128(
        const simd::M128 _value)
        requires std::is_floating_point_v<T>
    {
        VECTOR ret;
        simd::StoreFloat3(ret.e.data(), _value);
        return ret;
    }

    [[nodiscard]] simd::M128 ToM128() const
        requires std::is_floating_point_v<T>
    {
        return simd::LoadFloat3(e.data());
    }
#endif

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-() const
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] = -v[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator+(
        const VECTOR _other) const
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] += _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-(
        const VECTOR _other) const
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] -= _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator*(
        const T _scalar) const
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const T _scalar) const
        requires std::is_integral_v<T>
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] /= _scalar;
        }
        return v;
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
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] *= _other[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator/(
        const VECTOR _other) const
    {
        VECTOR v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v[i] /= _other[i];
        }
        return v;
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
        return VECTOR<T, 2> { x, y };
    }

    const static VECTOR kZero;
    const static VECTOR kOne;
    const static VECTOR kRight;
    const static VECTOR kUp;
    const static VECTOR kForward;
    const static VECTOR kUnitX;
    const static VECTOR kUnitY;
    const static VECTOR kUnitZ;
    const static VECTOR kMax;
    const static VECTOR kMin;

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

using VECTOR3  = VECTOR<float, 3>;
using VECTOR3S = VECTOR<int, 3>;
using VECTOR3U = VECTOR<uint32_t, 3>;

static_assert(PodT<VECTOR3>, "VECTOR3 must be POD type.");

// =======================================================
//  Constants
// =======================================================

template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kZero { T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kOne { T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kRight = { T { 1 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kUp = { T { 0 }, T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kForward = { T { 0 }, T { 0 }, T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kUnitX = { T { 1 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kUnitY = { T { 0 }, T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kUnitZ = { T { 0 }, T { 0 }, T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kMax { MathConstants<T>::kMax };
template<VectorScalarT T>
inline constexpr VECTOR<T, 3> VECTOR<T, 3>::kMin { MathConstants<T>::kMin };

template<>
struct MathConstants<VECTOR3>
{
    constexpr static VECTOR3 kZero    = VECTOR3::kZero;
    constexpr static VECTOR3 kOne     = VECTOR3::kOne;
    constexpr static VECTOR3 kRight   = VECTOR3::kRight;
    constexpr static VECTOR3 kUp      = VECTOR3::kUp;
    constexpr static VECTOR3 kForward = VECTOR3::kForward;
    constexpr static VECTOR3 kUnitX   = VECTOR3::kUnitX;
    constexpr static VECTOR3 kUnitY   = VECTOR3::kUnitY;
    constexpr static VECTOR3 kUnitZ   = VECTOR3::kUnitZ;
    constexpr static VECTOR3 kMax     = VECTOR3::kMax;
    constexpr static VECTOR3 kMin     = VECTOR3::kMin;
};

}   // namespace jug
