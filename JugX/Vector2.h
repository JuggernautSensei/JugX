#pragma once
#include "Math.h"

namespace jug
{

template<typename T>
concept VectorScalarT = std::is_integral_v<T> || std::is_same_v<T, float>;

template<VectorScalarT T, size_t N>
struct VECTOR;

template<VectorScalarT T>
struct VECTOR<T, 2>
{
    JUG_MATH_API VECTOR() = default;

    JUG_MATH_API constexpr VECTOR(
        const T _x,
        const T _y)
        : e { _x, _y }
    {
    }

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR(
        const T _value)
        : e { _value, _value }
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

    const static VECTOR kZero;
    const static VECTOR kOne;
    const static VECTOR kRight;
    const static VECTOR kUp;
    const static VECTOR kUnitX;
    const static VECTOR kUnitY;
    const static VECTOR kMax;
    const static VECTOR kMin;

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

using VECTOR2  = VECTOR<float, 2>;
using VECTOR2S = VECTOR<int, 2>;
using VECTOR2U = VECTOR<uint32_t, 2>;

static_assert(PodT<VECTOR2>, "VECTOR2 must be POD type.");

// =======================================================
//  Constants
// =======================================================

template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kZero { T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kOne { T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kRight = { T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kUp = { T { 0 }, T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kUnitX = { T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kUnitY = { T { 0 }, T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kMax { MathConstants<T>::kMax };
template<VectorScalarT T>
inline constexpr VECTOR<T, 2> VECTOR<T, 2>::kMin { MathConstants<T>::kMin };

template<>
struct MathConstants<VECTOR2>
{
    constexpr static VECTOR2 kZero  = VECTOR2::kZero;
    constexpr static VECTOR2 kOne   = VECTOR2::kOne;
    constexpr static VECTOR2 kRight = VECTOR2::kRight;
    constexpr static VECTOR2 kUp    = VECTOR2::kUp;
    constexpr static VECTOR2 kUnitX = VECTOR2::kUnitX;
    constexpr static VECTOR2 kUnitY = VECTOR2::kUnitY;
    constexpr static VECTOR2 kMax   = VECTOR2::kMax;
    constexpr static VECTOR2 kMin   = VECTOR2::kMin;
};

}   // namespace jug
