#pragma once
#include <concepts>

#include "SIMD.h"
#include "Vector3.h"

#ifdef JUG_SIMD_AVAILABLE
#    define JUG_VECTOR4_SIMD_PATH(_expr)           \
        JUG_BEGIN_MACRO_BLOCK                      \
        if constexpr (std::is_floating_point_v<T>) \
        {                                          \
            if (!std::is_constant_evaluated())     \
            {                                      \
                return (_expr);                    \
            }                                      \
        }                                          \
        JUG_END_MACRO_BLOCK
#else
#    define JUG_VECTOR4_SIMD_PATH(_expr)
#endif

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

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR(
        const T _value)
        : e { _value, _value, _value, _value }
    {
    }

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static VECTOR MakeFromM128(
        const simd::M128 _value)
        requires std::is_floating_point_v<T>
    {
        VECTOR ret;
        simd::StoreAligned(ret.e.data(), _value);
        return ret;
    }

    [[nodiscard]] simd::M128 ToM128() const
        requires std::is_floating_point_v<T>
    {
        return simd::LoadAligned(e.data());
    }
#endif

    [[nodiscard]] JUG_MATH_API constexpr VECTOR operator-() const
    {
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Negate(ToM128())));

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
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Add(ToM128(), _other.ToM128())));

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
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Sub(ToM128(), _other.ToM128())));

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
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Scale(ToM128(), _scalar)));

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
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Mult(ToM128(), _other.ToM128())));

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
        JUG_VECTOR4_SIMD_PATH(MakeFromM128(simd::Div(ToM128(), _other.ToM128())));

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
        JUG_VECTOR4_SIMD_PATH(simd::AllTrue(simd::CmpEq(ToM128(), _other.ToM128())));

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

    [[nodiscard]] JUG_MATH_API constexpr VECTOR<T, 3> ToVector3() const
    {
        return VECTOR<T, 3> { x, y, z };
    }

    const static VECTOR kZero;
    const static VECTOR kOne;
    const static VECTOR kRight;
    const static VECTOR kUp;
    const static VECTOR kForward;
    const static VECTOR kUnitX;
    const static VECTOR kUnitY;
    const static VECTOR kUnitZ;
    const static VECTOR kUnitW;
    const static VECTOR kMax;
    const static VECTOR kMin;

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
static_assert(alignof(VECTOR4) == 16, "VECTOR4 must be 16-byte aligned for SIMD");
static_assert(PodT<VECTOR4>, "VECTOR4 must be POD type.");

// =======================================================
//  Constants
// =======================================================

template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kZero { T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kOne { T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kRight = { T { 1 }, T { 0 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kUp = { T { 0 }, T { 1 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kForward = { T { 0 }, T { 0 }, T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kUnitX = { T { 1 }, T { 0 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kUnitY = { T { 0 }, T { 1 }, T { 0 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kUnitZ = { T { 0 }, T { 0 }, T { 1 }, T { 0 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kUnitW = { T { 0 }, T { 0 }, T { 0 }, T { 1 } };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kMax { MathConstants<T>::kMax };
template<VectorScalarT T>
inline constexpr VECTOR<T, 4> VECTOR<T, 4>::kMin { MathConstants<T>::kMin };

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

#undef JUG_VECTOR4_SIMD_PATH