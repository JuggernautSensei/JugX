

#pragma once
#include <array>

#include "Macro.h"
#include "Math.h"
#include "Typedef.h"
#include "TypeTraits.h"
#include "SIMD.h"
#include "Vector2.h"

namespace jug
{

struct VECTOR3
{
    using ValueT = float;

    JUG_MATH_API VECTOR3() = default;

    JUG_MATH_API constexpr VECTOR3(
        const float _x,
        const float _y,
        const float _z)
        : e { _x, _y, _z }
    {
    }

    JUG_MATH_API constexpr VECTOR3(
        const VECTOR2 _xy,
        const float   _z)
        : e { _xy.e[0], _xy.e[1], _z }
    {
    }

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR3(
        const float _value)
        : e { _value, _value, _value }
    {
    }

#ifdef JUG_SIMD_AVAILABLE

    // =======================================================
    //  SIMD utils
    // =======================================================

    [[nodiscard]] simd::M128 ToSIMD() const
    {
        return simd::LoadFloat3(e.data());
    }

    [[nodiscard]] static VECTOR3 MakeFromSIMD(
        const simd::M128 _value)
    {
        VECTOR3 v;
        simd::StoreFloat3(v.e.data(), _value);
        return v;
    }

#endif

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator-() const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator+(
        const VECTOR3 _other) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator-(
        const VECTOR3 _other) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator*(
        const float _scalar) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator/(
        const float _scalar) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator*(
        const VECTOR3 _other) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 operator/(
        const VECTOR3 _other) const
    {
        VECTOR3 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    // =======================================================
    //  Assignment
    // =======================================================

    JUG_MATH_API constexpr VECTOR3& operator+=(
        const VECTOR3 _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3& operator-=(
        const VECTOR3 _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3& operator*=(
        const VECTOR3 _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3& operator/=(
        const VECTOR3 _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR3 _other) const
    {
        for (size_t i = 0; i < kDim; ++i)
        {
            if (e[i] != _other.e[i])   // NOLINT
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator!=(
        const VECTOR3 _other) const
    {
        return !(*this == _other);
    }

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr ValueT& operator[](
        const size_t _index)
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr const ValueT& operator[](
        const size_t _index) const
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr ValueT* GetPtr()
    {
        return e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr const ValueT* GetPtr() const
    {
        return e.data();
    }

    // =======================================================
    //  Fields
    // =======================================================

    const static VECTOR3 kZero;
    const static VECTOR3 kOne;
    const static VECTOR3 kRight;
    const static VECTOR3 kUp;
    const static VECTOR3 kForward;
    const static VECTOR3 kUnitX;
    const static VECTOR3 kUnitY;
    const static VECTOR3 kUnitZ;
    const static VECTOR3 kMax;
    const static VECTOR3 kMin;

    constexpr static size_t kDim = 3;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        ARRAY<float, 3> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(PodT<VECTOR3>, "VECTOR3 must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR3 VECTOR3::kZero { 0.f };
inline constexpr VECTOR3 VECTOR3::kOne { 1.f };
inline constexpr VECTOR3 VECTOR3::kRight   = { 1.f, 0.f, 0.f };
inline constexpr VECTOR3 VECTOR3::kUp      = { 0.f, 1.f, 0.f };
inline constexpr VECTOR3 VECTOR3::kForward = { 0.f, 0.f, 1.f };
inline constexpr VECTOR3 VECTOR3::kUnitX   = { 1.f, 0.f, 0.f };
inline constexpr VECTOR3 VECTOR3::kUnitY   = { 0.f, 1.f, 0.f };
inline constexpr VECTOR3 VECTOR3::kUnitZ   = { 0.f, 0.f, 1.f };
inline constexpr VECTOR3 VECTOR3::kMax { MathConstants<float>::kMax };
inline constexpr VECTOR3 VECTOR3::kMin { MathConstants<float>::kMin };

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
