#pragma once
#include <cstdint>

#include "Math.h"
#include "Platform.h"
#include "TypeTraits.h"
#include "Vector4.h"

namespace jug
{

struct RGBA
{
    JUG_MATH_API RGBA() = default;

    JUG_MATH_API constexpr RGBA(
        const uint8_t _r,
        const uint8_t _g,
        const uint8_t _b,
        const uint8_t _a = 255)
        : a(_a)
        , b(_b)
        , g(_g)
        , r(_r)
    {
    }

    // ======================================================
    //  Factory
    // ======================================================

    [[nodiscard]] JUG_MATH_API static constexpr RGBA MakeFromLinear(
        const VECTOR4 _rgba)
    {
        RGBA rgba;
        rgba.r = static_cast<uint8_t>(jug::Clamp(_rgba.e[0], 0.f, 1.f) * 255.f);
        rgba.g = static_cast<uint8_t>(jug::Clamp(_rgba.e[1], 0.f, 1.f) * 255.f);
        rgba.b = static_cast<uint8_t>(jug::Clamp(_rgba.e[2], 0.f, 1.f) * 255.f);
        rgba.a = static_cast<uint8_t>(jug::Clamp(_rgba.e[3], 0.f, 1.f) * 255.f);
        return rgba;
    }

    // ======================================================
    //  Utils
    // ======================================================

    [[nodiscard]] JUG_MATH_API VECTOR4 ToLinear() const
    {
        return {
            static_cast<float>(r) / 255.f,
            static_cast<float>(g) / 255.f,
            static_cast<float>(b) / 255.f,
            static_cast<float>(a) / 255.f
        };
    }
    // ======================================================
    //  Fields
    // ======================================================

    const static RGBA kZero;
    const static RGBA kBlack;
    const static RGBA kWhite;
    const static RGBA kRed;
    const static RGBA kGreen;
    const static RGBA kBlue;
    const static RGBA kYellow;
    const static RGBA kCyan;
    const static RGBA kMagenta;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
#ifdef JUG_LITTLE_ENDIAN
            uint8_t a;
            uint8_t b;
            uint8_t g;
            uint8_t r;
#else
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
#endif
        };
        uint64_t rgba;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(PodT<RGBA>, "RGBA must be POD type.");

// ======================================================
//  Constants
// =====================================================

inline constexpr RGBA RGBA::kZero    = RGBA { 0, 0, 0, 0 };
inline constexpr RGBA RGBA::kBlack   = RGBA { 0, 0, 0, 255 };
inline constexpr RGBA RGBA::kWhite   = RGBA { 255, 255, 255, 255 };
inline constexpr RGBA RGBA::kRed     = RGBA { 255, 0, 0, 255 };
inline constexpr RGBA RGBA::kGreen   = RGBA { 0, 255, 0, 255 };
inline constexpr RGBA RGBA::kBlue    = RGBA { 0, 0, 255, 255 };
inline constexpr RGBA RGBA::kYellow  = RGBA { 255, 255, 0, 255 };
inline constexpr RGBA RGBA::kCyan    = RGBA { 0, 255, 255, 255 };
inline constexpr RGBA RGBA::kMagenta = RGBA { 255, 0, 255, 255 };

template<>
struct MathConstants<RGBA>
{
    static constexpr RGBA Zero = RGBA::kZero;
};

}   // namespace jug