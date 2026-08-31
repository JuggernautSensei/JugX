#pragma once
#include <array>

#include "Macros.h"
#include "Math.h"
#include "TypeTraits.h"
#include "SIMD.h"
#include "Vector2.h"
#include "Vector3.h"

namespace jug
{

#ifdef JUG_SIMD_AVAILABLE

struct alignas(16) VECTOR4
{
    using ValueT = float;

    VECTOR4() = default;

    constexpr VECTOR4(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        : e { _x, _y, _z, _w }
    {
    }

    constexpr VECTOR4(
        const VECTOR2 _xy,
        const float   _z,
        const float   _w)
        : e { _xy.e[0], _xy.e[1], _z, _w }
    {
    }

    constexpr VECTOR4(
        const VECTOR3 _xyz,
        const float   _w)
        : e { _xyz.e[0], _xyz.e[1], _xyz.e[2], _w }
    {
    }

    // Broadcast
    explicit constexpr VECTOR4(
        const float _value)
        : e { _value, _value, _value, _value }
    {
    }

    // =======================================================
    //  SIMD utils
    // =======================================================

    [[nodiscard]] simd::M128 ToSIMD() const
    {
        return simd::LoadAligned(e.data());
    }

    [[nodiscard]] static VECTOR4 MakeFromSIMD(
        const simd::M128 _value)
    {
        VECTOR4 v;
        simd::StoreAligned(v.e.data(), _value);
        return v;
    }

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] constexpr VECTOR4 operator-() const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Negate(ToSIMD()));
        }

        return VECTOR4 { -e[0], -e[1], -e[2], -e[3] };
    }

    [[nodiscard]] constexpr VECTOR4 operator+(
        const VECTOR4 _other) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Add(ToSIMD(), _other.ToSIMD()));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] constexpr VECTOR4 operator-(
        const VECTOR4 _other) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Sub(ToSIMD(), _other.ToSIMD()));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] constexpr VECTOR4 operator*(
        const float _scalar) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Scale(ToSIMD(), _scalar));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] constexpr VECTOR4 operator/(
        const float _scalar) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Div(ToSIMD(), simd::SetAll(_scalar)));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] constexpr VECTOR4 operator*(
        const VECTOR4 _other) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Mul(ToSIMD(), _other.ToSIMD()));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] constexpr VECTOR4 operator/(
        const VECTOR4 _other) const
    {
        if (!std::is_constant_evaluated())
        {
            return MakeFromSIMD(simd::Div(ToSIMD(), _other.ToSIMD()));
        }

        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    // =======================================================
    //  Assignment
    // =======================================================

    constexpr VECTOR4& operator+=(
        const VECTOR4 _other)
    {
        *this = *this + _other;
        return *this;
    }

    constexpr VECTOR4& operator-=(
        const VECTOR4 _other)
    {
        *this = *this - _other;
        return *this;
    }

    constexpr VECTOR4& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    constexpr VECTOR4& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    constexpr VECTOR4& operator*=(
        const VECTOR4 _other)
    {
        *this = *this * _other;
        return *this;
    }

    constexpr VECTOR4& operator/=(
        const VECTOR4 _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(
        const VECTOR4 _other) const
    {
        if (!std::is_constant_evaluated())
        {
            return simd::AllTrue(simd::CmpEq(ToSIMD(), _other.ToSIMD()));
        }

        for (size_t i = 0; i < kDim; ++i)
        {
            if (e[i] != _other.e[i])   // NOLINT
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr bool operator!=(
        const VECTOR4 _other) const
    {
        return !(*this == _other);
    }

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] constexpr ValueT& operator[](
        const size_t _index)
    {
        return e[_index];
    }

    [[nodiscard]] constexpr const ValueT& operator[](
        const size_t _index) const
    {
        return e[_index];
    }

    [[nodiscard]] constexpr ValueT* GetPtr()
    {
        return e.data();
    }

    [[nodiscard]] constexpr const ValueT* GetPtr() const
    {
        return e.data();
    }

    // =======================================================
    //  Constants
    // =======================================================

    const static VECTOR4 kZero;
    const static VECTOR4 kOne;
    const static VECTOR4 kRight;
    const static VECTOR4 kUp;
    const static VECTOR4 kForward;
    const static VECTOR4 kUnitX;
    const static VECTOR4 kUnitY;
    const static VECTOR4 kUnitZ;
    const static VECTOR4 kUnitW;
    const static VECTOR4 kMax;
    const static VECTOR4 kMin;

    constexpr static size_t kDim = 4;

    JUG_MATH_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        ARRAY<float, 4> e;
    };
    JUG_MATH_DISABLE_ANON_WARNING_END
};

#else

struct alignas(16) VECTOR4
{
    using ValueT                 = float;
    constexpr static size_t kDim = 4;

    JUG_MATH_API VECTOR4() = default;

    JUG_MATH_API constexpr VECTOR4(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        : e { _x, _y, _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR4(
        const VECTOR2 _xy,
        const float   _z,
        const float   _w)
        : e { _xy.e[0], _xy.e[1], _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR4(
        const VECTOR3 _xyz,
        const float   _w)
        : e { _xyz.e[0], _xyz.e[1], _xyz.e[2], _w }
    {
    }

    explicit JUG_MATH_API constexpr VECTOR4(
        const float _value)
        : e { _value, _value, _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator-() const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator+(
        const VECTOR4 _other) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator-(
        const VECTOR4 _other) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator*(
        const float _scalar) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator/(
        const float _scalar) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator*(
        const VECTOR4 _other) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator/(
        const VECTOR4 _other) const
    {
        VECTOR4 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    JUG_MATH_API constexpr VECTOR4& operator+=(
        const VECTOR4 _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4& operator-=(
        const VECTOR4 _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4& operator*=(
        const VECTOR4 _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4& operator/=(
        const VECTOR4 _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR4 _other) const
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
        const VECTOR4 _other) const
    {
        return !(*this == _other);
    }

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

    const static VECTOR4 kZero;
    const static VECTOR4 kOne;
    const static VECTOR4 kRight;
    const static VECTOR4 kUp;
    const static VECTOR4 kForward;
    const static VECTOR4 kUnitX;
    const static VECTOR4 kUnitY;
    const static VECTOR4 kUnitZ;
    const static VECTOR4 kUnitW;
    const static VECTOR4 kMax;
    const static VECTOR4 kMin;

    JUG_MATH_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        ARRAY<float, 4> e;
    };
    JUG_MATH_DISABLE_ANON_WARNING_END
};

#endif   // JUG_MATH_SIMD

static_assert(sizeof(VECTOR4) == 16, "VECTOR4 must be tightly packed");
static_assert(alignof(VECTOR4) == 16, "VECTOR4 must be 16-byte aligned for SIMD");
JUG_STATIC_ASSERT_POD(VECTOR4);

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR4 VECTOR4::kZero { 0.f };
inline constexpr VECTOR4 VECTOR4::kOne { 1.f };
inline constexpr VECTOR4 VECTOR4::kRight   = { 1.f, 0.f, 0.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kUp      = { 0.f, 1.f, 0.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kForward = { 0.f, 0.f, 1.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kUnitX   = { 1.f, 0.f, 0.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kUnitY   = { 0.f, 1.f, 0.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kUnitZ   = { 0.f, 0.f, 1.f, 0.f };
inline constexpr VECTOR4 VECTOR4::kUnitW   = { 0.f, 0.f, 0.f, 1.f };
inline constexpr VECTOR4 VECTOR4::kMax { MathConstants<float>::kMax };
inline constexpr VECTOR4 VECTOR4::kMin { MathConstants<float>::kMin };

template<>
struct MathConstants<VECTOR4>
{
    constexpr static VECTOR4 kZero { 0.f };
    constexpr static VECTOR4 kOne { 1.f };
    constexpr static VECTOR4 kRight   = { 1.f, 0.f, 0.f, 0.f };
    constexpr static VECTOR4 kUp      = { 0.f, 1.f, 0.f, 0.f };
    constexpr static VECTOR4 kForward = { 0.f, 0.f, 1.f, 0.f };
    constexpr static VECTOR4 kUnitX   = { 1.f, 0.f, 0.f, 0.f };
    constexpr static VECTOR4 kUnitY   = { 0.f, 1.f, 0.f, 0.f };
    constexpr static VECTOR4 kUnitZ   = { 0.f, 0.f, 1.f, 0.f };
    constexpr static VECTOR4 kUnitW   = { 0.f, 0.f, 0.f, 1.f };
    constexpr static VECTOR4 kMax { MathConstants<float>::kMax };
    constexpr static VECTOR4 kMin { MathConstants<float>::kMin };
};

}   // namespace jug
