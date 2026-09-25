#pragma once
#include <concepts>

#include "SIMD.h"
#include "Vector3.h"

// constexpr 평가가 아닐 때만 SIMD 경로로 반환한다
#ifdef JUG_SIMD_AVAILABLE
#    define JUG_SIMD_PATH(...)                 \
        if (!std::is_constant_evaluated()) \
        {                                  \
            return __VA_ARGS__;            \
        }
#else
#    define JUG_SIMD_PATH(...)
#endif

namespace jug
{

// =======================================================
//  Integer VECTOR4 (VECTOR4S / VECTOR4U)
// =======================================================

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
    {
        VECTOR ret = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            ret[i] /= _scalar;
        }
        return ret;
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
            if (e[i] != _other[i])
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

// =======================================================
//  VECTOR4
// =======================================================

// 레지스터 타입. 멤버가 simd::M128 하나뿐이라 JUG_VECTORCALL 함수 사이를 xmm 레지스터로 오간다 (DirectXMath XMVECTOR).
// 요소는 GetX()/SetX()/operator[] 로 접근한다. SIMD 가 없으면 (CUDA 디바이스 등) 같은 레이아웃의 float[4].
// 연산자는 자유 함수다 - 멤버 연산자는 this 가 메모리 포인터라 레지스터로 못 넘긴다.
template<>
struct alignas(16) VECTOR<float, 4>
{
    JUG_MATH_API VECTOR() = default;

    JUG_MATH_API constexpr VECTOR(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
#ifdef JUG_SIMD_AVAILABLE
        : v(simd::Set(_x, _y, _z, _w))
#else
        : e { _x, _y, _z, _w }
#endif
    {
    }

    JUG_MATH_API constexpr VECTOR(
        const VECTOR<float, 2> _xy,
        const float            _z,
        const float            _w)
        : VECTOR(_xy[0], _xy[1], _z, _w)
    {
    }

    JUG_MATH_API constexpr VECTOR(
        const VECTOR<float, 3> _xyz,
        const float            _w)
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            v = simd::Insert<3>(_xyz.ToM128(), _w);
            return;
        }
        v = simd::MakeConstant(_xyz[0], _xyz[1], _xyz[2], _w);
#else
        e[0] = _xyz[0];
        e[1] = _xyz[1];
        e[2] = _xyz[2];
        e[3] = _w;
#endif
    }

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR(
        const float _value)
#ifdef JUG_SIMD_AVAILABLE
        : v(simd::SetAll(_value))
#else
        : e { _value, _value, _value, _value }
#endif
    {
    }

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static constexpr VECTOR JUG_VECTORCALL MakeFromM128(
        const simd::M128 _value)
    {
        VECTOR ret;
        ret.v = _value;
        return ret;
    }

    [[nodiscard]] constexpr simd::M128 ToM128() const
    {
        return v;
    }
#endif

    // =======================================================
    //  Access
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr float GetX() const
    {
#ifdef JUG_SIMD_AVAILABLE
        JUG_SIMD_PATH(simd::Get<0>(v));
        return simd::GetLane(v, 0);
#else
        return e[0];
#endif
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetY() const
    {
#ifdef JUG_SIMD_AVAILABLE
        JUG_SIMD_PATH(simd::Get<1>(v));
        return simd::GetLane(v, 1);
#else
        return e[1];
#endif
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetZ() const
    {
#ifdef JUG_SIMD_AVAILABLE
        JUG_SIMD_PATH(simd::Get<2>(v));
        return simd::GetLane(v, 2);
#else
        return e[2];
#endif
    }

    [[nodiscard]] JUG_MATH_API constexpr float GetW() const
    {
#ifdef JUG_SIMD_AVAILABLE
        JUG_SIMD_PATH(simd::Get<3>(v));
        return simd::GetLane(v, 3);
#else
        return e[3];
#endif
    }

    JUG_MATH_API constexpr void SetX(
        const float _value)
    {
#ifdef JUG_SIMD_AVAILABLE
        v = std::is_constant_evaluated() ? simd::MakeConstant(_value, simd::GetLane(v, 1), simd::GetLane(v, 2), simd::GetLane(v, 3)) : simd::Insert<0>(v, _value);
#else
        e[0] = _value;
#endif
    }

    JUG_MATH_API constexpr void SetY(
        const float _value)
    {
#ifdef JUG_SIMD_AVAILABLE
        v = std::is_constant_evaluated() ? simd::MakeConstant(simd::GetLane(v, 0), _value, simd::GetLane(v, 2), simd::GetLane(v, 3)) : simd::Insert<1>(v, _value);
#else
        e[1] = _value;
#endif
    }

    JUG_MATH_API constexpr void SetZ(
        const float _value)
    {
#ifdef JUG_SIMD_AVAILABLE
        v = std::is_constant_evaluated() ? simd::MakeConstant(simd::GetLane(v, 0), simd::GetLane(v, 1), _value, simd::GetLane(v, 3)) : simd::Insert<2>(v, _value);
#else
        e[2] = _value;
#endif
    }

    JUG_MATH_API constexpr void SetW(
        const float _value)
    {
#ifdef JUG_SIMD_AVAILABLE
        v = std::is_constant_evaluated() ? simd::MakeConstant(simd::GetLane(v, 0), simd::GetLane(v, 1), simd::GetLane(v, 2), _value) : simd::Insert<3>(v, _value);
#else
        e[3] = _value;
#endif
    }

    // 인덱스가 런타임 값이면 메모리를 거친다 - 고정 인덱스는 GetX() ~ GetW() 를 쓸 것
    [[nodiscard]] JUG_MATH_API constexpr float operator[](
        const size_t _index) const
    {
#ifdef JUG_SIMD_AVAILABLE
        return simd::GetLane(v, _index);
#else
        return e[_index];
#endif
    }

    JUG_MATH_API constexpr void Set(
        const size_t _index,
        const float  _value)
    {
        switch (_index)
        {
            case 0: SetX(_value); return;
            case 1: SetY(_value); return;
            case 2: SetZ(_value); return;
            default: SetW(_value); return;
        }
    }

    [[nodiscard]] JUG_MATH_API float* GetPtr()
    {
        return reinterpret_cast<float*>(this);
    }

    [[nodiscard]] JUG_MATH_API const float* GetPtr() const
    {
        return reinterpret_cast<const float*>(this);
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR<float, 2> ToVector2() const
    {
        return VECTOR<float, 2> { GetX(), GetY() };
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR<float, 3> ToVector3() const
    {
        JUG_SIMD_PATH(VECTOR<float, 3>::MakeFromM128(v));
        return VECTOR<float, 3> { GetX(), GetY(), GetZ() };
    }

    // =======================================================
    //  Compound Assignment (정의는 자유 연산자 뒤)
    // =======================================================

    JUG_MATH_API constexpr VECTOR& operator+=(VECTOR _other);
    JUG_MATH_API constexpr VECTOR& operator-=(VECTOR _other);
    JUG_MATH_API constexpr VECTOR& operator*=(float _scalar);
    JUG_MATH_API constexpr VECTOR& operator/=(float _scalar);
    JUG_MATH_API constexpr VECTOR& operator*=(VECTOR _other);
    JUG_MATH_API constexpr VECTOR& operator/=(VECTOR _other);

    constexpr static size_t kDim = 4;

#ifdef JUG_SIMD_AVAILABLE
    simd::M128 v;
#else
    float e[4];
#endif
};

using VECTOR4  = VECTOR<float, 4>;
using VECTOR4S = VECTOR<int, 4>;
using VECTOR4U = VECTOR<uint32_t, 4>;

static_assert(sizeof(VECTOR4) == 16, "VECTOR4 must be tightly packed");
static_assert(alignof(VECTOR4) == 16, "VECTOR4 must be 16-byte aligned for SIMD");
static_assert(PodT<VECTOR4>, "VECTOR4 must be POD type.");

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator-(
    const VECTOR4 _v)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Negate(_v.v)));
    return VECTOR4 { -_v[0], -_v[1], -_v[2], -_v[3] };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator+(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Add(_x.v, _y.v)));
    return VECTOR4 { _x[0] + _y[0], _x[1] + _y[1], _x[2] + _y[2], _x[3] + _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator-(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Sub(_x.v, _y.v)));
    return VECTOR4 { _x[0] - _y[0], _x[1] - _y[1], _x[2] - _y[2], _x[3] - _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator*(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Mult(_x.v, _y.v)));
    return VECTOR4 { _x[0] * _y[0], _x[1] * _y[1], _x[2] * _y[2], _x[3] * _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator/(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Div(_x.v, _y.v)));
    return VECTOR4 { _x[0] / _y[0], _x[1] / _y[1], _x[2] / _y[2], _x[3] / _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator*(
    const VECTOR4 _v,
    const float   _scalar)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Scale(_v.v, _scalar)));
    return VECTOR4 { _v[0] * _scalar, _v[1] * _scalar, _v[2] * _scalar, _v[3] * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator*(
    const float   _scalar,
    const VECTOR4 _v)
{
    return _v * _scalar;
}

// 역수 곱. constexpr 과 런타임이 같은 결과를 내도록 SIMD 경로도 역수를 곱한다.
[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator/(
    const VECTOR4 _v,
    const float   _scalar)
{
    return _v * (1.f / _scalar);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 JUG_VECTORCALL operator/(
    const float   _scalar,
    const VECTOR4 _v)
{
    JUG_SIMD_PATH(VECTOR4::MakeFromM128(simd::Div(simd::SetAll(_scalar), _v.v)));
    return VECTOR4 { _scalar / _v[0], _scalar / _v[1], _scalar / _v[2], _scalar / _v[3] };
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator==(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    JUG_SIMD_PATH(simd::AllTrue(simd::CmpEq(_x.v, _y.v)));
    return _x[0] == _y[0] && _x[1] == _y[1] && _x[2] == _y[2] && _x[3] == _y[3];   // NOLINT
}

[[nodiscard]] JUG_MATH_API constexpr bool JUG_VECTORCALL operator!=(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return !(_x == _y);
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator+=(
    const VECTOR4 _other)
{
    *this = *this + _other;
    return *this;
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator-=(
    const VECTOR4 _other)
{
    *this = *this - _other;
    return *this;
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator*=(
    const VECTOR4 _other)
{
    *this = *this * _other;
    return *this;
}

JUG_MATH_API constexpr VECTOR4& VECTOR4::operator/=(
    const VECTOR4 _other)
{
    *this = *this / _other;
    return *this;
}

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
