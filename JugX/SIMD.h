#pragma once

// ===========================================
//  API
// ===========================================

#if !defined(__CUDA_ARCH__)
#    if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#        if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)                           \
            || defined(__SSE4_2__) || defined(__SSE4_1__) || defined(__SSE3__) || defined(__SSE2__) \
            || defined(__x86_64__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#            include <emmintrin.h>
#            define JUG_MATH_SIMD_SSE 1
#        endif
#        if defined(__AVX__)
#            include <immintrin.h>
#            define JUG_MATH_SIMD_AVX 1
#        endif
#        if defined(__FMA__) || defined(__AVX2__)   // MSVC 는 /arch:AVX2 에서 FMA3 를 켜지만 __FMA__ 를 정의하지 않는다.
#            include <immintrin.h>
#            define JUG_MATH_SIMD_FMA 1
#        endif
#    elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#        include <arm_neon.h>
#        define JUG_MATH_SIMD_NEON 1
#    endif

#    if defined(JUG_MATH_SIMD_SSE) || defined(JUG_MATH_SIMD_FMA) || defined(JUG_MATH_SIMD_NEON)
#        define JUG_SIMD_AVAILABLE 1
#    endif

#    ifdef JUG_SIMD_AVAILABLE

namespace jug::simd
{

#        if defined(JUG_MATH_SIMD_NEON)
using M128 = float32x4_t;
#        elif defined(JUG_MATH_SIMD_SSE)
using M128 = __m128;
#        endif

// =======================================================
//  Load / Store / Set
// =======================================================

[[nodiscard]] inline M128 Load(
    const float _pFloat4[4])
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_loadu_ps(_pFloat4);
#        endif
}

[[nodiscard]] inline M128 LoadAligned(
    const float _pFloat4[4])
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_load_ps(_pFloat4);
#        endif
}

inline void Store(
    float      _pOutFloat4[4],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_storeu_ps(_pOutFloat4, _value);
#        endif
}

inline void StoreAligned(
    float      _pOutFloat4[4],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_ps(_pOutFloat4, _value);
#        endif
}

inline void StoreAligned2(
    float      _pOutFloat8[8],
    const M128 _lo,
    const M128 _hi)
{
#        if defined(JUG_MATH_SIMD_AVX)
    _mm256_storeu_ps(_pOutFloat8, _mm256_set_m128(_hi, _lo));
#        else
    StoreAligned(_pOutFloat8, _lo);
    StoreAligned(_pOutFloat8 + 4, _hi);
#        endif
}

[[nodiscard]] inline M128 LoadFloat3(
    const float _pFloat3[3])
{
#        if defined(JUG_MATH_SIMD_NEON)
    const float32x4_t xy0 = vcombine_f32(vld1_f32(_pFloat3), vdup_n_f32(0.f));
    return vld1q_lane_f32(_pFloat3 + 2, xy0, 2);
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3)));
    const M128 z  = _mm_load_ss(_pFloat3 + 2);
    return _mm_movelh_ps(xy, z);
#        endif
}

inline void StoreFloat3(
    float      _pOutFloat3[3],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1_f32(_pOutFloat3, vget_low_f32(_value));
    vst1q_lane_f32(_pOutFloat3 + 2, _value, 2);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
    _mm_store_ss(_pOutFloat3 + 2, _mm_movehl_ps(_value, _value));
#        endif
}

[[nodiscard]] inline M128 Set(
    const float _x,
    const float _y,
    const float _z,
    const float _w)
{
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) const float values[4] = { _x, _y, _z, _w };
    return vld1q_f32(values);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set_ps(_w, _z, _y, _x);
#        endif
}

[[nodiscard]] inline M128 SetAll(
    const float _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(_value);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set1_ps(_value);
#        endif
}

[[nodiscard]] inline M128 Zero()
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(0.f);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_setzero_ps();
#        endif
}

// =======================================================
//  Basic
// =======================================================

[[nodiscard]] inline M128 Add(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vaddq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_add_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Sub(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vsubq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Mult(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vmulq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_mul_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Div(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__)
    return vdivq_f32(_a, _b);
#            else
    float32x4_t recip = vrecpeq_f32(_b);
    recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
    recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
    return vmulq_f32(_a, recip);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_div_ps(_a, _b);
#        endif
}

// _a * _b + _c
[[nodiscard]] inline M128 MultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__)
    return vfmaq_f32(_c, _a, _b);
#            else
    return vmlaq_f32(_c, _a, _b);
#            endif
#        elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fmadd_ps(_a, _b, _c);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_add_ps(_mm_mul_ps(_a, _b), _c);
#        endif
}

// _c - _a * _b
[[nodiscard]] inline M128 NegMultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vmlsq_f32(_c, _a, _b);
#        elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fnmadd_ps(_a, _b, _c);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_c, _mm_mul_ps(_a, _b));
#        endif
}

[[nodiscard]] inline M128 Scale(
    const M128  _a,
    const float _scalar)
{
    return Mult(_a, SetAll(_scalar));
}

[[nodiscard]] inline M128 Negate(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vnegq_f32(_a);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_xor_ps(_a, _mm_set1_ps(-0.f));
#        endif
}

[[nodiscard]] inline M128 Sqrt(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__)
    return vsqrtq_f32(_a);
#            else
    alignas(16) float tmp[4];
    vst1q_f32(tmp, _a);
    tmp[0] = std::sqrt(tmp[0]);
    tmp[1] = std::sqrt(tmp[1]);
    tmp[2] = std::sqrt(tmp[2]);
    tmp[3] = std::sqrt(tmp[3]);
    return vld1q_f32(tmp);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sqrt_ps(_a);
#        endif
}

[[nodiscard]] inline M128 RSqrt(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    float32x4_t y = vrsqrteq_f32(_a);
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    return y;
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 y0    = _mm_rsqrt_ps(_a);
    const M128 half  = _mm_set1_ps(0.5f);
    const M128 three = _mm_set1_ps(3.f);
    return _mm_mul_ps(_mm_mul_ps(half, y0), _mm_sub_ps(three, _mm_mul_ps(_a, _mm_mul_ps(y0, y0))));
#        endif
}

// =======================================================
//  Bitwise Operations
// =======================================================

[[nodiscard]] inline M128 And(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_and_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Or(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Xor(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_xor_ps(_a, _b);
#        endif
}

// (~_a) & _b
[[nodiscard]] inline M128 AndNot(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(_b), vreinterpretq_u32_f32(_a)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 MaskXYZ()
{
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) const uint32_t mask[4] = { 0xffffffffu, 0xffffffffu, 0xffffffffu, 0u };
    return vreinterpretq_f32_u32(vld1q_u32(mask));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
#        endif
}

[[nodiscard]] inline M128 ZeroW(
    const M128 _a)
{
    return And(_a, MaskXYZ());
}

// _a, _b 에서 마스크에 맞게 요소를 뽑아서 새로운 float4로 만듦
[[nodiscard]] inline M128 Select(
    const M128 _mask,
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vbslq_f32(_mask, _a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_mm_and_ps(_mask, _a), _mm_andnot_ps(_mask, _b));
#        endif
}

// =======================================================
//  Comparison
// =======================================================

[[nodiscard]] inline M128 CmpEq(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vceqq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpeq_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 CmpLt(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vcltq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmplt_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 CmpLe(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vcleq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmple_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 CmpGt(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vcgtq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpgt_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 CmpGe(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vcgeq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpge_ps(_a, _b);
#        endif
}

// 마스크를 int로 변환. 하위 4비트만 사용
[[nodiscard]] inline int MoveMask(
    const M128 _mask)
{
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) const uint32_t bits[4] = { 1u, 2u, 4u, 8u };
    const uint32x4_t           masked  = vandq_u32(_mask, vld1q_u32(bits));
#            if defined(__aarch64__)
    return static_cast<int>(vaddvq_u32(masked));
#            else
    uint32x2_t sum2 = vadd_u32(vget_low_u32(masked), vget_high_u32(masked));
    sum2            = vpadd_u32(sum2, sum2);
    return static_cast<int>(vget_lane_u32(sum2, 0));
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_movemask_ps(_mask);
#        endif
}

[[nodiscard]] inline bool AllTrue(
    const M128 _mask)
{
    return MoveMask(_mask) == 0xf;
}

[[nodiscard]] inline bool AllTrue3(
    const M128 _mask)
{
    return (MoveMask(_mask) & 0x7) == 0x7;
}

[[nodiscard]] inline bool AnyTrue(
    const M128 _mask)
{
    return MoveMask(_mask) != 0;
}

// =======================================================
//  Min / Max / Clamp / Abs
// =======================================================

[[nodiscard]] inline M128 Min(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vminq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_min_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Max(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vmaxq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_max_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 Clamp(
    const M128 _value,
    const M128 _min,
    const M128 _max)
{
    return Max(_min, Min(_max, _value));
}

[[nodiscard]] inline M128 Abs(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vabsq_f32(_a);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_mm_set1_ps(-0.f), _a);
#        endif
}

// =======================================================
//  Shuffle / Splat / GetX/Y/Z/W
// =======================================================

template<int X, int Y, int Z, int W>
[[nodiscard]] M128 Shuffle(
    const M128 _a)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle indices must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) float tmp[4];
    vst1q_f32(tmp, _a);
    alignas(16) const float ret[4] = { tmp[X], tmp[Y], tmp[Z], tmp[W] };
    return vld1q_f32(ret);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(W, Z, Y, X));
#        endif
}

template<int X, int Y, int Z, int W>
[[nodiscard]] M128 Shuffle2(
    const M128 _a,
    const M128 _b)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle2 indices must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) float ta[4];
    vst1q_f32(ta, _a);
    alignas(16) float tb[4];
    vst1q_f32(tb, _b);
    alignas(16) const float ret[4] = { ta[X], ta[Y], tb[Z], tb[W] };
    return vld1q_f32(ret);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _b, _MM_SHUFFLE(W, Z, Y, X));
#        endif
}

template<int kIndex>
[[nodiscard]] M128 Splat(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Splat index must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    return vdupq_laneq_f32(_a, kIndex);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(kIndex, kIndex, kIndex, kIndex));
#        endif
}

[[nodiscard]] inline float GetX(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, 0);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cvtss_f32(_a);
#        endif
}

[[nodiscard]] inline float GetY(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, 1);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cvtss_f32(Shuffle<1, 1, 1, 1>(_a));
#        endif
}

[[nodiscard]] inline float GetZ(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, 2);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cvtss_f32(Shuffle<2, 2, 2, 2>(_a));
#        endif
}

[[nodiscard]] inline float GetW(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, 3);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cvtss_f32(Shuffle<3, 3, 3, 3>(_a));
#        endif
}

inline void Transpose4(
    M128& _row0,
    M128& _row1,
    M128& _row2,
    M128& _row3)
{
#        if defined(JUG_MATH_SIMD_NEON)
    const float32x4x2_t t0 = vtrnq_f32(_row0, _row1);
    const float32x4x2_t t1 = vtrnq_f32(_row2, _row3);
    _row0                  = vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0]));
    _row1                  = vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1]));
    _row2                  = vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0]));
    _row3                  = vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1]));
#        elif defined(JUG_MATH_SIMD_SSE)
    _MM_TRANSPOSE4_PS(_row0, _row1, _row2, _row3);   // NOLINT
#        endif
}

namespace simd_detail
{
    [[nodiscard]] inline M128 HorizontalSumAll(
        const M128 _a)
    {
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__)
        return vdupq_n_f32(vaddvq_f32(_a));
#            else
        float32x2_t sum2 = vadd_f32(vget_low_f32(_a), vget_high_f32(_a));
        sum2             = vpadd_f32(sum2, sum2);
        return vdupq_lane_f32(sum2, 0);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
        M128 shuf = _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(2, 3, 0, 1));
        M128 sums = _mm_add_ps(_a, shuf);
        shuf      = _mm_movehl_ps(shuf, sums);
        sums      = _mm_add_ps(sums, shuf);
        return _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(0, 0, 0, 0));
#        endif
    }
}   // namespace simd_detail

// =======================================================
//  TVector operations
// =======================================================

[[nodiscard]] inline M128 Dot4V(
    const M128 _a,
    const M128 _b)
{
    return simd_detail::HorizontalSumAll(Mult(_a, _b));
}

[[nodiscard]] inline M128 Dot3V(
    const M128 _a,
    const M128 _b)
{
    return simd_detail::HorizontalSumAll(ZeroW(Mult(_a, _b)));
}

[[nodiscard]] inline float Dot4(
    const M128 _a,
    const M128 _b)
{
    return GetX(Dot4V(_a, _b));
}

[[nodiscard]] inline float Dot3(
    const M128 _a,
    const M128 _b)
{
    return GetX(Dot3V(_a, _b));
}

[[nodiscard]] inline M128 Cross3(
    const M128 _a,
    const M128 _b)
{
    const M128 a_yzx = Shuffle<1, 2, 0, 3>(_a);
    const M128 b_zxy = Shuffle<2, 0, 1, 3>(_b);
    const M128 a_zxy = Shuffle<2, 0, 1, 3>(_a);
    const M128 b_yzx = Shuffle<1, 2, 0, 3>(_b);
    const M128 ret   = Sub(Mult(a_yzx, b_zxy), Mult(a_zxy, b_yzx));
    return ZeroW(ret);
}

[[nodiscard]] inline float LengthSq3(
    const M128 _a)
{
    return Dot3(_a, _a);
}

[[nodiscard]] inline float LengthSq4(
    const M128 _a)
{
    return Dot4(_a, _a);
}

[[nodiscard]] inline float Length3(
    const M128 _a)
{
    return GetX(Sqrt(Dot3V(_a, _a)));
}

[[nodiscard]] inline float Length4(
    const M128 _a)
{
    return GetX(Sqrt(Dot4V(_a, _a)));
}

[[nodiscard]] inline M128 Normalize3(
    const M128 _a)
{
    const M128 lenSq   = Dot3V(_a, _a);
    const M128 nonZero = CmpGt(lenSq, Zero());
    const M128 scaled  = Select(nonZero, Mult(_a, RSqrt(lenSq)), Zero());
    const M128 wMask   = MaskXYZ();
    return Or(And(scaled, wMask), AndNot(wMask, _a));
}

[[nodiscard]] inline M128 Normalize4(
    const M128 _a)
{
    const M128 lenSq   = Dot4V(_a, _a);
    const M128 nonZero = CmpGt(lenSq, Zero());
    return Select(nonZero, Mult(_a, RSqrt(lenSq)), Zero());
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] inline M128 Lerp(
    const M128 _a,
    const M128 _b,
    const M128 _t)
{
    return MultAdd(Sub(_b, _a), _t, _a);
}

[[nodiscard]] inline M128 Lerp(
    const M128  _a,
    const M128  _b,
    const float _t)
{
    return Lerp(_a, _b, SetAll(_t));
}

}   // namespace jug::simd

#    endif   // JUG_SIMD_AVAILABLE
#endif       // !defined(__CUDA_ARCH__) && !defined(JUG_MATH_NO_SIMD)
