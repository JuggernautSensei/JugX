#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// ===========================================
//  Calling Convention
// ===========================================

#if defined(_MSC_VER) && !defined(__CUDACC__) && (defined(_M_X64) || (defined(_M_IX86) && defined(_M_IX86_FP) && _M_IX86_FP >= 2)) && !defined(_M_ARM64EC) && !defined(_M_HYBRID_X86_ARM64)
#    define JUG_VECTORCALL __vectorcall
#else
#    define JUG_VECTORCALL
#endif

// ===========================================
//  Detection
// ===========================================

#if !defined(__CUDA_ARCH__) && !defined(JUG_MATH_NO_SIMD)
#    if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#        if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__)                           \
            || defined(__SSE4_2__) || defined(__SSE4_1__) || defined(__SSE3__) || defined(__SSE2__) \
            || defined(__x86_64__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#            include <emmintrin.h>
#            define JUG_MATH_SIMD_SSE 1
#        endif
#        if defined(__AVX__) || defined(__SSE4_1__)
#            include <smmintrin.h>
#            define JUG_MATH_SIMD_SSE4 1
#        endif
#        if defined(__AVX__)
#            include <immintrin.h>
#            define JUG_MATH_SIMD_AVX 1
#        endif
#        if defined(__FMA__) || defined(__AVX2__)
#            include <immintrin.h>
#            define JUG_MATH_SIMD_FMA 1
#        endif
#    elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64)
#        include <arm_neon.h>
#        define JUG_MATH_SIMD_NEON 1
#    endif
#endif

#if defined(JUG_MATH_SIMD_SSE) || defined(JUG_MATH_SIMD_NEON)
#    define JUG_SIMD_AVAILABLE 1
#endif

namespace jug::simd
{

#if defined(JUG_MATH_SIMD_NEON)
using M128 = float32x4_t;
#elif defined(JUG_MATH_SIMD_SSE)
using M128 = __m128;
#else
struct alignas(16) M128
{
    float f[4];
};
#endif

// =======================================================
//  Compile Time Access
// =======================================================

// constexpr 평가 전용. 런타임 경로는 Get<k> / Set 을 쓸 것.
[[nodiscard]] constexpr float GetLane(
    const M128   _v,
    const size_t _index)
{
#if !defined(JUG_SIMD_AVAILABLE)
    return _v.f[_index];
#elif defined(_MSC_VER) && !defined(__clang__)
#    if defined(JUG_MATH_SIMD_NEON)
    return _v.n128_f32[_index];
#    else
    return _v.m128_f32[_index];
#    endif
#else
    return _v[_index];
#endif
}

// g++ 는 constexpr 안에서 벡터 원소 쓰기를 못 하지만 {..} 생성은 된다.
[[nodiscard]] constexpr M128 MakeConstant(
    const float _x,
    const float _y,
    const float _z,
    const float _w)
{
#if !defined(JUG_SIMD_AVAILABLE)
    return M128 { { _x, _y, _z, _w } };
#elif defined(_MSC_VER) && !defined(__clang__)
    M128 ret {};
#    if defined(JUG_MATH_SIMD_NEON)
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
    return M128 { _x, _y, _z, _w };
#endif
}

// =======================================================
//  Load / Store / Set
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Load(
    const float _pFloat4[4])
{
#if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_loadu_ps(_pFloat4);
#else
    return M128 { { _pFloat4[0], _pFloat4[1], _pFloat4[2], _pFloat4[3] } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL LoadAligned(
    const float _pFloat4[4])
{
#if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_load_ps(_pFloat4);
#else
    return Load(_pFloat4);
#endif
}

// z = w = 0
[[nodiscard]] inline M128 JUG_VECTORCALL LoadFloat2(
    const float _pFloat2[2])
{
#if defined(JUG_MATH_SIMD_NEON)
    return vcombine_f32(vld1_f32(_pFloat2), vdup_n_f32(0.f));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat2)));
#else
    return M128 { { _pFloat2[0], _pFloat2[1], 0.f, 0.f } };
#endif
}

// w = 0
[[nodiscard]] inline M128 JUG_VECTORCALL LoadFloat3(
    const float _pFloat3[3])
{
#if defined(JUG_MATH_SIMD_NEON)
    const float32x4_t xy0 = vcombine_f32(vld1_f32(_pFloat3), vdup_n_f32(0.f));
    return vld1q_lane_f32(_pFloat3 + 2, xy0, 2);
#elif defined(JUG_MATH_SIMD_SSE4)
    const M128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3)));
    return _mm_insert_ps(xy, _mm_load_ss(_pFloat3 + 2), 0x20);
#elif defined(JUG_MATH_SIMD_SSE)
    const M128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3)));
    return _mm_movelh_ps(xy, _mm_load_ss(_pFloat3 + 2));
#else
    return M128 { { _pFloat3[0], _pFloat3[1], _pFloat3[2], 0.f } };
#endif
}

inline void JUG_VECTORCALL Store(
    float      _pOutFloat4[4],
    const M128 _value)
{
#if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#elif defined(JUG_MATH_SIMD_SSE)
    _mm_storeu_ps(_pOutFloat4, _value);
#else
    for (int i = 0; i < 4; ++i)
    {
        _pOutFloat4[i] = _value.f[i];
    }
#endif
}

inline void JUG_VECTORCALL StoreAligned(
    float      _pOutFloat4[4],
    const M128 _value)
{
#if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_ps(_pOutFloat4, _value);
#else
    Store(_pOutFloat4, _value);
#endif
}

inline void JUG_VECTORCALL StoreFloat2(
    float      _pOutFloat2[2],
    const M128 _value)
{
#if defined(JUG_MATH_SIMD_NEON)
    vst1_f32(_pOutFloat2, vget_low_f32(_value));
#elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat2), _mm_castps_pd(_value));
#else
    _pOutFloat2[0] = _value.f[0];
    _pOutFloat2[1] = _value.f[1];
#endif
}

inline void JUG_VECTORCALL StoreFloat3(
    float      _pOutFloat3[3],
    const M128 _value)
{
#if defined(JUG_MATH_SIMD_NEON)
    vst1_f32(_pOutFloat3, vget_low_f32(_value));
    vst1q_lane_f32(_pOutFloat3 + 2, _value, 2);
#elif defined(JUG_MATH_SIMD_SSE4)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
    *reinterpret_cast<int*>(_pOutFloat3 + 2) = _mm_extract_ps(_value, 2);
#elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
    _mm_store_ss(_pOutFloat3 + 2, _mm_shuffle_ps(_value, _value, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    _pOutFloat3[0] = _value.f[0];
    _pOutFloat3[1] = _value.f[1];
    _pOutFloat3[2] = _value.f[2];
#endif
}

[[nodiscard]] constexpr M128 JUG_VECTORCALL Set(
    const float _x,
    const float _y,
    const float _z,
    const float _w)
{
    if (std::is_constant_evaluated())
    {
        return MakeConstant(_x, _y, _z, _w);
    }
#if defined(JUG_MATH_SIMD_NEON)
    alignas(16) const float values[4] = { _x, _y, _z, _w };
    return vld1q_f32(values);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set_ps(_w, _z, _y, _x);
#else
    return MakeConstant(_x, _y, _z, _w);
#endif
}

[[nodiscard]] constexpr M128 JUG_VECTORCALL SetAll(
    const float _value)
{
    if (std::is_constant_evaluated())
    {
        return MakeConstant(_value, _value, _value, _value);
    }
#if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(_value);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set1_ps(_value);
#else
    return MakeConstant(_value, _value, _value, _value);
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Zero()
{
#if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(0.f);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_setzero_ps();
#else
    return M128 { { 0.f, 0.f, 0.f, 0.f } };
#endif
}

// =======================================================
//  Basic
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Add(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vaddq_f32(_a, _b);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_add_ps(_a, _b);
#else
    return M128 { { _a.f[0] + _b.f[0], _a.f[1] + _b.f[1], _a.f[2] + _b.f[2], _a.f[3] + _b.f[3] } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Sub(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vsubq_f32(_a, _b);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_a, _b);
#else
    return M128 { { _a.f[0] - _b.f[0], _a.f[1] - _b.f[1], _a.f[2] - _b.f[2], _a.f[3] - _b.f[3] } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Mult(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vmulq_f32(_a, _b);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_mul_ps(_a, _b);
#else
    return M128 { { _a.f[0] * _b.f[0], _a.f[1] * _b.f[1], _a.f[2] * _b.f[2], _a.f[3] * _b.f[3] } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Div(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
#    if defined(__aarch64__) || defined(_M_ARM64)
    return vdivq_f32(_a, _b);
#    else
    float32x4_t recip = vrecpeq_f32(_b);
    recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
    recip             = vmulq_f32(vrecpsq_f32(_b, recip), recip);
    return vmulq_f32(_a, recip);
#    endif
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_div_ps(_a, _b);
#else
    return M128 { { _a.f[0] / _b.f[0], _a.f[1] / _b.f[1], _a.f[2] / _b.f[2], _a.f[3] / _b.f[3] } };
#endif
}

// _a * _b + _c
[[nodiscard]] inline M128 JUG_VECTORCALL MultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#if defined(JUG_MATH_SIMD_NEON)
#    if defined(__aarch64__) || defined(_M_ARM64)
    return vfmaq_f32(_c, _a, _b);
#    else
    return vmlaq_f32(_c, _a, _b);
#    endif
#elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fmadd_ps(_a, _b, _c);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_add_ps(_mm_mul_ps(_a, _b), _c);
#else
    return Add(Mult(_a, _b), _c);
#endif
}

// _a * _b - _c
[[nodiscard]] inline M128 JUG_VECTORCALL MultSub(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vsubq_f32(vmulq_f32(_a, _b), _c);
#elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fmsub_ps(_a, _b, _c);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_mm_mul_ps(_a, _b), _c);
#else
    return Sub(Mult(_a, _b), _c);
#endif
}

// _c - _a * _b
[[nodiscard]] inline M128 JUG_VECTORCALL NegMultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#if defined(JUG_MATH_SIMD_NEON)
#    if defined(__aarch64__) || defined(_M_ARM64)
    return vfmsq_f32(_c, _a, _b);
#    else
    return vmlsq_f32(_c, _a, _b);
#    endif
#elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fnmadd_ps(_a, _b, _c);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_c, _mm_mul_ps(_a, _b));
#else
    return Sub(_c, Mult(_a, _b));
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Negate(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vnegq_f32(_a);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_xor_ps(_a, _mm_set1_ps(-0.f));
#else
    return M128 { { -_a.f[0], -_a.f[1], -_a.f[2], -_a.f[3] } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Sqrt(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
#    if defined(__aarch64__) || defined(_M_ARM64)
    return vsqrtq_f32(_a);
#    else
    float32x4_t y = vrsqrteq_f32(_a);
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    return vmulq_f32(_a, y);
#    endif
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sqrt_ps(_a);
#else
    return M128 { { ::sqrtf(_a.f[0]), ::sqrtf(_a.f[1]), ::sqrtf(_a.f[2]), ::sqrtf(_a.f[3]) } };
#endif
}

// 근사 역제곱근 + 뉴턴 1회 (상대오차 ~2e-7)
[[nodiscard]] inline M128 JUG_VECTORCALL RSqrt(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
    float32x4_t y = vrsqrteq_f32(_a);
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    return y;
#elif defined(JUG_MATH_SIMD_SSE)
    const M128 y0 = _mm_rsqrt_ps(_a);
    return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), y0), _mm_sub_ps(_mm_set1_ps(3.f), _mm_mul_ps(_a, _mm_mul_ps(y0, y0))));
#else
    return Div(M128 { { 1.f, 1.f, 1.f, 1.f } }, Sqrt(_a));
#endif
}

// =======================================================
//  Bitwise
// =======================================================

#if !defined(JUG_SIMD_AVAILABLE)
[[nodiscard]] inline uint32_t Bits(
    const float _x)
{
    return std::bit_cast<uint32_t>(_x);
}

[[nodiscard]] inline float FromBits(
    const uint32_t _x)
{
    return std::bit_cast<float>(_x);
}
#endif

[[nodiscard]] inline M128 JUG_VECTORCALL And(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_and_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = FromBits(Bits(_a.f[i]) & Bits(_b.f[i]));
    }
    return ret;
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Or(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = FromBits(Bits(_a.f[i]) | Bits(_b.f[i]));
    }
    return ret;
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Xor(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_xor_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = FromBits(Bits(_a.f[i]) ^ Bits(_b.f[i]));
    }
    return ret;
#endif
}

// (~_a) & _b
[[nodiscard]] inline M128 JUG_VECTORCALL AndNot(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(_b), vreinterpretq_u32_f32(_a)));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = FromBits(~Bits(_a.f[i]) & Bits(_b.f[i]));
    }
    return ret;
#endif
}

// w 레인만 0 으로
[[nodiscard]] inline M128 JUG_VECTORCALL ZeroW(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vcombine_u32(vcreate_u32(0xffffffffffffffffull), vcreate_u32(0x00000000ffffffffull))));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_and_ps(_a, _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0)));
#else
    return M128 { { _a.f[0], _a.f[1], _a.f[2], 0.f } };
#endif
}

// 32비트 레인마다 정수 왼쪽 시프트
template<int kShift>
[[nodiscard]] M128 JUG_VECTORCALL ShiftLeft(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vshlq_n_u32(vreinterpretq_u32_f32(_a), kShift));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(_a), kShift));
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = FromBits(Bits(_a.f[i]) << kShift);
    }
    return ret;
#endif
}

// _mask 가 켜진 요소는 _a, 꺼진 요소는 _b
[[nodiscard]] inline M128 JUG_VECTORCALL Select(
    const M128 _mask,
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vbslq_f32(vreinterpretq_u32_f32(_mask), _a, _b);
#elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_blendv_ps(_b, _a, _mask);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_mm_and_ps(_mask, _a), _mm_andnot_ps(_mask, _b));
#else
    return Or(And(_mask, _a), AndNot(_mask, _b));
#endif
}

// kMask 의 i 번째 비트가 켜진 요소는 _b, 꺼진 요소는 _a
template<int kMask>
[[nodiscard]] M128 JUG_VECTORCALL Blend(
    const M128 _a,
    const M128 _b)
{
    static_assert(kMask >= 0 && kMask <= 15, "Blend mask must be within [0, 15]");
#if defined(JUG_MATH_SIMD_NEON)
    alignas(16) static constexpr uint32_t kBits[4] = { (kMask & 1) ? 0xffffffffu : 0u, (kMask & 2) ? 0xffffffffu : 0u, (kMask & 4) ? 0xffffffffu : 0u, (kMask & 8) ? 0xffffffffu : 0u };
    return vbslq_f32(vld1q_u32(kBits), _b, _a);
#elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_blend_ps(_a, _b, kMask);
#elif defined(JUG_MATH_SIMD_SSE)
    const M128 mask = _mm_castsi128_ps(_mm_setr_epi32((kMask & 1) ? -1 : 0, (kMask & 2) ? -1 : 0, (kMask & 4) ? -1 : 0, (kMask & 8) ? -1 : 0));
    return _mm_or_ps(_mm_andnot_ps(mask, _a), _mm_and_ps(mask, _b));
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = (kMask & (1 << i)) ? _b.f[i] : _a.f[i];
    }
    return ret;
#endif
}

// =======================================================
//  Comparison
// =======================================================

#if !defined(JUG_SIMD_AVAILABLE)
[[nodiscard]] inline float MaskOf(
    const bool _value)
{
    return FromBits(_value ? 0xffffffffu : 0u);
}
#endif

[[nodiscard]] inline M128 JUG_VECTORCALL CmpEq(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vceqq_f32(_a, _b));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpeq_ps(_a, _b);
#else
    return M128 { { MaskOf(_a.f[0] == _b.f[0]), MaskOf(_a.f[1] == _b.f[1]), MaskOf(_a.f[2] == _b.f[2]), MaskOf(_a.f[3] == _b.f[3]) } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpNe(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(_a, _b)));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpneq_ps(_a, _b);
#else
    return M128 { { MaskOf(_a.f[0] != _b.f[0]), MaskOf(_a.f[1] != _b.f[1]), MaskOf(_a.f[2] != _b.f[2]), MaskOf(_a.f[3] != _b.f[3]) } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpLt(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcltq_f32(_a, _b));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmplt_ps(_a, _b);
#else
    return M128 { { MaskOf(_a.f[0] < _b.f[0]), MaskOf(_a.f[1] < _b.f[1]), MaskOf(_a.f[2] < _b.f[2]), MaskOf(_a.f[3] < _b.f[3]) } };
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpLe(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcleq_f32(_a, _b));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmple_ps(_a, _b);
#else
    return M128 { { MaskOf(_a.f[0] <= _b.f[0]), MaskOf(_a.f[1] <= _b.f[1]), MaskOf(_a.f[2] <= _b.f[2]), MaskOf(_a.f[3] <= _b.f[3]) } };
#endif
}

// 레인마다 부호 비트를 모은다 (하위 4비트)
[[nodiscard]] inline int JUG_VECTORCALL MoveMask(
    const M128 _mask)
{
#if defined(JUG_MATH_SIMD_NEON)
    const uint32x4_t bits   = vcombine_u32(vcreate_u32(0x0000000200000001ull), vcreate_u32(0x0000000800000004ull));
    const uint32x4_t masked = vandq_u32(vreinterpretq_u32_f32(_mask), bits);
#    if defined(__aarch64__) || defined(_M_ARM64)
    return static_cast<int>(vaddvq_u32(masked));
#    else
    uint32x2_t sum2 = vadd_u32(vget_low_u32(masked), vget_high_u32(masked));
    sum2            = vpadd_u32(sum2, sum2);
    return static_cast<int>(vget_lane_u32(sum2, 0));
#    endif
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_movemask_ps(_mask);
#else
    int ret = 0;
    for (int i = 0; i < 4; ++i)
    {
        ret |= static_cast<int>(Bits(_mask.f[i]) >> 31) << i;
    }
    return ret;
#endif
}

// =======================================================
//  Min / Max / Abs
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Min(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vminq_f32(_a, _b);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_min_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = _a.f[i] < _b.f[i] ? _a.f[i] : _b.f[i];
    }
    return ret;
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Max(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vmaxq_f32(_a, _b);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_max_ps(_a, _b);
#else
    M128 ret;
    for (int i = 0; i < 4; ++i)
    {
        ret.f[i] = _a.f[i] > _b.f[i] ? _a.f[i] : _b.f[i];
    }
    return ret;
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Abs(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON)
    return vabsq_f32(_a);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_mm_set1_ps(-0.f), _a);
#else
    return AndNot(M128 { { -0.f, -0.f, -0.f, -0.f } }, _a);
#endif
}

// =======================================================
//  Shuffle / Splat / Get / Insert
// =======================================================

template<int X, int Y, int Z, int W>
[[nodiscard]] M128 JUG_VECTORCALL Shuffle(
    const M128 _a)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle indices must be within [0, 3]");
#if defined(JUG_MATH_SIMD_NEON) && (defined(__clang__) || defined(__GNUC__))
    return __builtin_shufflevector(_a, _a, X, Y, Z, W);
#elif defined(JUG_MATH_SIMD_NEON)
    alignas(16) float tmp[4];
    vst1q_f32(tmp, _a);
    alignas(16) const float ret[4] = { tmp[X], tmp[Y], tmp[Z], tmp[W] };
    return vld1q_f32(ret);
#elif defined(JUG_MATH_SIMD_AVX)
    return _mm_permute_ps(_a, _MM_SHUFFLE(W, Z, Y, X));
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(W, Z, Y, X));
#else
    return M128 { { _a.f[X], _a.f[Y], _a.f[Z], _a.f[W] } };
#endif
}

// x, y 는 _a 에서, z, w 는 _b 에서
template<int X, int Y, int Z, int W>
[[nodiscard]] M128 JUG_VECTORCALL Shuffle2(
    const M128 _a,
    const M128 _b)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle2 indices must be within [0, 3]");
#if defined(JUG_MATH_SIMD_NEON) && (defined(__clang__) || defined(__GNUC__))
    return __builtin_shufflevector(_a, _b, X, Y, Z + 4, W + 4);
#elif defined(JUG_MATH_SIMD_NEON)
    alignas(16) float ta[4];
    vst1q_f32(ta, _a);
    alignas(16) float tb[4];
    vst1q_f32(tb, _b);
    alignas(16) const float ret[4] = { ta[X], ta[Y], tb[Z], tb[W] };
    return vld1q_f32(ret);
#elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _b, _MM_SHUFFLE(W, Z, Y, X));
#else
    return M128 { { _a.f[X], _a.f[Y], _b.f[Z], _b.f[W] } };
#endif
}

template<int kIndex>
[[nodiscard]] M128 JUG_VECTORCALL Splat(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Splat index must be within [0, 3]");
#if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(vgetq_lane_f32(_a, kIndex));
#else
    return Shuffle<kIndex, kIndex, kIndex, kIndex>(_a);
#endif
}

template<int kIndex>
[[nodiscard]] float JUG_VECTORCALL Get(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Get index must be within [0, 3]");
#if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, kIndex);
#elif defined(JUG_MATH_SIMD_SSE)
    if constexpr (kIndex == 0)
    {
        return _mm_cvtss_f32(_a);
    }
    else
    {
        return _mm_cvtss_f32(Shuffle<kIndex, kIndex, kIndex, kIndex>(_a));
    }
#else
    return _a.f[kIndex];
#endif
}

[[nodiscard]] inline float JUG_VECTORCALL GetX(
    const M128 _a)
{
    return Get<0>(_a);
}

// kIndex 요소를 _value 로 바꾼 벡터
template<int kIndex>
[[nodiscard]] M128 JUG_VECTORCALL Insert(
    const M128  _a,
    const float _value)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Insert index must be within [0, 3]");
#if defined(JUG_MATH_SIMD_NEON)
    return vsetq_lane_f32(_value, _a, kIndex);
#elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_insert_ps(_a, _mm_set_ss(_value), kIndex << 4);
#elif defined(JUG_MATH_SIMD_SSE)
    if constexpr (kIndex == 0)
    {
        return _mm_move_ss(_a, _mm_set_ss(_value));
    }
    else
    {
        // kIndex 요소를 x 자리로 가져와 바꾸고 다시 돌려놓는다
        constexpr int kSwap = kIndex == 1 ? _MM_SHUFFLE(3, 2, 0, 1) : kIndex == 2 ? _MM_SHUFFLE(3, 0, 1, 2)
                                                                                  : _MM_SHUFFLE(0, 2, 1, 3);
        const M128    swap  = _mm_shuffle_ps(_a, _a, kSwap);
        const M128    moved = _mm_move_ss(swap, _mm_set_ss(_value));
        return _mm_shuffle_ps(moved, moved, kSwap);
    }
#else
    M128 ret      = _a;
    ret.f[kIndex] = _value;
    return ret;
#endif
}

inline void JUG_VECTORCALL Transpose4(
    M128& _row0,
    M128& _row1,
    M128& _row2,
    M128& _row3)
{
#if defined(JUG_MATH_SIMD_NEON)
    const float32x4x2_t t0 = vtrnq_f32(_row0, _row1);
    const float32x4x2_t t1 = vtrnq_f32(_row2, _row3);
    _row0                  = vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0]));
    _row1                  = vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1]));
    _row2                  = vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0]));
    _row3                  = vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1]));
#elif defined(JUG_MATH_SIMD_SSE)
    const M128 t0 = _mm_unpacklo_ps(_row0, _row1);
    const M128 t1 = _mm_unpacklo_ps(_row2, _row3);
    const M128 t2 = _mm_unpackhi_ps(_row0, _row1);
    const M128 t3 = _mm_unpackhi_ps(_row2, _row3);
    _row0         = _mm_movelh_ps(t0, t1);
    _row1         = _mm_movehl_ps(t1, t0);
    _row2         = _mm_movelh_ps(t2, t3);
    _row3         = _mm_movehl_ps(t3, t2);
#else
    const M128 r0 = _row0;
    const M128 r1 = _row1;
    const M128 r2 = _row2;
    const M128 r3 = _row3;
    _row0         = M128 { { r0.f[0], r1.f[0], r2.f[0], r3.f[0] } };
    _row1         = M128 { { r0.f[1], r1.f[1], r2.f[1], r3.f[1] } };
    _row2         = M128 { { r0.f[2], r1.f[2], r2.f[2], r3.f[2] } };
    _row3         = M128 { { r0.f[3], r1.f[3], r2.f[3], r3.f[3] } };
#endif
}

// =======================================================
//  Vector
// =======================================================

// 네 요소의 합을 모든 요소에
[[nodiscard]] inline M128 JUG_VECTORCALL SumAll(
    const M128 _a)
{
#if defined(JUG_MATH_SIMD_NEON) && (defined(__aarch64__) || defined(_M_ARM64))
    return vdupq_n_f32(vaddvq_f32(_a));
#else
    const M128 sums = Add(_a, Shuffle<1, 0, 3, 2>(_a));   // x+y x+y z+w z+w
    return Add(sums, Shuffle<2, 3, 0, 1>(sums));
#endif
}

// dpps 는 쓰지 않는다 - Zen4 에서 셔플 + 덧셈보다 두 배 이상 느리다
[[nodiscard]] inline M128 JUG_VECTORCALL Dot4V(
    const M128 _a,
    const M128 _b)
{
    return SumAll(Mult(_a, _b));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Dot3V(
    const M128 _a,
    const M128 _b)
{
#if defined(JUG_MATH_SIMD_SSE)
    const M128 m  = _mm_mul_ps(_a, _b);
    const M128 yz = Shuffle<1, 2, 1, 2>(m);
    M128       s  = _mm_add_ss(m, yz);
    s             = _mm_add_ss(s, Shuffle<1, 1, 1, 1>(yz));
    return Shuffle<0, 0, 0, 0>(s);
#else
    const M128 m = Mult(_a, _b);
    return Add(Add(Splat<0>(m), Splat<1>(m)), Splat<2>(m));
#endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Dot2V(
    const M128 _a,
    const M128 _b)
{
    const M128 m = Mult(_a, _b);
    return Splat<0>(Add(m, Shuffle<1, 1, 1, 1>(m)));
}

// w 는 보장하지 않는다 - xyz 만 쓸 것
[[nodiscard]] inline M128 JUG_VECTORCALL Cross3(
    const M128 _a,
    const M128 _b)
{
    // (a * b.yzx - a.yzx * b).yzx
    const M128 aYzx = Shuffle<1, 2, 0, 3>(_a);
    const M128 bYzx = Shuffle<1, 2, 0, 3>(_b);
    return Shuffle<1, 2, 0, 3>(MultSub(_a, bYzx, Mult(aYzx, _b)));
}

// 길이² 가 모든 레인에 있는 _lenSq 로 정규화. 0 이면 0 (DirectXMath 와 같은 의미).
// rsqrt + 뉴턴 1회로 나눗셈·제곱근 장치를 쓰지 않는다. 길이² 가 정규 float 미만이면 (|v| < 1e-19) 정확한 경로.
[[nodiscard]] inline M128 JUG_VECTORCALL NormalizeBy(
    const M128 _a,
    const M128 _lenSq)
{
    if (GetX(_lenSq) < 1.17549435e-38f)
    {
        return And(Div(_a, Sqrt(_lenSq)), CmpNe(_lenSq, Zero()));
    }
    return Mult(_a, RSqrt(_lenSq));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Normalize3(
    const M128 _a)
{
    return NormalizeBy(_a, Dot3V(_a, _a));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Lerp(
    const M128 _a,
    const M128 _b,
    const M128 _t)
{
    return MultAdd(Sub(_b, _a), _t, _a);
}

// =======================================================
//  Trigonometry
// =======================================================

// x = q*pi + r (|r| <= pi/2). sin(x) = (-1)^q sin(r), cos(x) = (-1)^q cos(r) 이라 결과에 q 의 홀짝을 부호로 xor 하면 된다.
// q 는 1.5*2^23 을 더해 반올림하고 (|x| < 2^22*pi), 그때 가수 최하위 비트가 곧 q 의 홀짝이다.
[[nodiscard]] inline M128 JUG_VECTORCALL ReduceAngle(
    const M128 _x,
    M128&      _outSign)
{
    const M128 magic = SetAll(12582912.f);
    const M128 t     = MultAdd(_x, SetAll(0.318309886f), magic);
    _outSign         = ShiftLeft<31>(t);
    return NegMultAdd(Sub(t, magic), SetAll(3.141592654f), _x);
}

// DirectXMath XMVectorSinCos 와 같은 계수 (sin 11차, cos 10차 minimax) 를 Estrin 으로 (곱셈-덧셈 사슬 3단)
inline void JUG_VECTORCALL SinCos(
    const M128 _x,
    M128&      _outSin,
    M128&      _outCos)
{
    M128       sign;
    const M128 r  = ReduceAngle(_x, sign);
    const M128 r2 = Mult(r, r);
    const M128 r4 = Mult(r2, r2);

    const M128 s01 = MultAdd(SetAll(-0.16666667f), r2, SetAll(1.f));
    const M128 s23 = MultAdd(SetAll(-0.00019840874f), r2, SetAll(0.0083333310f));
    const M128 s45 = MultAdd(SetAll(-2.3889859e-08f), r2, SetAll(2.7525562e-06f));
    _outSin        = Xor(Mult(MultAdd(MultAdd(s45, r4, s23), r4, s01), r), sign);

    const M128 c01 = MultAdd(SetAll(-0.5f), r2, SetAll(1.f));
    const M128 c23 = MultAdd(SetAll(-0.0013888378f), r2, SetAll(0.041666638f));
    const M128 c45 = MultAdd(SetAll(-2.6051615e-07f), r2, SetAll(2.4760495e-05f));
    _outCos        = Xor(MultAdd(MultAdd(c45, r4, c23), r4, c01), sign);
}

[[nodiscard]] inline M128 JUG_VECTORCALL Sin(
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

[[nodiscard]] inline M128 JUG_VECTORCALL Cos(
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

// 스칼라 각 하나의 (sin, cos, sin, cos). 두 다항식을 한 레지스터의 서로 다른 레인에서 같이 돌린다.
[[nodiscard]] inline M128 JUG_VECTORCALL SinCos1(
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
    return Xor(Mult(p, Blend<0b1010>(r, SetAll(1.f))), sign);   // sin 레인에만 r 을 곱한다
}

}   // namespace jug::simd

#ifdef JUG_SIMD_AVAILABLE
#    define JUG_SIMD_PATH(...)             \
        JUG_BEGIN_MACRO_BLOCK              \
        if (!std::is_constant_evaluated()) \
        {                                  \
            return __VA_ARGS__;            \
        }                                  \
        JUG_END_MACRO_BLOCK
#else
#    define JUG_SIMD_PATH(...)
#endif
