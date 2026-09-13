#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>

#include "TypeTraits.h"

namespace jug
{

// ===========================================
//  Macro
// ===========================================

#ifdef __CUDACC__
#    define JUG_MATH_API __host__ __device__
#else
#    define JUG_MATH_API
#endif

// ==========================================
//   Constants
// ==========================================

template<typename T>
struct MathConstants;

#define JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(_type)                     \
    template<>                                                               \
    struct MathConstants<_type>                                              \
    {                                                                        \
        constexpr static _type kMax  = std::numeric_limits<_type>::max();    \
        constexpr static _type kMin  = std::numeric_limits<_type>::lowest(); \
        constexpr static _type kZero = _type { 0 };                          \
        constexpr static _type kOne  = _type { 1 };                          \
    }

JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(int8_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint8_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(int16_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint16_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(int32_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint32_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(int64_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(uint64_t);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(float);
JUG_DEFINE_ARITHMETIC_MATH_CONSTANTS(double);

#define JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(_constant)    \
    template<typename T>                                      \
        requires requires { MathConstants<T>::k##_constant; } \
    [[nodiscard]] JUG_MATH_API constexpr T _constant()        \
    {                                                         \
        return MathConstants<T>::k##_constant;                \
    }   // namespace jug

#define JUG_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(_constant, _base) \
    template<typename T>                                                  \
        requires requires { MathConstants<T>::k##_constant; }             \
    [[nodiscard]] JUG_MATH_API constexpr T _constant()                    \
    {                                                                     \
        return -_base<T>();                                               \
    }

JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Zero);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(One);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Right);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Left, Right);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Up);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Down, Up);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Forward);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT_INVERSE(Backward, Forward);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitX);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitY);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitZ);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(UnitW);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Max);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Min);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Identity);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Unit);
JUG_DEFINE_MATH_CONSTANTS_SHORTCUT(Universe);

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
//  Float
// =========================================

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
[[nodiscard]] JUG_MATH_API constexpr float Log(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Log2(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Exp(float _x);
[[nodiscard]] JUG_MATH_API constexpr float Exp2(float _x);
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