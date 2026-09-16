// ReSharper disable CppClangTidyClangDiagnosticImplicitIntConversion
#pragma once

#include "Math.h"

namespace jug
{

enum class Float16 : uint16_t
{
};

[[nodiscard]] JUG_MATH_API constexpr Float16 MakeF16FromF32(
    const float _f32)
{
    union
    {
        float    fv;
        uint32_t ui;
    } ci;
    ci.fv = _f32;

    const uint32_t x        = ci.ui;
    const uint32_t sign     = x >> 31;
    const uint32_t exponent = (x >> 23) & 0xFF;
    const uint32_t mantissa = x & 0x7FFFFF;

    uint16_t hf = 0;

    if (exponent == 0xFF)
    {
        hf = (static_cast<uint16_t>(sign) << 15)
           | 0x7C00
           | (mantissa >> 13);
    }

    else if (exponent > 0x8E)
    {
        hf = (static_cast<uint16_t>(sign) << 15)
           | 0x7C00;
    }

    else if (exponent > 0x70)
    {
        hf = (static_cast<uint16_t>(sign) << 15)
           | (static_cast<uint16_t>((exponent - 112) << 10))
           | (mantissa >> 13);
    }

    else if (exponent > 0x66)
    {
        uint32_t man = (0x800000 | mantissa);

        uint32_t shift = 125 - exponent;
        man >>= shift;

        uint32_t rounding_bits = (0x800000 | mantissa) & ((1u << shift) - 1);
        if (rounding_bits > (1u << (shift - 1)))
        {
            man++;
        }
        else if (rounding_bits == (1u << (shift - 1)))
        {
            man = (man + 1) & ~1u;
        }

        hf = (static_cast<uint16_t>(sign) << 15) | static_cast<uint16_t>(man);
    }

    else
    {
        hf = static_cast<uint16_t>(sign << 15);
    }

    return static_cast<Float16>(hf);
}

[[nodiscard]] JUG_MATH_API constexpr float MakeF32FromF16(
    const Float16 _f16)
{
    union
    {
        float  fv;
        uint32_t ui;
    } co;

    const uint16_t hf       = static_cast<uint16_t>(_f16);
    const uint16_t sign     = (hf >> 15) & 0x1;
    const uint16_t exponent = (hf >> 10) & 0x1F;
    const uint16_t mantissa = hf & 0x3FF;

    if (exponent == 0x1F)
    {
        co.ui = (sign << 31) | 0x7F800000 | (mantissa << 13);
    }
    else if (exponent != 0)
    {
        co.ui = (sign << 31) | ((exponent + 112) << 23) | (mantissa << 13);
    }
    else
    {
        co.ui = (sign << 31) | (mantissa << 13);
    }

    return co.fv;
}

}   // namespace jug
