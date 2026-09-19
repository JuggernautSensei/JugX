#pragma once
#include "SIMD.h"
#include "Vector3.h"

namespace jug
{

#ifdef JUG_SIMD_AVAILABLE
#    define JUG_VECTOR4_SIMD_PATH(_expr)   \
        if (!std::is_constant_evaluated()) \
        return _expr
#else
#    define JUG_VECTOR4_SIMD_PATH(_expr)
#endif

struct alignas(16) VECTOR4
{
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

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR4(
        const float _value)
        : e { _value, _value, _value, _value }
    {
    }

#ifdef JUG_SIMD_AVAILABLE

    JUG_MATH_API /* implicit */ VECTOR4(
        const simd::M128 _value)
    {
        simd::StoreAligned(e.data(), _value);
    }

    [[nodiscard]] simd::M128 ToSIMD() const
    {
        return simd::LoadAligned(e.data());
    }

#endif

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4 operator-() const
    {
        JUG_VECTOR4_SIMD_PATH(simd::Negate(ToSIMD()));

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
        JUG_VECTOR4_SIMD_PATH(simd::Add(ToSIMD(), _other.ToSIMD()));

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
        JUG_VECTOR4_SIMD_PATH(simd::Sub(ToSIMD(), _other.ToSIMD()));

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
        JUG_VECTOR4_SIMD_PATH(simd::Scale(ToSIMD(), _scalar));

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
        JUG_VECTOR4_SIMD_PATH(simd::Div(ToSIMD(), simd::SetAll(_scalar)));

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
        JUG_VECTOR4_SIMD_PATH(simd::Mul(ToSIMD(), _other.ToSIMD()));

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
        JUG_VECTOR4_SIMD_PATH(simd::Div(ToSIMD(), _other.ToSIMD()));

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
        JUG_VECTOR4_SIMD_PATH(simd::AllTrue(simd::CmpEq(ToSIMD(), _other.ToSIMD())));

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

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr float& operator[](
        const size_t _index)
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr const float& operator[](
        const size_t _index) const
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
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

    constexpr static size_t kDim = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
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
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(VECTOR4) == 16, "VECTOR4 must be tightly packed");
static_assert(alignof(VECTOR4) == 16, "VECTOR4 must be 16-byte aligned for SIMD");
static_assert(PodT<VECTOR4>, "VECTOR4 must be POD type.");

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
    constexpr static VECTOR4 kZero    = VECTOR4::kZero;
    constexpr static VECTOR4 kOne     = VECTOR4::kOne;
    constexpr static VECTOR4 kRight   = VECTOR4::kRight;
    constexpr static VECTOR4 kUp      = VECTOR4::kUp;
    constexpr static VECTOR4 kForward = VECTOR4::kForward;
    constexpr static VECTOR4 kUnitX   = VECTOR4::kUnitX;
    constexpr static VECTOR4 kUnitY   = VECTOR4::kUnitY;
    constexpr static VECTOR4 kUnitZ   = VECTOR4::kUnitZ;
    constexpr static VECTOR4 kUnitW   = VECTOR4::kUnitW;
    constexpr static VECTOR4 kMax     = VECTOR4::kMax;
    constexpr static VECTOR4 kMin     = VECTOR4::kMin;
};

}   // namespace jug
