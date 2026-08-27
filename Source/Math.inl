#pragma once

#define DT math_detail

namespace jug
{

namespace
{
    constexpr uint32_t kFloatSignMask     = 0x80000000u;
    constexpr uint32_t kFloatExponentMask = 0x7f800000u;
    constexpr uint32_t kFloatMantissaMask = 0x007fffffu;
    constexpr float    kFloatInfinity     = std::bit_cast<float>(kFloatExponentMask);
    constexpr float    kFloatSmallest     = FLT_MIN;
}

// ==========================================
//  Type Safe Comparison
// ==========================================

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsEqual(T _x, U _y)
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
[[nodiscard]] JUG_MATH_API constexpr bool IsNotEqual(T _x, U _y)
{
    return !IsEqual(_x, _y);
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsLess(T _x, U _y)
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
[[nodiscard]] JUG_MATH_API constexpr bool IsGreater(T _x, U _y)
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
[[nodiscard]] JUG_MATH_API constexpr bool IsLessEqual(T _x, U _y)
{
    return !IsGreater(_x, _y);
}

template<std::integral T, std::integral U>
[[nodiscard]] JUG_MATH_API constexpr bool IsGreaterEqual(T _x, U _y)
{
    return !IsLess(_x, _y);
}

// =========================================
//  Basic Constexpr Math
// =========================================

template<typename T, typename U>
[[nodiscard]] JUG_MATH_API constexpr T Min(const T& _x, const U& _y)
{
    return (_x < _y) ? _x : static_cast<T>(_y);
}

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Min(const T& _x, const Us&... _ys)
{
    return Min(_x, Min(_ys...));
}

template<typename T, typename U>
[[nodiscard]] JUG_MATH_API constexpr T Max(T _x, U _y)
{
    return (_x > _y) ? _x : static_cast<T>(_y);
}

template<typename T, typename... Us>
[[nodiscard]] JUG_MATH_API constexpr T Max(const T& _x, const Us&... _ys)
{
    return Max(_x, Max(_ys...));
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Clamp(T _x, T _min, T _max)
{
    return _x < _min ? _min : (_x > _max ? _max : _x);
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Abs(T _x)
{
    return _x < Zero<T>() ? -_x : _x;
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Sign(T _x)
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
[[nodiscard]] JUG_MATH_API constexpr bool SignBit(T _x)
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
[[nodiscard]] JUG_MATH_API constexpr T CopySign(T _x, T _y)
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
[[nodiscard]] JUG_MATH_API constexpr T Mod(T _x, T _y)
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
[[nodiscard]] JUG_MATH_API constexpr T Wrap(T _x, T _wrap)
{
    const T tmp = Mod(_x, _wrap);
    return tmp < T { 0 } ? _wrap + tmp : tmp;
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Step(T _edge, T _x)
{
    return _x < _edge ? T { 0 } : T { 1 };
}

template<ArithmeticT T>
[[nodiscard]] JUG_MATH_API constexpr T Pulse(T _x, T _start, T _end)
{
    return Step(_start, _x) - Step(_end, _x);
}

// =========================================
//  Constexpr Integral Math
// =========================================

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Ceil(T _value, T _multiple)
{
    return ((_value + _multiple - 1) / _multiple) * _multiple;
}

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Floor(T _value, T _multiple)
{
    if constexpr (IsPowerOf2(_multiple))
    {
        return _value & ~(_multiple - 1);
    }
    else
    {
        return (_value / _multiple) * _multiple;
    }
}

template<std::integral T>
[[nodiscard]] JUG_MATH_API constexpr T Round(T _value, T _multiple)
{
    return Ceil(_value + _multiple / 2, _multiple);
}

// =========================================
//  Constexpr Float Math
// =========================================

[[nodiscard]] JUG_MATH_API constexpr float ToDeg(const float _rad)
{
    return _rad * kRad2Deg;
}

[[nodiscard]] JUG_MATH_API constexpr float ToRad(const float _deg)
{
    return _deg * kDeg2Rad;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsZeroApprox(const float _x)
{
    return Abs(_x) < kEpsilon;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(const float _x, const float _y)
{
    return IsZeroApprox(_x - _y);
}

[[nodiscard]] JUG_MATH_API constexpr float Trunc(const float _x)
{
    return static_cast<float>(static_cast<int>(_x));
}

[[nodiscard]] JUG_MATH_API constexpr float Fract(const float _x)
{
    return _x - Trunc(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Floor(const float _x)
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

[[nodiscard]] JUG_MATH_API constexpr float Ceil(const float _x)
{
    return -Floor(-_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Round(const float _x)
{
    return _x < 0.f ? Ceil(_x - 0.5f) : Floor(_x + 0.5f);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsNan(const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp > kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFinite(const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp < kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsInfinite(const float _f)
{
    const uint32_t tmp = std::bit_cast<uint32_t>(_f) & Max<int32_t>();
    return tmp == kFloatExponentMask;
}

[[nodiscard]] JUG_MATH_API constexpr float RcpSafe(const float _x)
{
    return 1.f / CopySign(Max(kFloatSmallest, Abs(_x)), _x);
}

[[nodiscard]] JUG_MATH_API constexpr float Lerp(const float _x, const float _y, const float _t)
{
    return _x + (_y - _x) * _t;
}

[[nodiscard]] JUG_MATH_API constexpr float InvLerp(const float _x, const float _y, const float _value)
{
    return (_value - _x) / (_y - _x);
}

[[nodiscard]] JUG_MATH_API constexpr float SmoothStep(const float _x)
{
    return _x * _x * (3.f - 2.f * _x);
}

[[nodiscard]] JUG_MATH_API constexpr float Bias(const float _time, const float _bias)
{
    return _time / ((1.f / _bias - 2.f) * (1.f - _time) + 1.f);
}

[[nodiscard]] JUG_MATH_API constexpr float Gain(const float _time, const float _gain)
{
    if (_time < 0.5f)
    {
        return Bias(_time * 2.f, _gain) * 0.5f;
    }
    return Bias(_time * 2.f - 1.f, 1.f - _gain) * 0.5f + 0.5f;
}

namespace DT
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

}   // namespace DT

[[nodiscard]] JUG_MATH_API constexpr float Log(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::Log(_x);
    }

    return ::logf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Exp(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::Exp(_x);
    }

    return ::expf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Pow(const float _x, const float _y)
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

        return DT::Exp(_y * DT::Log(Abs(_x)));
    }

    return ::powf(_x, _y);
}

[[nodiscard]] JUG_MATH_API constexpr float Sqrt(const float _x)
{
    if (std::is_constant_evaluated())
    {
        if (_x < 0.f)
        {
            return std::bit_cast<float>(kFloatExponentMask | kFloatMantissaMask);
        }

        if (_x < kFloatSmallest)
        {
            return 0.f;
        }

        return _x * Pow(_x, -0.5f);
    }

#ifdef JUG_SIMD_AVAILABLE
    return simd::GetX(simd::Sqrt(simd::SetAll(_x)));
#else
    return ::sqrtf(_x);
#endif
}

[[nodiscard]] JUG_MATH_API constexpr float RSqrt(const float _x)
{
    if (_x < kFloatSmallest)
    {
        return kFloatInfinity;
    }

    if (std::is_constant_evaluated())
    {
        return Pow(_x, -0.5f);
    }

#ifdef JUG_SIMD_AVAILABLE
    return simd::GetX(simd::RSqrt(simd::SetAll(_x)));
#else
    return 1.f / ::sqrtf(_x);
#endif
}

[[nodiscard]] JUG_MATH_API constexpr float Saturate(const float _x)
{
    return Clamp(_x, 0.f, 1.f);
}

namespace DT
{
    [[nodiscard]] JUG_MATH_API constexpr float Cos(
        const float _x)
    {
        const float scaled = _x * 2.f * (1.f / kPI);
        const float real   = Floor(scaled);
        const float xx     = _x - real * kHalfPI;
        const int   bits   = static_cast<int>(real) & 3;

        constexpr float kSinC2  = -0.16666667163372039794921875f;
        constexpr float kSinC4  = 8.333347737789154052734375e-3f;
        constexpr float kSinC6  = -1.9842604524455964565277099609375e-4f;
        constexpr float kSinC8  = 2.760012648650445044040679931640625e-6f;
        constexpr float kSinC10 = -2.50293279435709337121807038784027099609375e-8f;

        constexpr float kCosC2  = -0.5f;
        constexpr float kCosC4  = 4.166664183139801025390625e-2f;
        constexpr float kCosC6  = -1.388833043165504932403564453125e-3f;
        constexpr float kCosC8  = 2.47562347794882953166961669921875e-5f;
        constexpr float kCosC10 = -2.59630184018533327616751194000244140625e-7f;

        float c0  = xx;
        float c2  = kSinC2;
        float c4  = kSinC4;
        float c6  = kSinC6;
        float c8  = kSinC8;
        float c10 = kSinC10;

        if (bits == 0 || bits == 2)
        {
            c0  = 1.f;
            c2  = kCosC2;
            c4  = kCosC4;
            c6  = kCosC6;
            c8  = kCosC8;
            c10 = kCosC10;
        }

        const float xsq = xx * xx;
        const float r   = ((((c10 * xsq + c8) * xsq + c6) * xsq + c4) * xsq + c2) * xsq + 1.f;
        return (bits == 1 || bits == 2) ? -(r * c0) : (r * c0);
    }

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
        const float tmp    = (((((((c7 * absa + c6) * absa + c5) * absa + c4) * absa + c3) * absa + c2) * absa + c1) * absa + c0);
        const float tmp2   = tmp * Sqrt(1.f - absa);
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
}   // namespace DT

[[nodiscard]] JUG_MATH_API constexpr float Sin(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::Cos(_x - kHalfPI);
    }

    return ::sinf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Cos(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::Cos(_x);
    }

    return ::cosf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float Tan(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::Cos(_x - kHalfPI) / DT::Cos(_x);
    }

    return ::tanf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float ACos(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::ACos(_x);
    }

    return ::acosf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float ASin(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return kHalfPI - DT::ACos(_x);
    }

    return ::asinf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float ATan2(const float _y, const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::ATan2(_y, _x);
    }

    return ::atan2f(_y, _x);
}

[[nodiscard]] JUG_MATH_API constexpr float ATan(const float _x)
{
    if (std::is_constant_evaluated())
    {
        return DT::ATan2(_x, 1.f);
    }

    return ::atanf(_x);
}

[[nodiscard]] JUG_MATH_API constexpr float AngleDiff(const float _x, const float _y)
{
    const float dist = Wrap(_y - _x, k2PI);
    return Wrap(dist * 2.f, k2PI) - dist;
}

[[nodiscard]] JUG_MATH_API constexpr float AngleLerp(const float _x, const float _y, const float _t)
{
    return _x + AngleDiff(_x, _y) * _t;
}

}   // namespace jug

#undef DT