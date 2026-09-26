#pragma once
#include "SIMD.h"
#include "Vector.h"

namespace jug
{

// 연산 타입 (DirectXMath XMVECTOR). 멤버가 simd::M128 하나라 JUG_VECTORCALL 함수 사이를 레지스터로 오간다.
// 저장 타입과는 Load / Store 로만 오간다. N 이 안 쓰는 레인은 값이 정해져 있지 않고 모든 함수가 무시한다.
template<size_t N>
struct alignas(16) VECTOR_REG
{
    static_assert(N >= 2 && N <= 4, "VECTOR_REG supports 2 to 4 components");

    VECTOR_REG() = default;

    constexpr VECTOR_REG(
        const float _x,
        const float _y)
        requires(N == 2)
        : v(simd::Set(_x, _y, 0.f, 0.f))
    {
    }

    constexpr VECTOR_REG(
        const float _x,
        const float _y,
        const float _z)
        requires(N == 3)
        : v(simd::Set(_x, _y, _z, 0.f))
    {
    }

    constexpr VECTOR_REG(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
        requires(N == 4)
        : v(simd::Set(_x, _y, _z, _w))
    {
    }

    // 한 차원 작은 벡터 + 마지막 요소
    template<size_t M>
        requires(M + 1 == N)
    VECTOR_REG(
        const VECTOR_REG<M> _v,
        const float         _last)
        : v(simd::Insert<M>(_v.v, _last))
    {
    }

    // 뒤 요소를 버린다
    template<size_t M>
        requires(M > N)
    explicit constexpr VECTOR_REG(
        const VECTOR_REG<M> _v)
        : v(_v.v)
    {
    }

    explicit constexpr VECTOR_REG(
        const float _value)
        : v(simd::SetAll(_value))
    {
    }

    explicit constexpr VECTOR_REG(
        const simd::M128 _value)
        : v(_value)
    {
    }

    [[nodiscard]] float GetX() const
    {
        return simd::Get<0>(v);
    }

    [[nodiscard]] float GetY() const
    {
        return simd::Get<1>(v);
    }

    [[nodiscard]] float GetZ() const
        requires(N >= 3)
    {
        return simd::Get<2>(v);
    }

    [[nodiscard]] float GetW() const
        requires(N == 4)
    {
        return simd::Get<3>(v);
    }

    void SetX(
        const float _value)
    {
        v = simd::Insert<0>(v, _value);
    }

    void SetY(
        const float _value)
    {
        v = simd::Insert<1>(v, _value);
    }

    void SetZ(
        const float _value)
        requires(N >= 3)
    {
        v = simd::Insert<2>(v, _value);
    }

    void SetW(
        const float _value)
        requires(N == 4)
    {
        v = simd::Insert<3>(v, _value);
    }

    // 런타임 인덱스는 메모리를 거친다 - 고정 인덱스는 GetX() ~ GetW()
    [[nodiscard]] float operator[](
        const size_t _index) const
    {
        return simd::GetLane(v, _index);
    }

    VECTOR_REG& operator+=(
        const VECTOR_REG _other)
    {
        v = simd::Add(v, _other.v);
        return *this;
    }

    VECTOR_REG& operator-=(
        const VECTOR_REG _other)
    {
        v = simd::Sub(v, _other.v);
        return *this;
    }

    VECTOR_REG& operator*=(
        const VECTOR_REG _other)
    {
        v = simd::Mult(v, _other.v);
        return *this;
    }

    VECTOR_REG& operator/=(
        const VECTOR_REG _other)
    {
        v = simd::Div(v, _other.v);
        return *this;
    }

    VECTOR_REG& operator*=(
        const float _scalar)
    {
        v = simd::Mult(v, simd::SetAll(_scalar));
        return *this;
    }

    VECTOR_REG& operator/=(
        const float _scalar)
    {
        v = simd::Div(v, simd::SetAll(_scalar));
        return *this;
    }

    constexpr static size_t kDim = N;

    simd::M128 v;
};

using VECTOR2_REG = VECTOR_REG<2>;
using VECTOR3_REG = VECTOR_REG<3>;
using VECTOR4_REG = VECTOR_REG<4>;

static_assert(sizeof(VECTOR4_REG) == 16 && alignof(VECTOR4_REG) == 16, "VECTOR_REG must be a single 16-byte register");

// =======================================================
//  Constants
// =======================================================

template<>
struct MathConstants<VECTOR2_REG>
{
    constexpr static VECTOR2_REG kZero  = VECTOR2_REG { 0.f, 0.f };
    constexpr static VECTOR2_REG kOne   = VECTOR2_REG { 1.f, 1.f };
    constexpr static VECTOR2_REG kRight = VECTOR2_REG { 1.f, 0.f };
    constexpr static VECTOR2_REG kUp    = VECTOR2_REG { 0.f, 1.f };
    constexpr static VECTOR2_REG kUnitX = VECTOR2_REG { 1.f, 0.f };
    constexpr static VECTOR2_REG kUnitY = VECTOR2_REG { 0.f, 1.f };
};

template<>
struct MathConstants<VECTOR3_REG>
{
    constexpr static VECTOR3_REG kZero    = VECTOR3_REG { 0.f, 0.f, 0.f };
    constexpr static VECTOR3_REG kOne     = VECTOR3_REG { 1.f, 1.f, 1.f };
    constexpr static VECTOR3_REG kRight   = VECTOR3_REG { 1.f, 0.f, 0.f };
    constexpr static VECTOR3_REG kUp      = VECTOR3_REG { 0.f, 1.f, 0.f };
    constexpr static VECTOR3_REG kForward = VECTOR3_REG { 0.f, 0.f, 1.f };
    constexpr static VECTOR3_REG kUnitX   = VECTOR3_REG { 1.f, 0.f, 0.f };
    constexpr static VECTOR3_REG kUnitY   = VECTOR3_REG { 0.f, 1.f, 0.f };
    constexpr static VECTOR3_REG kUnitZ   = VECTOR3_REG { 0.f, 0.f, 1.f };
};

template<>
struct MathConstants<VECTOR4_REG>
{
    constexpr static VECTOR4_REG kZero    = VECTOR4_REG { 0.f, 0.f, 0.f, 0.f };
    constexpr static VECTOR4_REG kOne     = VECTOR4_REG { 1.f, 1.f, 1.f, 1.f };
    constexpr static VECTOR4_REG kRight   = VECTOR4_REG { 1.f, 0.f, 0.f, 0.f };
    constexpr static VECTOR4_REG kUp      = VECTOR4_REG { 0.f, 1.f, 0.f, 0.f };
    constexpr static VECTOR4_REG kForward = VECTOR4_REG { 0.f, 0.f, 1.f, 0.f };
    constexpr static VECTOR4_REG kUnitX   = VECTOR4_REG { 1.f, 0.f, 0.f, 0.f };
    constexpr static VECTOR4_REG kUnitY   = VECTOR4_REG { 0.f, 1.f, 0.f, 0.f };
    constexpr static VECTOR4_REG kUnitZ   = VECTOR4_REG { 0.f, 0.f, 1.f, 0.f };
    constexpr static VECTOR4_REG kUnitW   = VECTOR4_REG { 0.f, 0.f, 0.f, 1.f };
};

// =======================================================
//  Load / Store
// =======================================================

[[nodiscard]] inline VECTOR2_REG JUG_VECTORCALL Load(
    const VECTOR2& _v)
{
    return VECTOR2_REG { simd::LoadFloat2(_v.GetPtr()) };
}

[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL Load(
    const VECTOR3& _v)
{
    return VECTOR3_REG { simd::LoadFloat3(_v.GetPtr()) };
}

[[nodiscard]] inline VECTOR4_REG JUG_VECTORCALL Load(
    const VECTOR4& _v)
{
    return VECTOR4_REG { simd::LoadAligned(_v.GetPtr()) };
}

[[nodiscard]] inline VECTOR2 JUG_VECTORCALL Store(
    const VECTOR2_REG _v)
{
    VECTOR2 ret;
    simd::StoreFloat2(ret.GetPtr(), _v.v);
    return ret;
}

[[nodiscard]] inline VECTOR3 JUG_VECTORCALL Store(
    const VECTOR3_REG _v)
{
    VECTOR3 ret;
    simd::StoreFloat3(ret.GetPtr(), _v.v);
    return ret;
}

[[nodiscard]] inline VECTOR4 JUG_VECTORCALL Store(
    const VECTOR4_REG _v)
{
    VECTOR4 ret;
    simd::StoreAligned(ret.GetPtr(), _v.v);
    return ret;
}

// =======================================================
//  Operators
// =======================================================

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator-(
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Negate(_v.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator+(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Add(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator-(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Sub(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator*(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Mult(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator/(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Div(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator*(
    const VECTOR_REG<N> _v,
    const float         _scalar)
{
    return VECTOR_REG<N> { simd::Mult(_v.v, simd::SetAll(_scalar)) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator*(
    const float         _scalar,
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Mult(_v.v, simd::SetAll(_scalar)) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator/(
    const VECTOR_REG<N> _v,
    const float         _scalar)
{
    return VECTOR_REG<N> { simd::Div(_v.v, simd::SetAll(_scalar)) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL operator/(
    const float         _scalar,
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Div(simd::SetAll(_scalar), _v.v) };
}

template<size_t N>
[[nodiscard]] bool JUG_VECTORCALL operator==(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    constexpr int kMask = (1 << N) - 1;
    return (simd::MoveMask(simd::CmpEq(_x.v, _y.v)) & kMask) == kMask;
}

template<size_t N>
[[nodiscard]] bool JUG_VECTORCALL operator!=(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return !(_x == _y);
}

// =======================================================
//  Method
// =======================================================

namespace vector_reg_detail
{
    // 내적을 모든 레인에
    template<size_t N>
    [[nodiscard]] simd::M128 JUG_VECTORCALL DotV(
        const simd::M128 _x,
        const simd::M128 _y)
    {
        if constexpr (N == 2)
        {
            return simd::Dot2V(_x, _y);
        }
        else if constexpr (N == 3)
        {
            return simd::Dot3V(_x, _y);
        }
        else
        {
            return simd::Dot4V(_x, _y);
        }
    }
}   // namespace vector_reg_detail

template<size_t N>
[[nodiscard]] float JUG_VECTORCALL Dot(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return simd::GetX(vector_reg_detail::DotV<N>(_x.v, _y.v));
}

template<size_t N>
[[nodiscard]] float JUG_VECTORCALL LengthSq(
    const VECTOR_REG<N> _v)
{
    return Dot(_v, _v);
}

template<size_t N>
[[nodiscard]] float JUG_VECTORCALL Length(
    const VECTOR_REG<N> _v)
{
    return simd::GetX(simd::Sqrt(vector_reg_detail::DotV<N>(_v.v, _v.v)));
}

// 길이가 0 이면 0 (DirectXMath 와 같은 의미)
template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Normalize(
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::NormalizeBy(_v.v, vector_reg_detail::DotV<N>(_v.v, _v.v)) };
}

template<size_t N>
[[nodiscard]] bool JUG_VECTORCALL IsNormalized(
    const VECTOR_REG<N> _v)
{
    return IsEqualApprox(LengthSq(_v), 1.f);
}

template<size_t N>
[[nodiscard]] float JUG_VECTORCALL DistanceSq(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return LengthSq(_x - _y);
}

template<size_t N>
[[nodiscard]] float JUG_VECTORCALL Distance(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return Length(_x - _y);
}

[[nodiscard]] inline float JUG_VECTORCALL Cross(
    const VECTOR2_REG _x,
    const VECTOR2_REG _y)
{
    const simd::M128 m = simd::Mult(_x.v, simd::Shuffle<1, 0, 1, 0>(_y.v));   // x0*y1 x1*y0
    return simd::GetX(simd::Sub(m, simd::Shuffle<1, 1, 1, 1>(m)));
}

[[nodiscard]] inline VECTOR3_REG JUG_VECTORCALL Cross(
    const VECTOR3_REG _x,
    const VECTOR3_REG _y)
{
    return VECTOR3_REG { simd::Cross3(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Min(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Min(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Max(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y)
{
    return VECTOR_REG<N> { simd::Max(_x.v, _y.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Clamp(
    const VECTOR_REG<N> _v,
    const VECTOR_REG<N> _min,
    const VECTOR_REG<N> _max)
{
    return VECTOR_REG<N> { simd::Max(_min.v, simd::Min(_max.v, _v.v)) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Saturate(
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Max(simd::Zero(), simd::Min(simd::SetAll(1.f), _v.v)) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Abs(
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Abs(_v.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Rcp(
    const VECTOR_REG<N> _v)
{
    return VECTOR_REG<N> { simd::Div(simd::SetAll(1.f), _v.v) };
}

template<size_t N>
[[nodiscard]] VECTOR_REG<N> JUG_VECTORCALL Lerp(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y,
    const float         _t)
{
    return VECTOR_REG<N> { simd::Lerp(_x.v, _y.v, simd::SetAll(_t)) };
}

template<size_t N>
[[nodiscard]] bool JUG_VECTORCALL IsZeroApprox(
    const VECTOR_REG<N> _v,
    const float         _epsilon = kEpsilon)
{
    constexpr int kMask = (1 << N) - 1;
    return (simd::MoveMask(simd::CmpLt(simd::Abs(_v.v), simd::SetAll(_epsilon))) & kMask) == kMask;
}

template<size_t N>
[[nodiscard]] bool JUG_VECTORCALL IsEqualApprox(
    const VECTOR_REG<N> _x,
    const VECTOR_REG<N> _y,
    const float         _epsilon = kEpsilon)
{
    return IsZeroApprox(_x - _y, _epsilon);
}

}   // namespace jug
