#pragma once
#include <numbers>

#include "MathMacros.h"
#include "TypeTraits.h"

namespace jug
{

// ==========================================
//  Float Constants
// ==========================================

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

// ==========================================
//  Constants
// ==========================================

template<typename T>
struct MathConstants
{
};

#define JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(_type)                     \
    template<>                                                               \
    struct MathConstants<_type>                                              \
    {                                                                        \
        constexpr static _type kMax  = std::numeric_limits<_type>::max();    \
        constexpr static _type kMin  = std::numeric_limits<_type>::lowest(); \
        constexpr static _type kZero = _type { 0 };                          \
        constexpr static _type kOne  = _type { 1 };                          \
    }

JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(int8_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint8_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(int16_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint16_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(int32_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint32_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(int64_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint64_t);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(float);
JUG_MATH_DEFINE_ARITHMETIC_MATH_CONSTANTS(double);

#define JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(_constant)    \
    template<typename T>                                      \
        requires requires { MathConstants<T>::k##_constant; } \
    [[nodiscard]] JUG_MATH_API constexpr T _constant()        \
    {                                                         \
        return MathConstants<T>::k##_constant;                \
    }   // namespace jug

#define JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(_constant, _base) \
    template<typename T>                                                  \
        requires requires { MathConstants<T>::k##_constant; }             \
    [[nodiscard]] JUG_MATH_API constexpr T _constant()                    \
    {                                                                     \
        return -_base<T>();                                               \
    }

JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Zero);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(One);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Right);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Left, Right);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Up);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Down, Up);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Forward);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Backward, Forward);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitX);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitY);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitZ);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitW);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Max);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Min);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Identity);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Unit);
JUG_MATH_DEFINE_MATH_CONSTANTS_SHORTCUT(Universe);

// ==========================================
//  Type Safe Comparison
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
//  Constexpr Math
// =========================================

template<typename T, typename U = std::type_identity_t<T>>
[[nodiscard]] JUG_MATH_API constexpr T Min(const T& _x, const U& _y);

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Min(const T& _x, const Us&... _ys);

template<typename T, typename U = std::type_identity_t<T>>
[[nodiscard]] JUG_MATH_API constexpr T Max(T _x, U _y);

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Max(const T& _x, const Us&... _ys);

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

// =========================================
//  Constexpr Integral Math
// =========================================

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Ceil(T _value, T _multiple);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Floor(T _value, T _multiple);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Round(T _value, T _multiple);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr bool IsPowerOf2(T _value);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T CeilPowerOf2(T _value);

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T FloorPowerOf2(T _value);

// =========================================
//  Constexpr Float Math
// =========================================

[[nodiscard]] JUG_MATH_API constexpr float ToDeg(float _rad);
[[nodiscard]] JUG_MATH_API constexpr float ToRad(float _deg);
[[nodiscard]] JUG_MATH_API constexpr bool  IsZeroApprox(float _x);
[[nodiscard]] JUG_MATH_API constexpr bool  IsEqualApprox(float _x, float _y);
[[nodiscard]] JUG_MATH_API constexpr float Trunc(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Fract(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Floor(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Ceil(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Round(float _x);
[[nodiscard]] JUG_MATH_API constexpr bool  IsNan(float _f);
[[nodiscard]] JUG_MATH_API constexpr bool  IsFinite(float _f);
[[nodiscard]] JUG_MATH_API constexpr bool  IsInfinite(float _f);
[[nodiscard]] JUG_MATH_API constexpr float RcpSafe(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Lerp(float _x, float _y, float _t);
[[nodiscard]] JUG_MATH_API constexpr float InvLerp(float _x, float _y, float _value);
[[nodiscard]] JUG_MATH_API constexpr float SmoothStep(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Bias(float _time, float _bias);
[[nodiscard]] JUG_MATH_API constexpr float Gain(float _time, float _gain);
[[nodiscard]] JUG_MATH_API constexpr float LogCore(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Exp(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Pow(float _x, float _y);
[[nodiscard]] JUG_MATH_API constexpr float Sqrt(float _x);
[[nodiscard]] JUG_MATH_API constexpr float RSqrt(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Saturate(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Sin(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Cos(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Tan(float _x);
[[nodiscard]] JUG_MATH_API constexpr float ACos(float _x);
[[nodiscard]] JUG_MATH_API constexpr float ASin(float _x);
[[nodiscard]] JUG_MATH_API constexpr float ATan2(float _y, float _x);
[[nodiscard]] JUG_MATH_API constexpr float ATan(float _x);
[[nodiscard]] JUG_MATH_API constexpr float AngleDiff(float _x, float _y);
[[nodiscard]] JUG_MATH_API constexpr float AngleLerp(float _x, float _y, float _t);

}   // namespace jug

#include "Math.inl"