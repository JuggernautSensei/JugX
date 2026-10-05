#pragma once
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "Macro.h"

// ===========================================
//  Calling Convention
// ===========================================

#if defined(JUG_OS_WINDOWS) && (defined(JUG_COMPILER_MSVC) || defined(JUG_COMPILER_CLANG)) && (defined(JUG_ARCH_X64) || defined(JUG_ARCH_X86)) \
    && !defined(__CUDACC__) && !defined(_M_ARM64EC)
#    define JUG_VECTORCALL __vectorcall
#else
#    define JUG_VECTORCALL
#endif

#ifdef __CUDACC__
#    define JUG_MATH_API __host__ __device__
#else
#    define JUG_MATH_API
#endif

// ===========================================
//  Platform
// ===========================================

#if !defined(__CUDA_ARCH__) && !defined(JUG_NO_SIMD)
#    if defined(JUG_ARCH_X64) || defined(JUG_ARCH_X86)
#        if defined(JUG_ARCH_X64) || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#            include <emmintrin.h>
#            define JUG_SIMD_SSE 1
#        endif
#        if defined(__AVX__) || defined(__SSE4_1__)
#            include <smmintrin.h>
#            define JUG_SIMD_SSE4 1
#        endif
#        if defined(__AVX__)
#            include <immintrin.h>
#            define JUG_SIMD_AVX 1
#        endif
#        if defined(__FMA__) || defined(__AVX2__)
#            include <immintrin.h>
#            define JUG_SIMD_FMA 1
#        endif
#    elif defined(JUG_ARCH_ARM64) || defined(__ARM_NEON)
#        include <arm_neon.h>
#        define JUG_SIMD_NEON 1
#    endif
#endif

#if defined(JUG_SIMD_SSE) || defined(JUG_SIMD_NEON)
#    define JUG_SIMD_AVAILABLE 1
#endif

namespace jug::simd
{

#if defined(JUG_SIMD_NEON)
using M128 = float32x4_t;
#elif defined(JUG_SIMD_SSE)
using M128 = __m128;
#else
struct alignas(16) M128
{
    float f[4];
};
#endif

struct M128X4
{
    M128 r[4];
};

struct SIN_COS
{
    M128 sin;
    M128 cos;
};

namespace detail
{
    [[nodiscard]] JUG_MATH_API constexpr float Lane(
        const M128 _m128,
        const int  _index)
    {
#if !defined(JUG_SIMD_AVAILABLE)
        return _m128.f[_index];
#elif defined(JUG_COMPILER_MSVC) && defined(JUG_SIMD_NEON)
        return _m128.n128_f32[_index];
#elif defined(JUG_COMPILER_MSVC)
        return _m128.m128_f32[_index];
#else
        return _m128[_index];
#endif
    }

    [[nodiscard]] JUG_MATH_API constexpr M128 Set(
        const float _x,
        const float _y,
        const float _z,
        const float _w)
    {
#if !defined(JUG_SIMD_AVAILABLE)
        return M128 {
            { _x, _y, _z, _w }
        };
#elif defined(JUG_COMPILER_MSVC)
        M128 ret {};
#    if defined(JUG_SIMD_NEON)
        ret.n128_f32[0] = _x;
        ret.n128_f32[1] = _y;
        ret.n128_f32[2] = _z;
        ret.n128_f32[3] = _w;
#    else
        ret.m128_f32[0] = _x;
        ret.m128_f32[1] = _y;
        ret.m128_f32[2] = _z;
        ret.m128_f32[3] = _w;
#    endif
        return ret;
#else
        return M128 { _x, _y, _z, _w };   // NOLINT
#endif
    }

    [[nodiscard]] JUG_MATH_API constexpr float MaskOf(
        const bool _value)
    {
        return std::bit_cast<float>(_value ? 0xffffffffu : 0u);
    }
}   // namespace detail

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Sqrt(
    const float _x)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_SSE)
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_x)));
#else
        return ::sqrtf(_x);
#endif
    }

    if (_x != _x || _x < 0.f)   // NOLINT
    {
        return std::bit_cast<float>(0x7fc00000u);
    }

    if (_x == 0.f || _x > 3.40282347e+38f)
    {
        return _x;
    }

    float y = std::bit_cast<float>((std::bit_cast<uint32_t>(_x) >> 1) + 0x1fbd1df5u);
    for (int i = 0; i < 4; ++i)
    {
        y = 0.5f * (y + _x / y);
    }
    return y;
}

// ===========================================
//  No Inline Construct
// ===========================================

// release 빌드에서 inline 화 때문에 컴파일러 최적화가 망가지는 경우가 있어
// inline 화를 막기 위해서 사용한다.
// 해당 함수는 런타임에 불리지 않으므로 성능에 영향이 없다.
template<typename T, typename... Args>
[[nodiscard]] JUG_NO_INLINE JUG_MATH_API constexpr T NoInlineConstruct(
    const Args... _args)
{
    return T { _args... };
}

// =======================================================
//  Load / Store / Set
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Load(
    const float _pFloat4[4])
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vld1q_f32(_pFloat4);
#elif defined(JUG_SIMD_SSE)
        return _mm_loadu_ps(_pFloat4);
#endif
    }
    return detail::Set(_pFloat4[0], _pFloat4[1], _pFloat4[2], _pFloat4[3]);
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL LoadAligned(
    const float _pFloat4[4])
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vld1q_f32(_pFloat4);
#elif defined(JUG_SIMD_SSE)
        return _mm_load_ps(_pFloat4);
#endif
    }
    return detail::Set(_pFloat4[0], _pFloat4[1], _pFloat4[2], _pFloat4[3]);
}

// z = w = 0
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL LoadFloat2(
    const float _pFloat2[2])
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vcombine_f32(vld1_f32(_pFloat2), vdup_n_f32(0.f));
#elif defined(JUG_SIMD_SSE)
        return _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat2)));
#endif
    }
    return detail::Set(_pFloat2[0], _pFloat2[1], 0.f, 0.f);
}

// w = 0
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL LoadFloat3(
    const float _pFloat3[3])
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vld1q_lane_f32(_pFloat3 + 2, vcombine_f32(vld1_f32(_pFloat3), vdup_n_f32(0.f)), 2);
#elif defined(JUG_SIMD_SSE4)
        return _mm_insert_ps(_mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3))), _mm_load_ss(_pFloat3 + 2), 0x20);
#elif defined(JUG_SIMD_SSE)
        return _mm_movelh_ps(_mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3))), _mm_load_ss(_pFloat3 + 2));
#endif
    }
    return detail::Set(_pFloat3[0], _pFloat3[1], _pFloat3[2], 0.f);
}

JUG_MATH_API constexpr void JUG_VECTORCALL Store(
    float      _pOutFloat4[4],
    const M128 _value)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        vst1q_f32(_pOutFloat4, _value);
        return;
#elif defined(JUG_SIMD_SSE)
        _mm_storeu_ps(_pOutFloat4, _value);
        return;
#endif
    }
    for (int i = 0; i < 4; ++i)
    {
        _pOutFloat4[i] = detail::Lane(_value, i);
    }
}

JUG_MATH_API constexpr void JUG_VECTORCALL StoreAligned(
    float      _pOutFloat4[4],
    const M128 _value)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        vst1q_f32(_pOutFloat4, _value);
        return;
#elif defined(JUG_SIMD_SSE)
        _mm_store_ps(_pOutFloat4, _value);
        return;
#endif
    }
    for (int i = 0; i < 4; ++i)
    {
        _pOutFloat4[i] = detail::Lane(_value, i);
    }
}

JUG_MATH_API constexpr void JUG_VECTORCALL StoreFloat2(
    float      _pOutFloat2[2],
    const M128 _value)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        vst1_f32(_pOutFloat2, vget_low_f32(_value));
        return;
#elif defined(JUG_SIMD_SSE)
        _mm_store_sd(reinterpret_cast<double*>(_pOutFloat2), _mm_castps_pd(_value));
        return;
#endif
    }
    _pOutFloat2[0] = detail::Lane(_value, 0);
    _pOutFloat2[1] = detail::Lane(_value, 1);
}

JUG_MATH_API constexpr void JUG_VECTORCALL StoreFloat3(
    float      _pOutFloat3[3],
    const M128 _value)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        vst1_f32(_pOutFloat3, vget_low_f32(_value));
        vst1q_lane_f32(_pOutFloat3 + 2, _value, 2);
        return;
#elif defined(JUG_SIMD_SSE4)
        _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
        *reinterpret_cast<int*>(_pOutFloat3 + 2) = _mm_extract_ps(_value, 2);
        return;
#elif defined(JUG_SIMD_SSE)
        _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
        _mm_store_ss(_pOutFloat3 + 2, _mm_shuffle_ps(_value, _value, _MM_SHUFFLE(2, 2, 2, 2)));
        return;
#endif
    }
    _pOutFloat3[0] = detail::Lane(_value, 0);
    _pOutFloat3[1] = detail::Lane(_value, 1);
    _pOutFloat3[2] = detail::Lane(_value, 2);
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Set(
    const float _x,
    const float _y,
    const float _z,
    const float _w)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        alignas(16) const float values[4] = { _x, _y, _z, _w };
        return vld1q_f32(values);
#elif defined(JUG_SIMD_SSE)
        return _mm_set_ps(_w, _z, _y, _x);
#endif
    }
    return detail::Set(_x, _y, _z, _w);
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL SetAll(
    const float _value)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vdupq_n_f32(_value);
#elif defined(JUG_SIMD_SSE)
        return _mm_set1_ps(_value);
#endif
    }
    return detail::Set(_value, _value, _value, _value);
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Zero()
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vdupq_n_f32(0.f);
#elif defined(JUG_SIMD_SSE)
        return _mm_setzero_ps();
#endif
    }
    return detail::Set(0.f, 0.f, 0.f, 0.f);
}

// =======================================================
//  Basic
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Add(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vaddq_f32(_a, _b);
#elif defined(JUG_SIMD_SSE)
        return _mm_add_ps(_a, _b);
#endif
    }
    return detail::Set(detail::Lane(_a, 0) + detail::Lane(_b, 0), detail::Lane(_a, 1) + detail::Lane(_b, 1), detail::Lane(_a, 2) + detail::Lane(_b, 2), detail::Lane(_a, 3) + detail::Lane(_b, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Sub(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vsubq_f32(_a, _b);
#elif defined(JUG_SIMD_SSE)
        return _mm_sub_ps(_a, _b);
#endif
    }
    return detail::Set(detail::Lane(_a, 0) - detail::Lane(_b, 0), detail::Lane(_a, 1) - detail::Lane(_b, 1), detail::Lane(_a, 2) - detail::Lane(_b, 2), detail::Lane(_a, 3) - detail::Lane(_b, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Mult(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vmulq_f32(_a, _b);
#elif defined(JUG_SIMD_SSE)
        return _mm_mul_ps(_a, _b);
#endif
    }
    return detail::Set(detail::Lane(_a, 0) * detail::Lane(_b, 0), detail::Lane(_a, 1) * detail::Lane(_b, 1), detail::Lane(_a, 2) * detail::Lane(_b, 2), detail::Lane(_a, 3) * detail::Lane(_b, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Div(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && defined(JUG_ARCH_ARM64)
        return vdivq_f32(_a, _b);
#elif defined(JUG_SIMD_NEON)
        float32x4_t recip = vrecpeq_f32(_b);
        recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
        recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
        return vmulq_f32(_a, recip);
#elif defined(JUG_SIMD_SSE)
        return _mm_div_ps(_a, _b);
#endif
    }
    return detail::Set(detail::Lane(_a, 0) / detail::Lane(_b, 0), detail::Lane(_a, 1) / detail::Lane(_b, 1), detail::Lane(_a, 2) / detail::Lane(_b, 2), detail::Lane(_a, 3) / detail::Lane(_b, 3));
}

// _a * _b + _c
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL MultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && defined(JUG_ARCH_ARM64)
        return vfmaq_f32(_c, _a, _b);
#elif defined(JUG_SIMD_NEON)
        return vmlaq_f32(_c, _a, _b);
#elif defined(JUG_SIMD_FMA)
        return _mm_fmadd_ps(_a, _b, _c);
#elif defined(JUG_SIMD_SSE)
        return _mm_add_ps(_mm_mul_ps(_a, _b), _c);
#endif
    }
    return Add(Mult(_a, _b), _c);
}

// _a * _b - _c
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL MultSub(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vsubq_f32(vmulq_f32(_a, _b), _c);
#elif defined(JUG_SIMD_FMA)
        return _mm_fmsub_ps(_a, _b, _c);
#elif defined(JUG_SIMD_SSE)
        return _mm_sub_ps(_mm_mul_ps(_a, _b), _c);
#endif
    }
    return Sub(Mult(_a, _b), _c);
}

// _c - _a * _b
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL NegMultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && defined(JUG_ARCH_ARM64)
        return vfmsq_f32(_c, _a, _b);
#elif defined(JUG_SIMD_NEON)
        return vmlsq_f32(_c, _a, _b);
#elif defined(JUG_SIMD_FMA)
        return _mm_fnmadd_ps(_a, _b, _c);
#elif defined(JUG_SIMD_SSE)
        return _mm_sub_ps(_c, _mm_mul_ps(_a, _b));
#endif
    }
    return Sub(_c, Mult(_a, _b));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Negate(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vnegq_f32(_a);
#elif defined(JUG_SIMD_SSE)
        return _mm_xor_ps(_a, _mm_set1_ps(-0.f));
#endif
    }

    return detail::Set(-detail::Lane(_a, 0), -detail::Lane(_a, 1), -detail::Lane(_a, 2), -detail::Lane(_a, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Sqrt(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && defined(JUG_ARCH_ARM64)
        return vsqrtq_f32(_a);
#elif defined(JUG_SIMD_NEON)
        float32x4_t y = vrsqrteq_f32(_a);
        y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
        y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
        return vmulq_f32(_a, y);
#elif defined(JUG_SIMD_SSE)
        return _mm_sqrt_ps(_a);
#endif
    }

    return detail::Set(Sqrt(detail::Lane(_a, 0)), Sqrt(detail::Lane(_a, 1)), Sqrt(detail::Lane(_a, 2)), Sqrt(detail::Lane(_a, 3)));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL RSqrt(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        float32x4_t y = vrsqrteq_f32(_a);
        y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
        y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
        return y;
#elif defined(JUG_SIMD_SSE)
        const M128 y0 = _mm_rsqrt_ps(_a);
        return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.f), _mm_mul_ps(_a, _mm_mul_ps(y0, y0))));
#endif
    }
    return Div(SetAll(1.f), Sqrt(_a));
}

[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL RSqrt(
    const float _x)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_SSE)
        const float y0 = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(_x)));
        return 0.5f * y0 * (3.f - _x * y0 * y0);
#endif
    }
    return 1.f / Sqrt(_x);
}

// =======================================================
//  Bitwise
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL And(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_SIMD_SSE)
        return _mm_and_ps(_a, _b);
#endif
    }

    return detail::Set(std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 0)) & std::bit_cast<uint32_t>(detail::Lane(_b, 0))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 1)) & std::bit_cast<uint32_t>(detail::Lane(_b, 1))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 2)) & std::bit_cast<uint32_t>(detail::Lane(_b, 2))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 3)) & std::bit_cast<uint32_t>(detail::Lane(_b, 3))));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Or(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_SIMD_SSE)
        return _mm_or_ps(_a, _b);
#endif
    }

    return detail::Set(std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 0)) | std::bit_cast<uint32_t>(detail::Lane(_b, 0))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 1)) | std::bit_cast<uint32_t>(detail::Lane(_b, 1))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 2)) | std::bit_cast<uint32_t>(detail::Lane(_b, 2))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 3)) | std::bit_cast<uint32_t>(detail::Lane(_b, 3))));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Xor(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_SIMD_SSE)
        return _mm_xor_ps(_a, _b);
#endif
    }

    return detail::Set(std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 0)) ^ std::bit_cast<uint32_t>(detail::Lane(_b, 0))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 1)) ^ std::bit_cast<uint32_t>(detail::Lane(_b, 1))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 2)) ^ std::bit_cast<uint32_t>(detail::Lane(_b, 2))), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 3)) ^ std::bit_cast<uint32_t>(detail::Lane(_b, 3))));
}

// (~_a) & _b
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL AndNot(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(_b), vreinterpretq_u32_f32(_a)));
#elif defined(JUG_SIMD_SSE)
        return _mm_andnot_ps(_a, _b);
#endif
    }

    return detail::Set(std::bit_cast<float>(~std::bit_cast<uint32_t>(detail::Lane(_a, 0)) & std::bit_cast<uint32_t>(detail::Lane(_b, 0))), std::bit_cast<float>(~std::bit_cast<uint32_t>(detail::Lane(_a, 1)) & std::bit_cast<uint32_t>(detail::Lane(_b, 1))), std::bit_cast<float>(~std::bit_cast<uint32_t>(detail::Lane(_a, 2)) & std::bit_cast<uint32_t>(detail::Lane(_b, 2))), std::bit_cast<float>(~std::bit_cast<uint32_t>(detail::Lane(_a, 3)) & std::bit_cast<uint32_t>(detail::Lane(_b, 3))));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL ZeroW(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vcombine_u32(vcreate_u32(0xffffffffffffffffull), vcreate_u32(0x00000000ffffffffull))));
#elif defined(JUG_SIMD_SSE)
        return _mm_and_ps(_a, _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0)));
#endif
    }

    return detail::Set(detail::Lane(_a, 0), detail::Lane(_a, 1), detail::Lane(_a, 2), 0.f);
}

template<int kShift>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL ShiftLeft(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vshlq_n_u32(vreinterpretq_u32_f32(_a), kShift));
#elif defined(JUG_SIMD_SSE)
        return _mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(_a), kShift));
#endif
    }

    return detail::Set(std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 0)) << kShift), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 1)) << kShift), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 2)) << kShift), std::bit_cast<float>(std::bit_cast<uint32_t>(detail::Lane(_a, 3)) << kShift));
}

// _mask 가 켜진 요소는 _a, 꺼진 요소는 _b
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Select(
    const M128 _mask,
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vbslq_f32(vreinterpretq_u32_f32(_mask), _a, _b);
#elif defined(JUG_SIMD_SSE4)
        return _mm_blendv_ps(_b, _a, _mask);
#elif defined(JUG_SIMD_SSE)
        return _mm_or_ps(_mm_and_ps(_mask, _a), _mm_andnot_ps(_mask, _b));
#endif
    }
    return Or(And(_mask, _a), AndNot(_mask, _b));
}

template<int kMask>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Blend(
    const M128 _a,
    const M128 _b)
{
    static_assert(kMask >= 0 && kMask <= 15, "Blend mask must be within [0, 15]");
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        alignas(16) constexpr uint32_t kBlendMask[4] = { (kMask & 1) ? 0xffffffffu : 0u, (kMask & 2) ? 0xffffffffu : 0u, (kMask & 4) ? 0xffffffffu : 0u, (kMask & 8) ? 0xffffffffu : 0u };
        return vbslq_f32(vld1q_u32(kBlendMask), _b, _a);
#elif defined(JUG_SIMD_SSE4)
        return _mm_blend_ps(_a, _b, kMask);
#elif defined(JUG_SIMD_SSE)
        const M128 mask = _mm_castsi128_ps(_mm_setr_epi32((kMask & 1) ? -1 : 0, (kMask & 2) ? -1 : 0, (kMask & 4) ? -1 : 0, (kMask & 8) ? -1 : 0));
        return _mm_or_ps(_mm_andnot_ps(mask, _a), _mm_and_ps(mask, _b));
#endif
    }
    return detail::Set((kMask & 1) ? detail::Lane(_b, 0) : detail::Lane(_a, 0), (kMask & 2) ? detail::Lane(_b, 1) : detail::Lane(_a, 1), (kMask & 4) ? detail::Lane(_b, 2) : detail::Lane(_a, 2), (kMask & 8) ? detail::Lane(_b, 3) : detail::Lane(_a, 3));
}

// =======================================================
//  Comparison
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL CmpEq(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vceqq_f32(_a, _b));
#elif defined(JUG_SIMD_SSE)
        return _mm_cmpeq_ps(_a, _b);
#endif
    }

    return detail::Set(detail::MaskOf(detail::Lane(_a, 0) == detail::Lane(_b, 0)), detail::MaskOf(detail::Lane(_a, 1) == detail::Lane(_b, 1)), detail::MaskOf(detail::Lane(_a, 2) == detail::Lane(_b, 2)), detail::MaskOf(detail::Lane(_a, 3) == detail::Lane(_b, 3)));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL CmpNe(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(_a, _b)));
#elif defined(JUG_SIMD_SSE)
        return _mm_cmpneq_ps(_a, _b);
#endif
    }

    return detail::Set(detail::MaskOf(detail::Lane(_a, 0) != detail::Lane(_b, 0)), detail::MaskOf(detail::Lane(_a, 1) != detail::Lane(_b, 1)), detail::MaskOf(detail::Lane(_a, 2) != detail::Lane(_b, 2)), detail::MaskOf(detail::Lane(_a, 3) != detail::Lane(_b, 3)));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL CmpLt(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vcltq_f32(_a, _b));
#elif defined(JUG_SIMD_SSE)
        return _mm_cmplt_ps(_a, _b);
#endif
    }

    return detail::Set(detail::MaskOf(detail::Lane(_a, 0) < detail::Lane(_b, 0)), detail::MaskOf(detail::Lane(_a, 1) < detail::Lane(_b, 1)), detail::MaskOf(detail::Lane(_a, 2) < detail::Lane(_b, 2)), detail::MaskOf(detail::Lane(_a, 3) < detail::Lane(_b, 3)));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL CmpLe(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vreinterpretq_f32_u32(vcleq_f32(_a, _b));
#elif defined(JUG_SIMD_SSE)
        return _mm_cmple_ps(_a, _b);
#endif
    }

    return detail::Set(detail::MaskOf(detail::Lane(_a, 0) <= detail::Lane(_b, 0)), detail::MaskOf(detail::Lane(_a, 1) <= detail::Lane(_b, 1)), detail::MaskOf(detail::Lane(_a, 2) <= detail::Lane(_b, 2)), detail::MaskOf(detail::Lane(_a, 3) <= detail::Lane(_b, 3)));
}

[[nodiscard]] JUG_MATH_API constexpr int JUG_VECTORCALL MoveMask(
    const M128 _mask)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        const uint32x4_t bits   = vcombine_u32(vcreate_u32(0x0000000200000001ull), vcreate_u32(0x0000000800000004ull));
        const uint32x4_t masked = vandq_u32(vreinterpretq_u32_f32(_mask), bits);
#    if defined(JUG_ARCH_ARM64)
        return static_cast<int>(vaddvq_u32(masked));
#    else
        uint32x2_t sum2 = vadd_u32(vget_low_u32(masked), vget_high_u32(masked));
        sum2            = vpadd_u32(sum2, sum2);
        return static_cast<int>(vget_lane_u32(sum2, 0));
#    endif
#elif defined(JUG_SIMD_SSE)
        return _mm_movemask_ps(_mask);
#endif
    }
    int ret = 0;
    for (int i = 0; i < 4; ++i)
    {
        ret |= static_cast<int>(std::bit_cast<uint32_t>(detail::Lane(_mask, i)) >> 31) << i;
    }
    return ret;
}

// =======================================================
//  Min / Max / Abs
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Min(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vminq_f32(_a, _b);
#elif defined(JUG_SIMD_SSE)
        return _mm_min_ps(_a, _b);
#endif
    }

    return detail::Set(detail::Lane(_a, 0) < detail::Lane(_b, 0) ? detail::Lane(_a, 0) : detail::Lane(_b, 0), detail::Lane(_a, 1) < detail::Lane(_b, 1) ? detail::Lane(_a, 1) : detail::Lane(_b, 1), detail::Lane(_a, 2) < detail::Lane(_b, 2) ? detail::Lane(_a, 2) : detail::Lane(_b, 2), detail::Lane(_a, 3) < detail::Lane(_b, 3) ? detail::Lane(_a, 3) : detail::Lane(_b, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Max(
    const M128 _a,
    const M128 _b)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vmaxq_f32(_a, _b);
#elif defined(JUG_SIMD_SSE)
        return _mm_max_ps(_a, _b);
#endif
    }

    return detail::Set(detail::Lane(_a, 0) > detail::Lane(_b, 0) ? detail::Lane(_a, 0) : detail::Lane(_b, 0), detail::Lane(_a, 1) > detail::Lane(_b, 1) ? detail::Lane(_a, 1) : detail::Lane(_b, 1), detail::Lane(_a, 2) > detail::Lane(_b, 2) ? detail::Lane(_a, 2) : detail::Lane(_b, 2), detail::Lane(_a, 3) > detail::Lane(_b, 3) ? detail::Lane(_a, 3) : detail::Lane(_b, 3));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Abs(
    const M128 _a)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vabsq_f32(_a);
#elif defined(JUG_SIMD_SSE)
        return _mm_andnot_ps(_mm_set1_ps(-0.f), _a);
#endif
    }
    return AndNot(SetAll(-0.f), _a);
}

// =======================================================
//  Shuffle / Splat / Get / Insert
// =======================================================

template<int X, int Y, int Z, int W>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Shuffle(
    const M128 _a)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle indices must be within [0, 3]");
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && (defined(JUG_COMPILER_CLANG) || defined(JUG_COMPILER_GCC))
        return __builtin_shufflevector(_a, _a, X, Y, Z, W);
#elif defined(JUG_SIMD_NEON)
        alignas(16) float tmp[4];
        vst1q_f32(tmp, _a);
        alignas(16) const float ret[4] = { tmp[X], tmp[Y], tmp[Z], tmp[W] };
        return vld1q_f32(ret);
#elif defined(JUG_SIMD_AVX)
        return _mm_permute_ps(_a, _MM_SHUFFLE(W, Z, Y, X));
#elif defined(JUG_SIMD_SSE)
        return _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(W, Z, Y, X));
#endif
    }

    return detail::Set(detail::Lane(_a, X), detail::Lane(_a, Y), detail::Lane(_a, Z), detail::Lane(_a, W));
}

template<int X, int Y, int Z, int W>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Shuffle2(
    const M128 _a,
    const M128 _b)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle2 indices must be within [0, 3]");
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON) && (defined(JUG_COMPILER_CLANG) || defined(JUG_COMPILER_GCC))
        return __builtin_shufflevector(_a, _b, X, Y, Z + 4, W + 4);
#elif defined(JUG_SIMD_NEON)
        alignas(16) float ta[4];
        vst1q_f32(ta, _a);
        alignas(16) float tb[4];
        vst1q_f32(tb, _b);
        alignas(16) const float ret[4] = { ta[X], ta[Y], tb[Z], tb[W] };
        return vld1q_f32(ret);
#elif defined(JUG_SIMD_SSE)
        return _mm_shuffle_ps(_a, _b, _MM_SHUFFLE(W, Z, Y, X));
#endif
    }

    return detail::Set(detail::Lane(_a, X), detail::Lane(_a, Y), detail::Lane(_b, Z), detail::Lane(_b, W));
}

template<int kIndex>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Splat(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Splat index must be within [0, 3]");
#if defined(JUG_SIMD_NEON)
    if (!std::is_constant_evaluated())
    {
        return vdupq_n_f32(vgetq_lane_f32(_a, kIndex));
    }
#endif
    return Shuffle<kIndex, kIndex, kIndex, kIndex>(_a);
}

template<int kIndex>
[[nodiscard]] JUG_MATH_API constexpr float JUG_VECTORCALL Get(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Get index must be within [0, 3]");
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vgetq_lane_f32(_a, kIndex);
#elif defined(JUG_SIMD_SSE)
        if constexpr (kIndex == 0)
        {
            return _mm_cvtss_f32(_a);
        }
        else
        {
            return _mm_cvtss_f32(Shuffle<kIndex, kIndex, kIndex, kIndex>(_a));
        }
#endif
    }
    return detail::Lane(_a, kIndex);
}

template<int kIndex>
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Insert(
    const M128  _a,
    const float _value)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Insert index must be within [0, 3]");
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        return vsetq_lane_f32(_value, _a, kIndex);
#elif defined(JUG_SIMD_SSE4)
        return _mm_insert_ps(_a, _mm_set_ss(_value), kIndex << 4);
#elif defined(JUG_SIMD_SSE)
        if constexpr (kIndex == 0)
        {
            return _mm_move_ss(_a, _mm_set_ss(_value));
        }
        else
        {
            constexpr int kSwap = kIndex == 1 ? _MM_SHUFFLE(3, 2, 0, 1)
                                : kIndex == 2 ? _MM_SHUFFLE(3, 0, 1, 2)
                                              : _MM_SHUFFLE(0, 2, 1, 3);

            const M128 swap  = _mm_shuffle_ps(_a, _a, kSwap);
            const M128 moved = _mm_move_ss(swap, _mm_set_ss(_value));
            return _mm_shuffle_ps(moved, moved, kSwap);
        }
#endif
    }

    return detail::Set(kIndex == 0 ? _value : detail::Lane(_a, 0), kIndex == 1 ? _value : detail::Lane(_a, 1), kIndex == 2 ? _value : detail::Lane(_a, 2), kIndex == 3 ? _value : detail::Lane(_a, 3));
}

JUG_MATH_API constexpr void JUG_VECTORCALL Transpose4(
    M128& _row0,
    M128& _row1,
    M128& _row2,
    M128& _row3)
{
    if (!std::is_constant_evaluated())
    {
#if defined(JUG_SIMD_NEON)
        const float32x4x2_t t0 = vtrnq_f32(_row0, _row1);
        const float32x4x2_t t1 = vtrnq_f32(_row2, _row3);
        _row0                  = vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0]));
        _row1                  = vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1]));
        _row2                  = vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0]));
        _row3                  = vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1]));
        return;
#elif defined(JUG_SIMD_SSE)
        const M128 t0 = _mm_unpacklo_ps(_row0, _row1);
        const M128 t1 = _mm_unpacklo_ps(_row2, _row3);
        const M128 t2 = _mm_unpackhi_ps(_row0, _row1);
        const M128 t3 = _mm_unpackhi_ps(_row2, _row3);
        _row0         = _mm_movelh_ps(t0, t1);
        _row1         = _mm_movehl_ps(t1, t0);
        _row2         = _mm_movelh_ps(t2, t3);
        _row3         = _mm_movehl_ps(t3, t2);
        return;
#endif
    }

    const M128 r0 = _row0;
    const M128 r1 = _row1;
    const M128 r2 = _row2;
    const M128 r3 = _row3;
    _row0         = detail::Set(detail::Lane(r0, 0), detail::Lane(r1, 0), detail::Lane(r2, 0), detail::Lane(r3, 0));
    _row1         = detail::Set(detail::Lane(r0, 1), detail::Lane(r1, 1), detail::Lane(r2, 1), detail::Lane(r3, 1));
    _row2         = detail::Set(detail::Lane(r0, 2), detail::Lane(r1, 2), detail::Lane(r2, 2), detail::Lane(r3, 2));
    _row3         = detail::Set(detail::Lane(r0, 3), detail::Lane(r1, 3), detail::Lane(r2, 3), detail::Lane(r3, 3));
}

// =======================================================
//  Vector
// =======================================================

// 네 요소의 합을 모든 요소에
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL SumAll(
    const M128 _a)
{
#if defined(JUG_SIMD_NEON) && defined(JUG_ARCH_ARM64)
    if (!std::is_constant_evaluated())
    {
        return vdupq_n_f32(vaddvq_f32(_a));
    }
#endif
    const M128 sums = Add(_a, Shuffle<1, 0, 3, 2>(_a));
    return Add(sums, Shuffle<2, 3, 0, 1>(sums));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Dot4V(
    const M128 _a,
    const M128 _b)
{
    return SumAll(Mult(_a, _b));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Dot3V(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_SIMD_SSE)
    if (!std::is_constant_evaluated())
    {
        const M128 m  = _mm_mul_ps(_a, _b);
        const M128 yz = Shuffle<1, 2, 1, 2>(m);
        M128       s  = _mm_add_ss(m, yz);
        s             = _mm_add_ss(s, Shuffle<1, 1, 1, 1>(yz));
        return Shuffle<0, 0, 0, 0>(s);
    }
#endif
    const M128 m = Mult(_a, _b);
    return Add(Add(Splat<0>(m), Splat<1>(m)), Splat<2>(m));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Cross3(
    const M128 _a,
    const M128 _b)
{
    const M128 aYzx = Shuffle<1, 2, 0, 3>(_a);
    const M128 bYzx = Shuffle<1, 2, 0, 3>(_b);
    return Shuffle<1, 2, 0, 3>(MultSub(_a, bYzx, Mult(aYzx, _b)));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL NormalizeBy(
    const M128 _a,
    const M128 _lenSq)
{
    const float lenSq = Get<0>(_lenSq);
    if (lenSq < 1.17549435e-38f)
    {
        return lenSq == 0.f ? Zero() : Div(_a, Sqrt(_lenSq));
    }
    return Mult(_a, RSqrt(_lenSq));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Normalize3(
    const M128 _a)
{
    return NormalizeBy(_a, Dot3V(_a, _a));
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Lerp(
    const M128 _a,
    const M128 _b,
    const M128 _t)
{
    return MultAdd(Sub(_b, _a), _t, _a);
}

// =======================================================
//  Trigonometry
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL ReduceAngle(
    const M128 _x,
    M128&      _outSign)
{
    const M128 magic = SetAll(12582912.f);
    const M128 t     = MultAdd(_x, SetAll(0.318309886f), magic);
    _outSign         = ShiftLeft<31>(t);
    return NegMultAdd(Sub(t, magic), SetAll(3.141592654f), _x);
}

JUG_MATH_API constexpr SIN_COS JUG_VECTORCALL SinCos(
    const M128 _x)
{
    M128       sign;
    const M128 r  = ReduceAngle(_x, sign);
    const M128 r2 = Mult(r, r);
    const M128 r4 = Mult(r2, r2);

    const M128 s01 = MultAdd(SetAll(-0.16666667f), r2, SetAll(1.f));
    const M128 s23 = MultAdd(SetAll(-0.00019840874f), r2, SetAll(0.0083333310f));
    const M128 s45 = MultAdd(SetAll(-2.3889859e-08f), r2, SetAll(2.7525562e-06f));

    const M128 c01 = MultAdd(SetAll(-0.5f), r2, SetAll(1.f));
    const M128 c23 = MultAdd(SetAll(-0.0013888378f), r2, SetAll(0.041666638f));
    const M128 c45 = MultAdd(SetAll(-2.6051615e-07f), r2, SetAll(2.4760495e-05f));
    return { Xor(Mult(MultAdd(MultAdd(s45, r4, s23), r4, s01), r), sign), Xor(MultAdd(MultAdd(c45, r4, c23), r4, c01), sign) };
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Sin(
    const M128 _x)
{
    M128       sign;
    const M128 r  = ReduceAngle(_x, sign);
    const M128 r2 = Mult(r, r);
    const M128 r4 = Mult(r2, r2);

    const M128 s01 = MultAdd(SetAll(-0.16666667f), r2, SetAll(1.f));
    const M128 s23 = MultAdd(SetAll(-0.00019840874f), r2, SetAll(0.0083333310f));
    const M128 s45 = MultAdd(SetAll(-2.3889859e-08f), r2, SetAll(2.7525562e-06f));
    return Xor(Mult(MultAdd(MultAdd(s45, r4, s23), r4, s01), r), sign);
}

[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL Cos(
    const M128 _x)
{
    M128       sign;
    const M128 r  = ReduceAngle(_x, sign);
    const M128 r2 = Mult(r, r);
    const M128 r4 = Mult(r2, r2);

    const M128 c01 = MultAdd(SetAll(-0.5f), r2, SetAll(1.f));
    const M128 c23 = MultAdd(SetAll(-0.0013888378f), r2, SetAll(0.041666638f));
    const M128 c45 = MultAdd(SetAll(-2.6051615e-07f), r2, SetAll(2.4760495e-05f));
    return Xor(MultAdd(MultAdd(c45, r4, c23), r4, c01), sign);
}

// (sin, cos, sin, cos)
[[nodiscard]] JUG_MATH_API constexpr M128 JUG_VECTORCALL SinCos1(
    const float _x)
{
    M128       sign;
    const M128 r  = ReduceAngle(SetAll(_x), sign);
    const M128 r2 = Mult(r, r);
    const M128 r4 = Mult(r2, r2);

    const M128 p01 = MultAdd(Set(-0.16666667f, -0.5f, -0.16666667f, -0.5f), r2, SetAll(1.f));
    const M128 p23 = MultAdd(Set(-0.00019840874f, -0.0013888378f, -0.00019840874f, -0.0013888378f), r2, Set(0.0083333310f, 0.041666638f, 0.0083333310f, 0.041666638f));
    const M128 p45 = MultAdd(Set(-2.3889859e-08f, -2.6051615e-07f, -2.3889859e-08f, -2.6051615e-07f), r2, Set(2.7525562e-06f, 2.4760495e-05f, 2.7525562e-06f, 2.4760495e-05f));
    const M128 p   = MultAdd(MultAdd(p45, r4, p23), r4, p01);
    return Xor(Mult(p, Blend<0b1010>(r, SetAll(1.f))), sign);
}

}   // namespace jug::simd
