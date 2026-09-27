#pragma once
#include "Vector2.h"

namespace jug
{

// ==========================================
//  Comparison
// ==========================================

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqual(
    T _x,
    U _y)
{
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    {
        using CommonT = std::common_type_t<T, U>;
        return static_cast<CommonT>(_x) == static_cast<CommonT>(_y);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        return (_x >= 0) && (static_cast<std::make_unsigned_t<T>>(_x) == static_cast<std::make_unsigned_t<U>>(_y));
    }
    else
    {
        return (_y >= 0) && (static_cast<std::make_unsigned_t<T>>(_x) == static_cast<std::make_unsigned_t<U>>(_y));
    }
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsNotEqual(
    T _x,
    U _y)
{
    return !IsEqual(_x, _y);
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsLess(
    T _x,
    U _y)
{
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    {
        using CommonT = std::common_type_t<T, U>;
        return static_cast<CommonT>(_x) < static_cast<CommonT>(_y);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        return (_x < 0) || (static_cast<std::make_unsigned_t<T>>(_x) < static_cast<std::make_unsigned_t<U>>(_y));
    }
    else
    {
        return (_y > 0) && (static_cast<std::make_unsigned_t<T>>(_x) < static_cast<std::make_unsigned_t<U>>(_y));
    }
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsGreater(
    T _x,
    U _y)
{
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
    {
        using CommonT = std::common_type_t<T, U>;
        return static_cast<CommonT>(_x) > static_cast<CommonT>(_y);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        return (_x > 0) && (static_cast<std::make_unsigned_t<T>>(_x) > static_cast<std::make_unsigned_t<U>>(_y));
    }
    else
    {
        return (_y < 0) || (static_cast<std::make_unsigned_t<T>>(_x) > static_cast<std::make_unsigned_t<U>>(_y));
    }
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsLessEqual(
    T _x,
    U _y)
{
    return !IsGreater(_x, _y);
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsGreaterEqual(
    T _x,
    U _y)
{
    return !IsLess(_x, _y);
}

// =========================================
//  Basic
// =========================================

template<typename T>
[[nodiscard]] JUG_MATH_API constexpr T Min(
    const T                       _x,
    const std::type_identity_t<T> _y)
{
    return (_x < _y) ? _x : static_cast<T>(_y);
}

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Min(
    const T                       _x,
    const std::type_identity_t<T> _y,
    Us... _args)
{
    return Min(_x, Min(_y, _args...));
}

template<typename T>
[[nodiscard]] JUG_MATH_API constexpr T Max(
    T                       _x,
    std::type_identity_t<T> _y)
{
    return (_x > _y) ? _x : static_cast<T>(_y);
}

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Max(
    const T                       _x,
    const std::type_identity_t<T> _y,
    Us... _args)
{
    return Max(_x, Max(_y, _args...));
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Clamp(
    T _x,
    T _min,
    T _max)
{
    return _x < _min ? _min : (_x > _max ? _max : _x);
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Abs(
    T _x)
{
    if constexpr (std::is_same_v<T, float>)
    {
        return std::bit_cast<float>(std::bit_cast<uint32_t>(_x) & ~kFloatSignMask);
    }
    else
    {
        return _x < Zero<T>() ? -_x : _x;
    }
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Sign(
    T _x)
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return _x != Zero<T>();
    }
    else
    {
        return static_cast<T>(Zero<T>() < _x) - static_cast<T>(Zero<T>() > _x);
    }
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr bool SignBit(
    T _x)
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return false;
    }
    else
    {
        return _x == -Zero<T>() ? _x != Zero<T>() : _x < Zero<T>();   // NOLINT
    }
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T CopySign(
    T _x,
    T _y)
{
    static_assert(std::is_same_v<double, T> == false, "CopySign is not implemented for double yet.");

    if constexpr (std::is_same_v<T, float>)
    {
        const uint32_t magnitude = std::bit_cast<uint32_t>(_x) & ~kFloatSignMask;
        const uint32_t sign      = std::bit_cast<uint32_t>(_y) & kFloatSignMask;
        return std::bit_cast<float>(magnitude | sign);
    }
    else
    {
        if constexpr (std::is_signed_v<T>)
        {
            const T mask = _y >> (sizeof(T) * 8 - 1);
            return (Abs(_x) ^ mask) - mask;
        }
        else
        {
            return _x;
        }
    }
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Mod(
    T _x,
    T _y)
{
    if constexpr (std::is_integral_v<T>)
    {
        return _x % _y;
    }
    else
    {
        return _x - _y * Floor(_x / _y);
    }
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Wrap(
    T _x,
    T _wrap)
{
    const T tmp = Mod(_x, _wrap);
    return tmp < T { 0 } ? _wrap + tmp : tmp;
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Step(
    T _edge,
    T _x)
{
    return _x < _edge ? T { 0 } : T { 1 };
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Pulse(
    T _x,
    T _start,
    T _end)
{
    return Step(_start, _x) - Step(_end, _x);
}

// ===========================================
//  Integral
// ===========================================

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Floor(
    T _x,
    T _multiple)
{
    T remainder = _x % _multiple;
    return remainder == T(0) ? _x : _x - remainder - (remainder < T(0) ? _multiple : T(0));
}

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Ceil(
    T _x,
    T _multiple)
{
    T remainder = _x % _multiple;
    return remainder == T(0) ? _x : _x - remainder + (remainder > T(0) ? _multiple : T(0));
}

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Round(
    T _x,
    T _multiple)
{
    T f    = Floor(_x, _multiple);
    T diff = _x - f;
    return diff >= (_multiple + T(1)) / T(2) ? f + _multiple : f;
}

// =========================================
//  Float
// =========================================

[[nodiscard]] JUG_MATH_API constexpr float ToDeg(
    const float _rad)
{
    return _rad * kRad2Deg;
}

[[nodiscard]] JUG_MATH_API constexpr float ToRad(
    const float _deg)
{
    return _deg * kDeg2Rad;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(
    const float _x,
    const float _epsilon)
{
    return Abs(_x) < _epsilon;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const float _x,
    const float _y,
    const float _epsilon)
{
    return IsZeroApprox(_x - _y, _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr float Trunc(
    const float _x)
{
    return static_cast<float>(static_cast<int>(_x));
}

[[nodiscard]] JUG_MATH_API constexpr float Fract(
    const float _x)
{
    return _x - Trunc(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Floor(
    const float _x)
{
    if (_x < 0.f)
    {
        const float fr = Fract(-_x);
        const float tr = Trunc(-_x);
        return -tr - (fr != 0.f ? 1.f : 0.f);
    }
    else
    {
        return Trunc(_x);
    }
}

[[nodiscard]] JUG_MATH_API constexpr float Ceil(
    const float _x)
{
    return -Floor(-_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Round(
    const float _x)
{
    return _x < 0.f ? Ceil(_x - 0.5f) : Floor(_x + 0.5f);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNan(
    const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp > kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFinite(
    const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp < kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsInfinite(
    const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp == kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr float RcpSafe(
    const float _x)
{
    return 1.f / CopySign(Max(kFloatSmallest, Abs(_x)), _x);
}

[[nodiscard]] JUG_MATH_API constexpr float Lerp(
    const float _x,
    const float _y,
    const float _t)
{
    return _x + (_y - _x) * _t;
}

[[nodiscard]] JUG_MATH_API constexpr float InvLerp(
    const float _x,
    const float _y,
    const float _value)
{
    return (_value - _x) / (_y - _x);
}

[[nodiscard]] JUG_MATH_API constexpr float SmoothStep(
    const float _x)
{
    return _x * _x * (3.f - 2.f * _x);
}

[[nodiscard]] JUG_MATH_API constexpr float Bias(
    const float _time,
    const float _bias)
{
    return _time / ((1.f / _bias - 2.f) * (1.f - _time) + 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr float Gain(
    const float _time,
    const float _gain)
{
    if (_time < 0.5f)
    {
        return Bias(_time * 2.f, _gain) * 0.5f;
    }
    return Bias(_time * 2.f - 1.f, 1.f - _gain) * 0.5f + 0.5f;
}

namespace math_detail
{

    [[nodiscard]] JUG_MATH_API constexpr float Log(
        const float _x)
    {
        if (_x < 0.f)
        {
            return std::bit_cast<float>(kFloatSignMask | kFloatExponentMask | kFloatMantissaMask);
        }

        if (_x == 0.f)
        {
            return -kFloatInfinity;
        }

        const uint32_t ftob    = std::bit_cast<uint32_t>(_x);
        const int      exp0    = static_cast<int>((ftob & kFloatExponentMask) >> 23) - 0x7e;
        const uint32_t masked1 = (ftob & (kFloatSignMask | kFloatMantissaMask)) | 0x3f000000u;
        float          ff      = std::bit_cast<float>(masked1);
        int            exp     = exp0;
        if (ff < kSqrt2 * 0.5f)
        {
            ff *= 2.f;
            --exp;
        }

        constexpr float kC0     = 6.666666666666735130e-01f;
        constexpr float kC1     = 3.999999999940941908e-01f;
        constexpr float kC2     = 2.857142874366239149e-01f;
        constexpr float kC3     = 2.222219843214978396e-01f;
        constexpr float kC4     = 1.818357216161805012e-01f;
        constexpr float kC5     = 1.531383769920937332e-01f;
        constexpr float kC6     = 1.479819860511658591e-01f;
        constexpr float kNat2Lo = 1.90821492927058770002e-10f;

        ff -= 1.f;
        const float kk   = static_cast<float>(exp);
        const float hi   = kk * kLogNat2;
        const float lo   = kk * kNat2Lo;
        const float ss   = ff / (2.f + ff);
        const float s2   = ss * ss;
        const float s4   = s2 * s2;
        const float t1   = s2 * (((kC6 * s4 + kC4) * s4 + kC2) * s4 + kC0);
        const float t2   = s4 * ((kC5 * s4 + kC3) * s4 + kC1);
        const float hfsq = 0.5f * ff * ff;
        return hi - ((hfsq - (ss * (hfsq + t1 + t2) + lo)) - ff);
    }

    [[nodiscard]] JUG_MATH_API constexpr float Exp(
        const float _x)
    {
        if (Abs(_x) <= kEpsilon)
        {
            return _x + 1.f;
        }

        if (_x <= Log(kFloatSmallest))
        {
            return 0.f;
        }

        if (IsInfinite(_x) && _x < 0.f)
        {
            return 0.f;
        }

        constexpr float kC0     = 1.66666666666666019037e-01f;
        constexpr float kC1     = -2.77777777770155933842e-03f;
        constexpr float kC2     = 6.61375632143793436117e-05f;
        constexpr float kC3     = -1.65339022054652515390e-06f;
        constexpr float kC4     = 4.13813679705723846039e-08f;
        constexpr float kNat2Lo = 1.90821492927058770002e-10f;

        const float kk    = Round(_x * kInvLogNat2);
        const float hi    = _x - kk * kLogNat2;
        const float lo    = kk * kNat2Lo;
        const float hml   = hi - lo;
        const float hmlsq = hml * hml;
        const float tmp   = hml - hmlsq * ((((kC4 * hmlsq + kC3) * hmlsq + kC2) * hmlsq + kC1) * hmlsq + kC0);
        const float tmp5  = hml * tmp / (2.f - tmp);
        const float tmp6  = 1.f - ((lo - tmp5) - hi);

        const uint32_t ftob    = std::bit_cast<uint32_t>(tmp6);
        const uint32_t expbits = static_cast<uint32_t>(static_cast<int>((ftob & kFloatExponentMask) >> 23) + static_cast<int>(kk));
        const uint32_t ret     = (ftob & ~kFloatExponentMask) | ((expbits & 0xffu) << 23);
        return std::bit_cast<float>(ret);
    }

}   // namespace math_detail

[[nodiscard]] JUG_MATH_API constexpr float Log(
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return math_detail::Log(_x);
    }

    return ::logf(_x);
}

[[nodiscard]] constexpr JUG_MATH_API float Log2(
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return math_detail::Log(_x) * kInvLogNat2;
    }

    return ::log2f(_x);
}

[[nodiscard]] constexpr JUG_MATH_API float Exp2(
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return Pow(2.f, _x);
    }

    return ::exp2f(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Exp(
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return math_detail::Exp(_x);
    }

    return ::expf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Pow(
    const float _x,
    const float _y)
{
    if (std::is_constant_evaluated())
    {
        if (Abs(_y) < kFloatSmallest)
        {
            return 1.f;
        }

        if (Abs(_x) < kFloatSmallest)
        {
            return 0.f;
        }

        return math_detail::Exp(_y * math_detail::Log(Abs(_x)));
    }

    return ::powf(_x, _y);
}

[[nodiscard]] JUG_MATH_API constexpr float Sqrt(
    const float _x)
{
    return simd::Sqrt(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float RSqrt(
    const float _x)
{
    if (_x < kFloatSmallest)
    {
        return kFloatInfinity;
    }
    return simd::RSqrt(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Saturate(
    const float _x)
{
    return Clamp(_x, 0.f, 1.f);
}

namespace math_detail
{
    [[nodiscard]] JUG_MATH_API constexpr float ACos(
        const float _x)
    {
        constexpr float c0 = 1.5707963050f;
        constexpr float c1 = -0.2145988016f;
        constexpr float c2 = 0.0889789874f;
        constexpr float c3 = -0.0501743046f;
        constexpr float c4 = 0.0308918810f;
        constexpr float c5 = -0.0170881256f;
        constexpr float c6 = 0.0066700901f;
        constexpr float c7 = -0.0012624911f;

        const float absa   = Abs(_x);
        const float absa2  = absa * absa;
        const float absa4  = absa2 * absa2;
        const float tmp    = ((c7 * absa + c6) * absa2 + (c5 * absa + c4)) * absa4 + ((c3 * absa + c2) * absa2 + (c1 * absa + c0));
        const float tmp2   = tmp * Sqrt(Max(1.f - absa, 0.f));
        const float negate = static_cast<float>(_x < 0.f);
        const float tmp3   = tmp2 - 2.f * negate * tmp2;
        return negate * kPI + tmp3;
    }

    [[nodiscard]] JUG_MATH_API constexpr float ATan2(
        const float _y,
        const float _x)
    {
        constexpr float kTanPiOver8 = 0.41421356237f;

        const float ax     = Abs(_x);
        const float ay     = Abs(_y);
        const float maxaxy = Max(ax, ay);
        const float minaxy = Min(ax, ay);

        if (maxaxy == 0.f)
        {
            return _y < 0.f ? -0.f : 0.f;
        }

        const float mxy    = minaxy / maxaxy;
        float       z      = mxy;
        float       offset = 0.f;
        if (mxy > kTanPiOver8)
        {
            z      = (mxy - 1.f) / (mxy + 1.f);
            offset = kHalfPI * 0.5f;
        }

        const float z2   = z * z;
        const float poly = ((((((z2 / 13.f - 1.f / 11.f) * z2 + 1.f / 9.f) * z2 - 1.f / 7.f) * z2 + 1.f / 5.f) * z2 - 1.f / 3.f) * z2 + 1.f);
        const float tmp  = z * poly + offset;
        const float tmp6 = ay > ax ? kHalfPI - tmp : tmp;
        const float tmp7 = _x < 0.f ? kPI - tmp6 : tmp6;
        return _y < 0.f ? -tmp7 : tmp7;
    }
}   // namespace math_detail

[[nodiscard]] JUG_MATH_API constexpr float Sin(
    const float _x)
{
    return simd::Get<0>(simd::Sin(simd::SetAll(_x)));
}

[[nodiscard]] JUG_MATH_API constexpr float Cos(
    const float _x)
{
    return simd::Get<0>(simd::Cos(simd::SetAll(_x)));
}

JUG_MATH_API constexpr SIN_COS_RESULT SinCos(
    const float _x)
{
    const simd::M128 sc = simd::SinCos1(_x);
    return { simd::Get<0>(sc), simd::Get<1>(sc) };
}

[[nodiscard]] JUG_MATH_API constexpr float Tan(
    const float _x)
{
    const auto [s, c] = SinCos(_x);
    return s / c;
}

[[nodiscard]] JUG_MATH_API constexpr float ACos(
    const float _x)
{
    return math_detail::ACos(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float ASin(
    const float _x)
{
    return kHalfPI - math_detail::ACos(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float ATan2(
    const float _y,
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return math_detail::ATan2(_y, _x);
    }

    return ::atan2f(_y, _x);
}

[[nodiscard]] JUG_MATH_API constexpr float ATan(
    const float _x)
{
    if (std::is_constant_evaluated())
    {
        return math_detail::ATan2(_x, 1.f);
    }

    return ::atanf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float AngleDiff(
    const float _x,
    const float _y)
{
    const float dist = Wrap(_y - _x, k2PI);
    return Wrap(dist * 2.f, k2PI) - dist;
}

[[nodiscard]] JUG_MATH_API constexpr float AngleLerp(
    const float _x,
    const float _y,
    const float _t)
{
    return _x + AngleDiff(_x, _y) * _t;
}

}   // namespace jug
