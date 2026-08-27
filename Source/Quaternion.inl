#pragma once
#include "Matrix.h"   // NOLINT

namespace jug
{

[[nodiscard]] JUG_MATH_API constexpr QUATERNION QUATERNION::MakeFromMatrix(
    const MATRIX& _mtx)
{
    const float m00 = _mtx.r[0].e[0], m01 = _mtx.r[0].e[1], m02 = _mtx.r[0].e[2];
    const float m10 = _mtx.r[1].e[0], m11 = _mtx.r[1].e[1], m12 = _mtx.r[1].e[2];
    const float m20 = _mtx.r[2].e[0], m21 = _mtx.r[2].e[1], m22 = _mtx.r[2].e[2];

    const float trace = m00 + m11 + m22;

    if (trace > 0.f)
    {
        const float s    = Sqrt(trace + 1.f) * 2.f;   // s = 4w
        const float invS = 1.f / s;
        return QUATERNION { (m12 - m21) * invS, (m20 - m02) * invS, (m01 - m10) * invS, s * 0.25f };
    }

    if (m00 > m11 && m00 > m22)
    {
        const float s    = Sqrt(1.f + m00 - m11 - m22) * 2.f;   // s = 4x
        const float invS = 1.f / s;
        return QUATERNION { s * 0.25f, (m01 + m10) * invS, (m02 + m20) * invS, (m12 - m21) * invS };
    }

    if (m11 > m22)
    {
        const float s    = Sqrt(1.f + m11 - m00 - m22) * 2.f;   // s = 4y
        const float invS = 1.f / s;
        return QUATERNION { (m01 + m10) * invS, s * 0.25f, (m12 + m21) * invS, (m20 - m02) * invS };
    }

    const float s    = Sqrt(1.f + m22 - m00 - m11) * 2.f;   // s = 4z
    const float invS = 1.f / s;
    return QUATERNION { (m02 + m20) * invS, (m12 + m21) * invS, s * 0.25f, (m01 - m10) * invS };
}

}   // namespace jug
