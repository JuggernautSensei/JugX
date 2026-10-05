#pragma once

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
        const VEC4 _rgba)
    {
        RGBA rgba;
        rgba.r = static_cast<uint8_t>(Clamp(_rgba[0] * 255.f, 0.f, 255.f));
        rgba.g = static_cast<uint8_t>(Clamp(_rgba[1] * 255.f, 0.f, 255.f));
        rgba.b = static_cast<uint8_t>(Clamp(_rgba[2] * 255.f, 0.f, 255.f));
        rgba.a = static_cast<uint8_t>(Clamp(_rgba[3] * 255.f, 0.f, 255.f));
        return rgba;
    }

    // ======================================================
    //  Utils
    // ======================================================

    [[nodiscard]] JUG_MATH_API VEC4 ToLinear() const
    {
        return {
            static_cast<float>(r) / 255.f,
            static_cast<float>(g) / 255.f,
            static_cast<float>(b) / 255.f,
            static_cast<float>(a) / 255.f
        };
    }

    [[nodiscard]] JUG_MATH_API bool operator==(
        const RGBA _rgba) const
    {
        return rgba == _rgba.rgba;
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Zero()
    {
        return RGBA { 0, 0, 0, 0 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Black()
    {
        return RGBA { 0, 0, 0, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA White()
    {
        return RGBA { 255, 255, 255, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Red()
    {
        return RGBA { 255, 0, 0, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Green()
    {
        return RGBA { 0, 255, 0, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Blue()
    {
        return RGBA { 0, 0, 255, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Yellow()
    {
        return RGBA { 255, 255, 0, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Cyan()
    {
        return RGBA { 0, 255, 255, 255 };
    }

    [[nodiscard]] JUG_MATH_API static constexpr RGBA Magenta()
    {
        return RGBA { 255, 0, 255, 255 };
    }

    // ======================================================
    //  Fields
    // ======================================================

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

}   // namespace jug
