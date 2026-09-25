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

    // 요소 셋으로 조립한다 (w = 0). LoadFloat3 로 읽으면 스칼라로 막 만든 임시값·상수에서
    // 작은 store 여러 개를 큰 load 하나로 읽게 돼 store-forwarding 이 실패한다.
    [[nodiscard]] simd::M128 JUG_VECTORCALL ToM128() const
        requires std::is_floating_point_v<T>
    {
        return simd::Set(e[0], e[1], e[2], 0.f);
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
        return VECTOR<T, 2> { e[0], e[1] };
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

using VECTOR3  = VECTOR<float, 3>;
using VECTOR3S = VECTOR<int, 3>;
using VECTOR3U = VECTOR<uint32_t, 3>;

static_assert(PodT<VECTOR3>, "VECTOR3 must be POD type.");

// =======================================================
//  Constants
// =======================================================

// 클래스 템플릿의 static 상수는 clang/gcc 에서 constexpr 로 못 쓴다 - MathConstants 로만 둔다.
template<VectorScalarT T>
struct MathConstants<VECTOR<T, 3>>
{
    constexpr static VECTOR<T, 3> kZero    = VECTOR<T, 3> { T { 0 } };
    constexpr static VECTOR<T, 3> kOne     = VECTOR<T, 3> { T { 1 } };
    constexpr static VECTOR<T, 3> kRight   = VECTOR<T, 3> { T { 1 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 3> kUp      = VECTOR<T, 3> { T { 0 }, T { 1 }, T { 0 } };
    constexpr static VECTOR<T, 3> kForward= VECTOR<T, 3> { T { 0 }, T { 0 }, T { 1 } };
    constexpr static VECTOR<T, 3> kUnitX   = VECTOR<T, 3> { T { 1 }, T { 0 }, T { 0 } };
    constexpr static VECTOR<T, 3> kUnitY   = VECTOR<T, 3> { T { 0 }, T { 1 }, T { 0 } };
    constexpr static VECTOR<T, 3> kUnitZ   = VECTOR<T, 3> { T { 0 }, T { 0 }, T { 1 } };
    constexpr static VECTOR<T, 3> kMax     = VECTOR<T, 3> { MathConstants<T>::kMax };
    constexpr static VECTOR<T, 3> kMin     = VECTOR<T, 3> { MathConstants<T>::kMin };
};

}   // namespace jug
