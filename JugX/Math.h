#pragma once
#include <cfloat>
#include <numbers>

#include "SIMD.h"
#include "TypeTraits.h"

namespace jug
{

// ===========================================
//  Constants
// ===========================================

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Zero()
{
    return T { 0 };
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T One()
{
    return T { 1 };
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Max()
{
    return std::numeric_limits<T>::max();
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Min()
{
    return std::numeric_limits<T>::lowest();
}

#define MATH_CONSTANTS(_constant)                      \
    template<typename T>                               \
        requires requires { T::_constant(); }          \
    [[nodiscard]] JUG_MATH_API constexpr T _constant() \
    {                                                  \
        return T::_constant();                         \
    }

MATH_CONSTANTS(Zero)
MATH_CONSTANTS(One)
MATH_CONSTANTS(Right)
MATH_CONSTANTS(Up)
MATH_CONSTANTS(Forward)
MATH_CONSTANTS(UnitX)
MATH_CONSTANTS(UnitY)
MATH_CONSTANTS(UnitZ)
MATH_CONSTANTS(UnitW)
MATH_CONSTANTS(Max)
MATH_CONSTANTS(Min)
MATH_CONSTANTS(Identity)
MATH_CONSTANTS(Unit)
MATH_CONSTANTS(Universe)

#undef MATH_CONSTANTS

#define MATH_CONSTANTS_INV(_constant, _base)           \
    template<typename T>                               \
        requires requires { T::_base(); }              \
    [[nodiscard]] JUG_MATH_API constexpr T _constant() \
    {                                                  \
        return -T::_base();                            \
    }

MATH_CONSTANTS_INV(Left, Right)
MATH_CONSTANTS_INV(Down, Up)
MATH_CONSTANTS_INV(Backward, Forward)

#undef MATH_CONSTANTS_INV

// ===========================================
//  Structs
// ===========================================

struct SIN_COS
{
    float sin;
    float cos;
};

// ==========================================
//  Comparison
// ==========================================

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqual(T _x, U _y);

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsNotEqual(T _x, U _y);

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsLess(T _x, U _y);

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsGreater(T _x, U _y);

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsLessEqual(T _x, U _y);

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsGreaterEqual(T _x, U _y);

// =========================================
//  Basic
// =========================================

template<typename T>
[[nodiscard]] JUG_MATH_API constexpr T Min(T _x, std::type_identity_t<T> _y);

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Min(T _x, std::type_identity_t<T> _y, Us... _args);

template<typename T>
[[nodiscard]] JUG_MATH_API constexpr T Max(T _x, std::type_identity_t<T> _y);

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Max(T _x, std::type_identity_t<T> _y, Us... _args);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Clamp(T _x, T _min, T _max);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Abs(T _x);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Sign(T _x);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr bool SignBit(T _x);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T CopySign(T _x, T _y);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Mod(T _x, T _y);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Wrap(T _x, T _wrap);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Step(T _edge, T _x);

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Pulse(T _x, T _start, T _end);

// ===========================================
//  Integeral
// ===========================================

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Floor(T _x, T _multiple);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Ceil(T _x, T _multiple);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Round(T _x, T _multiple);

// =========================================
//  Float
// =========================================

constexpr uint32_t kFloatSignMask     = 0x80000000u;
constexpr uint32_t kFloatExponentMask = 0x7f800000u;
constexpr uint32_t kFloatMantissaMask = 0x007fffffu;
constexpr float    kFloatInfinity     = std::bit_cast<float>(kFloatExponentMask);
constexpr float    kFloatSmallest     = FLT_MIN;

constexpr static float kPI        = std::numbers::pi_v<float>;
constexpr static float k2PI       = 2.f * kPI;
constexpr static float kHalfPI    = kPI * 0.5f;
constexpr static float kInvPI     = 1.f / kPI;
constexpr static float kInv2PI    = 1.f / k2PI;
constexpr static float kInvHalfPI = 1.f / kHalfPI;

constexpr static float kDeg2Rad   = kPI / 180.f;
constexpr static float kRad2Deg   = 180.f / kPI;
constexpr static float kEpsilon   = 1e-5f;
constexpr static float kAlmostOne = 1.f - kEpsilon;

constexpr static float kSqrt2      = 1.41421356237309504880f;
constexpr static float kLogNat2    = 0.6931471805599453f;
constexpr static float kInvLogNat2 = 1.4426950408889634f;

[[nodiscard]] JUG_MATH_API constexpr float   ToDeg(float _rad);
[[nodiscard]] JUG_MATH_API constexpr float   ToRad(float _deg);
[[nodiscard]] JUG_MATH_API constexpr bool    IsZeroApprox(float _x, float _epsilon = kEpsilon);
[[nodiscard]] JUG_MATH_API constexpr bool    IsEqualApprox(float _x, float _y, float _epsilon = kEpsilon);
[[nodiscard]] JUG_MATH_API constexpr float   Trunc(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Fract(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Floor(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Ceil(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Round(float _x);
[[nodiscard]] JUG_MATH_API constexpr bool    IsNan(float _f);
[[nodiscard]] JUG_MATH_API constexpr bool    IsFinite(float _f);
[[nodiscard]] JUG_MATH_API constexpr bool    IsInfinite(float _f);
[[nodiscard]] JUG_MATH_API constexpr float   RcpSafe(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Lerp(float _x, float _y, float _t);
[[nodiscard]] JUG_MATH_API constexpr float   InvLerp(float _x, float _y, float _value);
[[nodiscard]] JUG_MATH_API constexpr float   SmoothStep(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Bias(float _time, float _bias);
[[nodiscard]] JUG_MATH_API constexpr float   Gain(float _time, float _gain);
[[nodiscard]] JUG_MATH_API constexpr float   Log(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Log2(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Exp(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Exp2(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Pow(float _x, float _y);
[[nodiscard]] JUG_MATH_API constexpr float   Sqrt(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   RSqrt(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Saturate(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Sin(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Cos(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   Tan(float _x);
[[nodiscard]] JUG_MATH_API constexpr SIN_COS SinCos(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   ACos(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   ASin(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   ATan2(float _y, float _x);
[[nodiscard]] JUG_MATH_API constexpr float   ATan(float _x);
[[nodiscard]] JUG_MATH_API constexpr float   AngleDiff(float _x, float _y);
[[nodiscard]] JUG_MATH_API constexpr float   AngleLerp(float _x, float _y, float _t);

}   // namespace jug

#include "Math.inl"
