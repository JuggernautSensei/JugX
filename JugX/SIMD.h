#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

// ===========================================
//  Calling Convention
// ===========================================

// DirectXMath 의 XM_CALLCONV 와 같은 역할. __vectorcall 은 __m128 과 HVA(__m128 최대 4개로만 이뤄진 구조체)를
// 인자·반환 모두 xmm 레지스터로 주고받는다. 인라인이 안 된 호출도 메모리를 거치지 않는다.
#if defined(_MSC_VER) && !defined(__CUDACC__) && (defined(_M_X64) || (defined(_M_IX86) && defined(_M_IX86_FP) && _M_IX86_FP >= 2)) && !defined(_M_ARM64EC) && !defined(_M_HYBRID_X86_ARM64)
#    define JUG_VECTORCALL __vectorcall
#else
#    define JUG_VECTORCALL
#endif

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
#        if defined(__AVX__) || defined(__SSE4_1__)
#            include <smmintrin.h>
#            define JUG_MATH_SIMD_SSE4 1
#        endif
#        if defined(__AVX__)
#            include <immintrin.h>
#            define JUG_MATH_SIMD_AVX 1
#        endif
#        if defined(__FMA__) || defined(__AVX2__)   // MSVC 는 /arch:AVX2 에서 FMA3 를 켜지만 __FMA__ 를 정의하지 않는다.
#            include <immintrin.h>
#            define JUG_MATH_SIMD_FMA 1
#        endif
#    elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64)
#        include <arm_neon.h>
#        define JUG_MATH_SIMD_NEON 1
#    endif

#    if defined(JUG_MATH_SIMD_SSE) || defined(JUG_MATH_SIMD_NEON)
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
//  Compile Time Access
// =======================================================

// constexpr 평가 전용. 런타임에 쓰면 레지스터 값을 메모리로 내렸다가 읽는다 - 런타임 경로는 아래 intrinsic 을 쓸 것.
// MSVC 의 __m128 / __n128 은 union 이라 m128_f32 / n128_f32 로, clang·gcc 는 벡터 확장 타입이라 첨자로 읽는다.
[[nodiscard]] constexpr float GetLane(
    const M128   _v,
    const size_t _index)
{
#        if defined(_MSC_VER) && !defined(__clang__)
#            if defined(JUG_MATH_SIMD_NEON)
    return _v.n128_f32[_index];
#            else
    return _v.m128_f32[_index];
#            endif
#        else
    return _v[_index];
#        endif
}

// constexpr 평가 전용. g++ 는 constexpr 안에서 벡터 원소 쓰기를 못 하지만 {..} 생성은 된다.
[[nodiscard]] constexpr M128 MakeConstant(
    const float _x,
    const float _y,
    const float _z,
    const float _w)
{
#        if defined(_MSC_VER) && !defined(__clang__)
    M128 ret {};
#            if defined(JUG_MATH_SIMD_NEON)
    ret.n128_f32[0] = _x;
    ret.n128_f32[1] = _y;
    ret.n128_f32[2] = _z;
    ret.n128_f32[3] = _w;
#            else
    ret.m128_f32[0] = _x;
    ret.m128_f32[1] = _y;
    ret.m128_f32[2] = _z;
    ret.m128_f32[3] = _w;
#            endif
    return ret;
#        else
    return M128 { _x, _y, _z, _w };
#        endif
}

// =======================================================
//  Load / Store / Set
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Load(
    const float _pFloat4[4])
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_loadu_ps(_pFloat4);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL LoadAligned(
    const float _pFloat4[4])
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vld1q_f32(_pFloat4);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_load_ps(_pFloat4);
#        endif
}

inline void JUG_VECTORCALL Store(
    float      _pOutFloat4[4],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_storeu_ps(_pOutFloat4, _value);
#        endif
}

inline void JUG_VECTORCALL StoreAligned(
    float      _pOutFloat4[4],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1q_f32(_pOutFloat4, _value);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_ps(_pOutFloat4, _value);
#        endif
}

// w = 0
[[nodiscard]] inline M128 JUG_VECTORCALL LoadFloat3(
    const float _pFloat3[3])
{
#        if defined(JUG_MATH_SIMD_NEON)
    const float32x4_t xy0 = vcombine_f32(vld1_f32(_pFloat3), vdup_n_f32(0.f));
    return vld1q_lane_f32(_pFloat3 + 2, xy0, 2);
#        elif defined(JUG_MATH_SIMD_SSE4)
    const M128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3)));
    return _mm_insert_ps(xy, _mm_load_ss(_pFloat3 + 2), 0x20);
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(_pFloat3)));
    const M128 z  = _mm_load_ss(_pFloat3 + 2);
    return _mm_movelh_ps(xy, z);
#        endif
}

// x, y 만 저장 (8바이트)
inline void JUG_VECTORCALL StoreFloat2(
    float      _pOutFloat2[2],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1_f32(_pOutFloat2, vget_low_f32(_value));
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat2), _mm_castps_pd(_value));
#        endif
}

inline void JUG_VECTORCALL StoreFloat3(
    float      _pOutFloat3[3],
    const M128 _value)
{
#        if defined(JUG_MATH_SIMD_NEON)
    vst1_f32(_pOutFloat3, vget_low_f32(_value));
    vst1q_lane_f32(_pOutFloat3 + 2, _value, 2);
#        elif defined(JUG_MATH_SIMD_SSE4)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
    *reinterpret_cast<int*>(_pOutFloat3 + 2) = _mm_extract_ps(_value, 2);
#        elif defined(JUG_MATH_SIMD_SSE)
    _mm_store_sd(reinterpret_cast<double*>(_pOutFloat3), _mm_castps_pd(_value));
    _mm_store_ss(_pOutFloat3 + 2, _mm_movehl_ps(_value, _value));
#        endif
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
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) const float values[4] = { _x, _y, _z, _w };
    return vld1q_f32(values);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set_ps(_w, _z, _y, _x);
#        endif
}

[[nodiscard]] constexpr M128 JUG_VECTORCALL SetAll(
    const float _value)
{
    if (std::is_constant_evaluated())
    {
        return MakeConstant(_value, _value, _value, _value);
    }
#        if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(_value);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_set1_ps(_value);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Zero()
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

[[nodiscard]] inline M128 JUG_VECTORCALL Add(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vaddq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_add_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Sub(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vsubq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Mult(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vmulq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_mul_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Div(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
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
[[nodiscard]] inline M128 JUG_VECTORCALL MultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
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

// _a * _b - _c
[[nodiscard]] inline M128 JUG_VECTORCALL MultSub(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vsubq_f32(vmulq_f32(_a, _b), _c);
#        elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fmsub_ps(_a, _b, _c);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_mm_mul_ps(_a, _b), _c);
#        endif
}

// _c - _a * _b
[[nodiscard]] inline M128 JUG_VECTORCALL NegMultAdd(
    const M128 _a,
    const M128 _b,
    const M128 _c)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
    return vfmsq_f32(_c, _a, _b);
#            else
    return vmlsq_f32(_c, _a, _b);
#            endif
#        elif defined(JUG_MATH_SIMD_FMA)
    return _mm_fnmadd_ps(_a, _b, _c);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sub_ps(_c, _mm_mul_ps(_a, _b));
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Scale(
    const M128  _a,
    const float _scalar)
{
    return Mult(_a, SetAll(_scalar));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Negate(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vnegq_f32(_a);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_xor_ps(_a, _mm_set1_ps(-0.f));
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Sqrt(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
    return vsqrtq_f32(_a);
#            else
    float32x4_t y = vrsqrteq_f32(_a);
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
    return vmulq_f32(_a, y);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_sqrt_ps(_a);
#        endif
}

// 근사 역제곱근 + 뉴턴 1회 (상대오차 ~1e-7)
[[nodiscard]] inline M128 JUG_VECTORCALL RSqrt(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    float32x4_t y = vrsqrteq_f32(_a);
    y             = vmulq_f32(y, vrsqrtsq_f32(vmulq_f32(_a, y), y));
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

[[nodiscard]] inline M128 JUG_VECTORCALL And(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_and_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Or(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Xor(
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
[[nodiscard]] inline M128 JUG_VECTORCALL AndNot(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(_b), vreinterpretq_u32_f32(_a)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL MaskXYZ()
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcombine_u32(vcreate_u32(0xffffffffffffffffull), vcreate_u32(0x00000000ffffffffull)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL ZeroW(
    const M128 _a)
{
    return And(_a, MaskXYZ());
}

// _mask 가 켜진 요소는 _a, 꺼진 요소는 _b
[[nodiscard]] inline M128 JUG_VECTORCALL Select(
    const M128 _mask,
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vbslq_f32(vreinterpretq_u32_f32(_mask), _a, _b);
#        elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_blendv_ps(_b, _a, _mask);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_or_ps(_mm_and_ps(_mask, _a), _mm_andnot_ps(_mask, _b));
#        endif
}

// kMask 의 i 번째 비트가 켜진 요소는 _b, 꺼진 요소는 _a (상수 마스크 Select)
template<int kMask>
[[nodiscard]] M128 JUG_VECTORCALL Blend(
    const M128 _a,
    const M128 _b)
{
    static_assert(kMask >= 0 && kMask <= 15, "Blend mask must be within [0, 15]");
#        if defined(JUG_MATH_SIMD_NEON)
    alignas(16) static constexpr uint32_t kBits[4] = { (kMask & 1) ? 0xffffffffu : 0u, (kMask & 2) ? 0xffffffffu : 0u, (kMask & 4) ? 0xffffffffu : 0u, (kMask & 8) ? 0xffffffffu : 0u };
    return vbslq_f32(vld1q_u32(kBits), _b, _a);
#        elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_blend_ps(_a, _b, kMask);
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 mask = _mm_castsi128_ps(_mm_setr_epi32((kMask & 1) ? -1 : 0, (kMask & 2) ? -1 : 0, (kMask & 4) ? -1 : 0, (kMask & 8) ? -1 : 0));
    return _mm_or_ps(_mm_andnot_ps(mask, _a), _mm_and_ps(mask, _b));
#        endif
}

// =======================================================
//  Comparison
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL CmpEq(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vceqq_f32(_a, _b));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpeq_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpNe(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(_a, _b)));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpneq_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpLt(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcltq_f32(_a, _b));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmplt_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpLe(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcleq_f32(_a, _b));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmple_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpGt(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcgtq_f32(_a, _b));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpgt_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL CmpGe(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vreinterpretq_f32_u32(vcgeq_f32(_a, _b));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_cmpge_ps(_a, _b);
#        endif
}

// 마스크를 int로 변환. 하위 4비트만 사용
[[nodiscard]] inline int JUG_VECTORCALL MoveMask(
    const M128 _mask)
{
#        if defined(JUG_MATH_SIMD_NEON)
    const uint32x4_t bits   = vcombine_u32(vcreate_u32(0x0000000200000001ull), vcreate_u32(0x0000000800000004ull));
    const uint32x4_t masked = vandq_u32(vreinterpretq_u32_f32(_mask), bits);
#            if defined(__aarch64__) || defined(_M_ARM64)
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

[[nodiscard]] inline bool JUG_VECTORCALL AllTrue(
    const M128 _mask)
{
    return MoveMask(_mask) == 0xf;
}

[[nodiscard]] inline bool JUG_VECTORCALL AllTrue3(
    const M128 _mask)
{
    return (MoveMask(_mask) & 0x7) == 0x7;
}

[[nodiscard]] inline bool JUG_VECTORCALL AnyTrue(
    const M128 _mask)
{
    return MoveMask(_mask) != 0;
}

// =======================================================
//  Min / Max / Clamp / Abs / Round
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Min(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vminq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_min_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Max(
    const M128 _a,
    const M128 _b)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vmaxq_f32(_a, _b);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_max_ps(_a, _b);
#        endif
}

[[nodiscard]] inline M128 JUG_VECTORCALL Clamp(
    const M128 _value,
    const M128 _min,
    const M128 _max)
{
    return Max(_min, Min(_max, _value));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Abs(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
    return vabsq_f32(_a);
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_andnot_ps(_mm_set1_ps(-0.f), _a);
#        endif
}

// 가장 가까운 정수 (짝수 우선). |x| >= 2^23 은 이미 정수라 그대로 둔다.
[[nodiscard]] inline M128 JUG_VECTORCALL Round(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
    return vrndnq_f32(_a);
#            else
    const float32x4_t magic = vreinterpretq_f32_u32(vorrq_u32(vandq_u32(vreinterpretq_u32_f32(_a), vdupq_n_u32(0x80000000u)), vdupq_n_u32(0x4b000000u)));
    const float32x4_t r     = vsubq_f32(vaddq_f32(_a, magic), magic);
    const uint32x4_t  small = vcltq_f32(vabsq_f32(_a), vdupq_n_f32(8388608.f));
    return vbslq_f32(small, r, _a);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_round_ps(_a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 sign  = _mm_and_ps(_a, _mm_set1_ps(-0.f));
    const M128 magic = _mm_or_ps(sign, _mm_set1_ps(8388608.f));
    const M128 r     = _mm_sub_ps(_mm_add_ps(_a, magic), magic);
    const M128 small = _mm_cmplt_ps(_mm_andnot_ps(_mm_set1_ps(-0.f), _a), _mm_set1_ps(8388608.f));
    return _mm_or_ps(_mm_and_ps(small, r), _mm_andnot_ps(small, _a));
#        endif
}

// =======================================================
//  Shuffle / Splat / Get / Insert
// =======================================================

template<int X, int Y, int Z, int W>
[[nodiscard]] M128 JUG_VECTORCALL Shuffle(
    const M128 _a)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle indices must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON) && (defined(__clang__) || defined(__GNUC__))
    return __builtin_shufflevector(_a, _a, X, Y, Z, W);
#        elif defined(JUG_MATH_SIMD_NEON)
    alignas(16) float tmp[4];
    vst1q_f32(tmp, _a);
    alignas(16) const float ret[4] = { tmp[X], tmp[Y], tmp[Z], tmp[W] };
    return vld1q_f32(ret);
#        elif defined(JUG_MATH_SIMD_AVX)
    return _mm_permute_ps(_a, _MM_SHUFFLE(W, Z, Y, X));
#        elif defined(JUG_MATH_SIMD_SSE)
    return _mm_shuffle_ps(_a, _a, _MM_SHUFFLE(W, Z, Y, X));
#        endif
}

// x, y 는 _a 에서, z, w 는 _b 에서
template<int X, int Y, int Z, int W>
[[nodiscard]] M128 JUG_VECTORCALL Shuffle2(
    const M128 _a,
    const M128 _b)
{
    static_assert(X >= 0 && X <= 3 && Y >= 0 && Y <= 3 && Z >= 0 && Z <= 3 && W >= 0 && W <= 3, "Shuffle2 indices must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON) && (defined(__clang__) || defined(__GNUC__))
    return __builtin_shufflevector(_a, _b, X, Y, Z + 4, W + 4);
#        elif defined(JUG_MATH_SIMD_NEON)
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
[[nodiscard]] M128 JUG_VECTORCALL Splat(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Splat index must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    return vdupq_n_f32(vgetq_lane_f32(_a, kIndex));
#        else
    return Shuffle<kIndex, kIndex, kIndex, kIndex>(_a);
#        endif
}

template<int kIndex>
[[nodiscard]] float JUG_VECTORCALL Get(
    const M128 _a)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Get index must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    return vgetq_lane_f32(_a, kIndex);
#        elif defined(JUG_MATH_SIMD_SSE)
    if constexpr (kIndex == 0)
    {
        return _mm_cvtss_f32(_a);
    }
    else
    {
        return _mm_cvtss_f32(Shuffle<kIndex, kIndex, kIndex, kIndex>(_a));
    }
#        endif
}

[[nodiscard]] inline float JUG_VECTORCALL GetX(
    const M128 _a)
{
    return Get<0>(_a);
}

[[nodiscard]] inline float JUG_VECTORCALL GetY(
    const M128 _a)
{
    return Get<1>(_a);
}

[[nodiscard]] inline float JUG_VECTORCALL GetZ(
    const M128 _a)
{
    return Get<2>(_a);
}

[[nodiscard]] inline float JUG_VECTORCALL GetW(
    const M128 _a)
{
    return Get<3>(_a);
}

// kIndex 요소를 _value 로 바꾼 벡터
template<int kIndex>
[[nodiscard]] M128 JUG_VECTORCALL Insert(
    const M128  _a,
    const float _value)
{
    static_assert(kIndex >= 0 && kIndex <= 3, "Insert index must be within [0, 3]");
#        if defined(JUG_MATH_SIMD_NEON)
    return vsetq_lane_f32(_value, _a, kIndex);
#        elif defined(JUG_MATH_SIMD_SSE4)
    return _mm_insert_ps(_a, _mm_set_ss(_value), kIndex << 4);
#        elif defined(JUG_MATH_SIMD_SSE)
    if constexpr (kIndex == 0)
    {
        return _mm_move_ss(_a, _mm_set_ss(_value));
    }
    else
    {
        // kIndex 요소를 x 자리로 가져와 바꾸고 다시 돌려놓는다
        constexpr int kSwap = kIndex == 1 ? _MM_SHUFFLE(3, 2, 0, 1) : kIndex == 2 ? _MM_SHUFFLE(3, 0, 1, 2) : _MM_SHUFFLE(0, 2, 1, 3);
        const M128    swap  = _mm_shuffle_ps(_a, _a, kSwap);
        const M128    moved = _mm_move_ss(swap, _mm_set_ss(_value));
        return _mm_shuffle_ps(moved, moved, kSwap);
    }
#        endif
}

inline void JUG_VECTORCALL Transpose4(
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
    const M128 t0 = _mm_unpacklo_ps(_row0, _row1);
    const M128 t1 = _mm_unpacklo_ps(_row2, _row3);
    const M128 t2 = _mm_unpackhi_ps(_row0, _row1);
    const M128 t3 = _mm_unpackhi_ps(_row2, _row3);
    _row0         = _mm_movelh_ps(t0, t1);
    _row1         = _mm_movehl_ps(t1, t0);
    _row2         = _mm_movelh_ps(t2, t3);
    _row3         = _mm_movehl_ps(t3, t2);
#        endif
}

// =======================================================
//  Vector
// =======================================================

// 네 요소의 합을 모든 요소에
[[nodiscard]] inline M128 JUG_VECTORCALL SumAll(
    const M128 _a)
{
#        if defined(JUG_MATH_SIMD_NEON)
#            if defined(__aarch64__) || defined(_M_ARM64)
    return vdupq_n_f32(vaddvq_f32(_a));
#            else
    float32x2_t sum2 = vadd_f32(vget_low_f32(_a), vget_high_f32(_a));
    sum2             = vpadd_f32(sum2, sum2);
    return vdupq_lane_f32(sum2, 0);
#            endif
#        elif defined(JUG_MATH_SIMD_SSE)
    const M128 shuf = Shuffle<1, 0, 3, 2>(_a);   // y x w z
    const M128 sums = _mm_add_ps(_a, shuf);       // x+y, x+y, z+w, z+w
    return _mm_add_ps(sums, Shuffle<2, 3, 0, 1>(sums));
#        endif
}

// dpps 는 쓰지 않는다 - Zen4 에서 셔플 + 덧셈보다 두 배 이상 느리다 (DirectXMath 는 SSE4 에서 dpps 를 쓴다)
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
#        if defined(JUG_MATH_SIMD_SSE)
    // x*x' + y*y' + z*z' 를 스칼라 슬롯에서 더하고 퍼뜨린다
    const M128 m  = _mm_mul_ps(_a, _b);
    const M128 yz = Shuffle<1, 2, 1, 2>(m);
    M128       s  = _mm_add_ss(m, yz);
    s             = _mm_add_ss(s, Shuffle<1, 1, 1, 1>(yz));
    return Shuffle<0, 0, 0, 0>(s);
#        else
    return SumAll(ZeroW(Mult(_a, _b)));
#        endif
}

[[nodiscard]] inline float JUG_VECTORCALL Dot4(
    const M128 _a,
    const M128 _b)
{
    return GetX(Dot4V(_a, _b));
}

[[nodiscard]] inline float JUG_VECTORCALL Dot3(
    const M128 _a,
    const M128 _b)
{
    return GetX(Dot3V(_a, _b));
}

// w 는 보장하지 않는다 (FMA 면 aw*bw 의 반올림 오차가 남는다) - xyz 만 쓸 것
[[nodiscard]] inline M128 JUG_VECTORCALL Cross3(
    const M128 _a,
    const M128 _b)
{
    // a.yzx * b.zxy - a.zxy * b.yzx 를 셔플 3번으로: (a * b.yzx - a.yzx * b) 를 yzx 로 돌린다
    const M128 aYzx = Shuffle<1, 2, 0, 3>(_a);
    const M128 bYzx = Shuffle<1, 2, 0, 3>(_b);
    const M128 c    = MultSub(_a, bYzx, Mult(aYzx, _b));
    return Shuffle<1, 2, 0, 3>(c);
}

[[nodiscard]] inline float JUG_VECTORCALL LengthSq3(
    const M128 _a)
{
    return Dot3(_a, _a);
}

[[nodiscard]] inline float JUG_VECTORCALL LengthSq4(
    const M128 _a)
{
    return Dot4(_a, _a);
}

[[nodiscard]] inline float JUG_VECTORCALL Length3(
    const M128 _a)
{
    return GetX(Sqrt(Dot3V(_a, _a)));
}

[[nodiscard]] inline float JUG_VECTORCALL Length4(
    const M128 _a)
{
    return GetX(Sqrt(Dot4V(_a, _a)));
}

// 길이 0 이면 0 (DirectXMath 와 같은 의미). w 도 같은 길이로 나뉜다.
// rsqrt + 뉴턴 1회로 나눗셈·제곱근 장치를 쓰지 않는다 (상대오차 ~3e-7, 정확한 경로의 두세 배 처리량).
// 길이² 가 정규 float 보다 작으면 (|v| < 1e-19, 0 포함) rsqrt 가 무한대라 정확한 경로로 간다 - 거의 안 타는 분기.
[[nodiscard]] inline M128 JUG_VECTORCALL Normalize3(
    const M128 _a)
{
    const M128 lenSq = Dot3V(_a, _a);
    if (GetX(lenSq) < 1.17549435e-38f)
    {
        return And(Div(_a, Sqrt(lenSq)), CmpNe(lenSq, Zero()));
    }
    return Mult(_a, RSqrt(lenSq));
}

[[nodiscard]] inline M128 JUG_VECTORCALL Normalize4(
    const M128 _a)
{
    const M128 lenSq = Dot4V(_a, _a);
    if (GetX(lenSq) < 1.17549435e-38f)
    {
        return And(Div(_a, Sqrt(lenSq)), CmpNe(lenSq, Zero()));
    }
    return Mult(_a, RSqrt(lenSq));
}

// =======================================================
//  Interpolation
// =======================================================

[[nodiscard]] inline M128 JUG_VECTORCALL Lerp(
    const M128 _a,
    const M128 _b,
    const M128 _t)
{
    return MultAdd(Sub(_b, _a), _t, _a);
}

[[nodiscard]] inline M128 JUG_VECTORCALL Lerp(
    const M128  _a,
    const M128  _b,
    const float _t)
{
    return Lerp(_a, _b, SetAll(_t));
}

// =======================================================
//  Trigonometry
// =======================================================

// sin, cos 를 한 번에. DirectXMath XMVectorSinCos 와 같은 식:
// [-pi, pi] 로 줄이고, |x| > pi/2 면 x = sign(x)*pi - x 로 접어 cos 부호를 뒤집은 뒤
// sin 은 11차, cos 는 10차 minimax 다항식.
inline void JUG_VECTORCALL SinCos(
    const M128 _x,
    M128&      _outSin,
    M128&      _outCos)
{
    // x - 2pi * round(x / 2pi)
    const M128 quotient = Round(Mult(_x, SetAll(0.159154943f)));
    M128       x        = NegMultAdd(quotient, SetAll(6.283185307f), _x);

    const M128 sign    = And(x, SetAll(-0.f));
    const M128 c       = Or(SetAll(3.141592654f), sign);   // sign(x) * pi
    const M128 absx    = AndNot(SetAll(-0.f), x);
    const M128 rflx    = Sub(c, x);
    const M128 inRange = CmpLe(absx, SetAll(1.570796327f));
    x                  = Select(inRange, x, rflx);
    const M128 cosSign = Select(inRange, SetAll(1.f), SetAll(-1.f));

    const M128 x2 = Mult(x, x);

    // sin
    M128 s = MultAdd(SetAll(-2.3889859e-08f), x2, SetAll(2.7525562e-06f));
    s      = MultAdd(s, x2, SetAll(-0.00019840874f));
    s      = MultAdd(s, x2, SetAll(0.0083333310f));
    s      = MultAdd(s, x2, SetAll(-0.16666667f));
    s      = MultAdd(s, x2, SetAll(1.f));
    _outSin = Mult(s, x);

    // cos
    M128 k = MultAdd(SetAll(-2.6051615e-07f), x2, SetAll(2.4760495e-05f));
    k      = MultAdd(k, x2, SetAll(-0.0013888378f));
    k      = MultAdd(k, x2, SetAll(0.041666638f));
    k      = MultAdd(k, x2, SetAll(-0.5f));
    k      = MultAdd(k, x2, SetAll(1.f));
    _outCos = Mult(k, cosSign);
}


// sin 만 (DirectXMath XMVectorSin). 구간 축소는 SinCos 와 같고 cos 다항식을 뺀 것
[[nodiscard]] inline M128 JUG_VECTORCALL Sin(
    const M128 _x)
{
    const M128 quotient = Round(Mult(_x, SetAll(0.159154943f)));
    M128       x        = NegMultAdd(quotient, SetAll(6.283185307f), _x);

    const M128 sign    = And(x, SetAll(-0.f));
    const M128 c       = Or(SetAll(3.141592654f), sign);
    const M128 absx    = AndNot(SetAll(-0.f), x);
    const M128 rflx    = Sub(c, x);
    const M128 inRange = CmpLe(absx, SetAll(1.570796327f));
    x                  = Select(inRange, x, rflx);

    const M128 x2 = Mult(x, x);
    M128       s  = MultAdd(SetAll(-2.3889859e-08f), x2, SetAll(2.7525562e-06f));
    s             = MultAdd(s, x2, SetAll(-0.00019840874f));
    s             = MultAdd(s, x2, SetAll(0.0083333310f));
    s             = MultAdd(s, x2, SetAll(-0.16666667f));
    s             = MultAdd(s, x2, SetAll(1.f));
    return Mult(s, x);
}

// cos 만 (DirectXMath XMVectorCos)
[[nodiscard]] inline M128 JUG_VECTORCALL Cos(
    const M128 _x)
{
    const M128 quotient = Round(Mult(_x, SetAll(0.159154943f)));
    M128       x        = NegMultAdd(quotient, SetAll(6.283185307f), _x);

    const M128 sign    = And(x, SetAll(-0.f));
    const M128 c       = Or(SetAll(3.141592654f), sign);
    const M128 absx    = AndNot(SetAll(-0.f), x);
    const M128 rflx    = Sub(c, x);
    const M128 inRange = CmpLe(absx, SetAll(1.570796327f));
    x                  = Select(inRange, x, rflx);
    const M128 cosSign = Select(inRange, SetAll(1.f), SetAll(-1.f));

    const M128 x2 = Mult(x, x);
    M128       k  = MultAdd(SetAll(-2.6051615e-07f), x2, SetAll(2.4760495e-05f));
    k             = MultAdd(k, x2, SetAll(-0.0013888378f));
    k             = MultAdd(k, x2, SetAll(0.041666638f));
    k             = MultAdd(k, x2, SetAll(-0.5f));
    k             = MultAdd(k, x2, SetAll(1.f));
    return Mult(k, cosSign);
}

// 스칼라 각 하나의 (sin, cos, sin, cos). sin 과 cos 다항식을 한 레지스터의 서로 다른 레인에서 같이 돌려서
// SinCos(SetAll(x)) 보다 다항식 연산이 절반이다. 식은 SinCos 와 같다.
[[nodiscard]] inline M128 JUG_VECTORCALL SinCos1(
    const float _x)
{
    const M128 x        = SetAll(_x);
    const M128 quotient = Round(Mult(x, SetAll(0.159154943f)));
    M128       y        = NegMultAdd(quotient, SetAll(6.283185307f), x);

    const M128 sign    = And(y, SetAll(-0.f));
    const M128 c       = Or(SetAll(3.141592654f), sign);
    const M128 absy    = AndNot(SetAll(-0.f), y);
    const M128 rfly    = Sub(c, y);
    const M128 inRange = CmpLe(absy, SetAll(1.570796327f));
    y                  = Select(inRange, y, rfly);
    const M128 cosSign = Select(inRange, SetAll(1.f), SetAll(-1.f));

    // 짝수 레인 sin 계수, 홀수 레인 cos 계수
    const M128 y2 = Mult(y, y);
    M128       p  = MultAdd(Set(-2.3889859e-08f, -2.6051615e-07f, -2.3889859e-08f, -2.6051615e-07f), y2, Set(2.7525562e-06f, 2.4760495e-05f, 2.7525562e-06f, 2.4760495e-05f));
    p             = MultAdd(p, y2, Set(-0.00019840874f, -0.0013888378f, -0.00019840874f, -0.0013888378f));
    p             = MultAdd(p, y2, Set(0.0083333310f, 0.041666638f, 0.0083333310f, 0.041666638f));
    p             = MultAdd(p, y2, Set(-0.16666667f, -0.5f, -0.16666667f, -0.5f));
    p             = MultAdd(p, y2, SetAll(1.f));
    return Mult(p, Blend<0b1010>(y, cosSign));   // sin 레인 * y, cos 레인 * 부호
}

}   // namespace jug::simd

#    endif   // JUG_SIMD_AVAILABLE
#endif       // !defined(__CUDA_ARCH__)

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
