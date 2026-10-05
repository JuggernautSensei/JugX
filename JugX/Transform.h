#pragma once
#include "Matrix.h"
#include "Quaternion.h"
#include "Vector3.h"

namespace jug
{

struct TRANSFORM
{
    [[nodiscard]] static TRANSFORM MakeFromMatrix(
        const MAT4& _mat)
    {
        TRANSFORM t;
        Decompose(_mat, &t.scale, &t.rotation, &t.translation);
        return t;
    }

    [[nodiscard]] JUG_MATH_API constexpr MAT4 ToMatrix() const
    {
        return MAT4::MakeSRT(scale, rotation, translation);
    }

    [[nodiscard]] JUG_MATH_API constexpr MAT4 ToInvMatrix() const
    {
        return MAT4::MakeInvSRT(scale, rotation, translation);
    }

    [[nodiscard]] JUG_MATH_API constexpr static TRANSFORM Identity()
    {
        return TRANSFORM { VEC3::Zero(), QUAT::Identity(), VEC3::One() };
    }

    VEC3 translation;
    QUAT rotation;
    VEC3 scale;
};

static_assert(PodT<TRANSFORM>, "TRANSFORM must be POD type.");

[[nodiscard]] JUG_MATH_API constexpr TRANSFORM Lerp(
    const TRANSFORM& _a,
    const TRANSFORM& _b,
    const float      _t)
{
    return TRANSFORM {
        Lerp(_a.translation, _b.translation, _t),
        Nlerp(_a.rotation, _b.rotation, _t),
        Lerp(_a.scale, _b.scale, _t),
    };
}

}   // namespace jug
