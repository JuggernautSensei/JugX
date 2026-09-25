#pragma once
#include "Vector.h"

namespace jug
{

struct QUATERNION;

// 1. 4x4 행렬
// 2. 행 우선
// 3. 행 벡터
// 4. 왼손 좌표계

struct alignas(16) MATRIX
{
    using ValueT = float;

    JUG_MATH_API MATRIX() = default;

    JUG_MATH_API constexpr MATRIX(
        const VECTOR4 _row0,
        const VECTOR4 _row1,
        const VECTOR4 _row2,
        const VECTOR4 _row3)
        : r { _row0, _row1, _row2, _row3 }
    {
    }

#ifdef JUG_SIMD_AVAILABLE
    [[nodiscard]] static MATRIX MakeFromM128(
        const simd::M128 _row0,
        const simd::M128 _row1,
        const simd::M128 _row2,
        const simd::M128 _row3)
    {
        MATRIX ret;
        simd::StoreAligned2(ret.r[0].GetPtr(), _row0, _row1);
        simd::StoreAligned2(ret.r[2].GetPtr(), _row2, _row3);
        return ret;
    }
#endif

    // clang-format off
    JUG_MATH_API constexpr MATRIX(
        const float _m00, const float _m01, const float _m02, const float _m03,
        const float _m10, const float _m11, const float _m12, const float _m13,
        const float _m20, const float _m21, const float _m22, const float _m23,
        const float _m30, const float _m31, const float _m32, const float _m33)
        : r {
           VECTOR4 { _m00, _m01, _m02, _m03 },
           VECTOR4 { _m10, _m11, _m12, _m13 },
           VECTOR4 { _m20, _m21, _m22, _m23 },
           VECTOR4 { _m30, _m31, _m32, _m33 }
        }
    {
    }
    // clang-format on

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeTranslation(
        const VECTOR3 _translation)
    {
        return MATRIX {
            {             1.f,             0.f,             0.f, 0.f },
            {             0.f,             1.f,             0.f, 0.f },
            {             0.f,             0.f,             1.f, 0.f },
            { _translation[0], _translation[1], _translation[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeScale(
        const VECTOR3 _scale)
    {
        return MATRIX {
            { _scale[0],       0.f,       0.f, 0.f },
            {       0.f, _scale[1],       0.f, 0.f },
            {       0.f,       0.f, _scale[2], 0.f },
            {       0.f,       0.f,       0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationX(
        const float _rad)
    {
        const float s = Sin(_rad);
        const float c = Cos(_rad);
        return MATRIX {
            { 1.f, 0.f, 0.f, 0.f },
            { 0.f,   c,   s, 0.f },
            { 0.f,  -s,   c, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationY(
        const float _rad)
    {
        const float s = Sin(_rad);
        const float c = Cos(_rad);
        return MATRIX {
            {   c, 0.f,  -s, 0.f },
            { 0.f, 1.f, 0.f, 0.f },
            {   s, 0.f,   c, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationZ(
        const float _rad)
    {
        const float s = Sin(_rad);
        const float c = Cos(_rad);
        return MATRIX {
            {   c,   s, 0.f, 0.f },
            {  -s,   c, 0.f, 0.f },
            { 0.f, 0.f, 1.f, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const VECTOR3 _pyrRad)
    {
        const float cx = Cos(_pyrRad[0]), sx = Sin(_pyrRad[0]);
        const float cy = Cos(_pyrRad[1]), sy = Sin(_pyrRad[1]);
        const float cz = Cos(_pyrRad[2]), sz = Sin(_pyrRad[2]);

        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return MATRIX {
            { r00, r01, r02, 0.f },
            { r10, r11, r12, 0.f },
            { r20, r21, r22, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const VECTOR3 _axis,
        const float   _rad)
    {
        JUG_ASSERT(IsNormalized(_axis), "Rotation axis must be normalized");

        const float x = _axis[0];
        const float y = _axis[1];
        const float z = _axis[2];
        const float s = Sin(_rad);
        const float c = Cos(_rad);
        const float t = 1.f - c;

        return MATRIX {
            {     t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0.f },
            { t * x * y - s * z,     t * y * y + c, t * y * z + s * x, 0.f },
            { t * x * z + s * y, t * y * z - s * x,     t * z * z + c, 0.f },
            {               0.f,               0.f,               0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        QUATERNION _q);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotationLookTo(
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
        JUG_ASSERT(IsNormalized(_dir), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_dir, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f = _dir;
        const VECTOR3 r = Normalize(Cross(_up, f));
        const VECTOR3 u = Cross(f, r);
        return MATRIX {
            { r[0], r[1], r[2], 0.f },
            { u[0], u[1], u[2], 0.f },
            { f[0], f[1], f[2], 0.f },
            {  0.f,  0.f,  0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f = _forward;
        const VECTOR3 r = Normalize(Cross(_up, f));
        const VECTOR3 u = Cross(f, r);

        return MATRIX {
            { r[0] * _scale[0], r[1] * _scale[0], r[2] * _scale[0], 0.f },
            { u[0] * _scale[1], u[1] * _scale[1], u[2] * _scale[1], 0.f },
            { f[0] * _scale[2], f[1] * _scale[2], f[2] * _scale[2], 0.f },
            {  _translation[0],  _translation[1],  _translation[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale.x) && !IsZeroApprox(_scale.y) && !IsZeroApprox(_scale.z), "Scale cannot have a zero component.");

        const float cx = Cos(_pyrRad[0]), sx = Sin(_pyrRad[0]);
        const float cy = Cos(_pyrRad[1]), sy = Sin(_pyrRad[1]);
        const float cz = Cos(_pyrRad[2]), sz = Sin(_pyrRad[2]);
        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return {
            { _scale[0] * r00, _scale[0] * r01, _scale[0] * r02, 0.f },
            { _scale[1] * r10, _scale[1] * r11, _scale[1] * r12, 0.f },
            { _scale[2] * r20, _scale[2] * r21, _scale[2] * r22, 0.f },
            { _translation[0], _translation[1], _translation[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        VECTOR3    _scale,
        QUATERNION _rotation,
        VECTOR3    _translation);

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _forward,
        const VECTOR3 _up,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(IsNormalized(_forward), "Forward dir must be normalized");
        JUG_ASSERT(IsNormalized(_up), "Up dir must be normalized");
        JUG_ASSERT(!IsParallel(_forward, _up), "Forward and up dirs cannot be parallel");

        const VECTOR3 f    = _forward;
        const VECTOR3 r    = Normalize(Cross(_up, f));
        const VECTOR3 u    = Cross(f, r);
        const VECTOR3 invs = RcpSafe(_scale);
        const float   tx = _translation[0], ty = _translation[1], tz = _translation[2];

        return MATRIX {
            {                                 r[0] * invs[0],                                 u[0] * invs[1],                                 f[0] * invs[2], 0.f },
            {                                 r[1] * invs[0],                                 u[1] * invs[1],                                 f[1] * invs[2], 0.f },
            {                                 r[2] * invs[0],                                 u[2] * invs[1],                                 f[2] * invs[2], 0.f },
            { -(tx * r[0] + ty * r[1] + tz * r[2]) * invs[0], -(tx * u[0] + ty * u[1] + tz * u[2]) * invs[1], -(tx * f[0] + ty * f[1] + tz * f[2]) * invs[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale.x) && !IsZeroApprox(_scale.y) && !IsZeroApprox(_scale.z), "Scale cannot have a zero component.");

        const float cx = Cos(_pyrRad[0]), sx = Sin(_pyrRad[0]);
        const float cy = Cos(_pyrRad[1]), sy = Sin(_pyrRad[1]);
        const float cz = Cos(_pyrRad[2]), sz = Sin(_pyrRad[2]);

        const float isx = 1.f / _scale[0], isy = 1.f / _scale[1], isz = 1.f / _scale[2];
        const float tx = _translation[0], ty = _translation[1], tz = _translation[2];

        const float r00 = cy * cz + sx * sy * sz, r01 = cx * sz, r02 = sx * cy * sz - cz * sy;
        const float r10 = sx * sy * cz - cy * sz, r11 = cx * cz, r12 = sy * sz + sx * cy * cz;
        const float r20 = cx * sy, r21 = -sx, r22 = cx * cy;

        return {
            {                               r00 * isx,                               r10 * isy,                               r20 * isz, 0.f },
            {                               r01 * isx,                               r11 * isy,                               r21 * isz, 0.f },
            {                               r02 * isx,                               r12 * isy,                               r22 * isz, 0.f },
            { -(tx * r00 + ty * r01 + tz * r02) * isx, -(tx * r10 + ty * r11 + tz * r12) * isy, -(tx * r20 + ty * r21 + tz * r22) * isz, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX MakeInvSRT(
        VECTOR3    _scale,
        QUATERNION _rotation,
        VECTOR3    _translation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _dir,
        const VECTOR3 _up)
    {
        return MakeInvSRT(One<VECTOR3>(), _dir, _up, _eye);
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        const VECTOR3 _eye,
        const VECTOR3 _pyrRad)
    {
        return MakeInvSRT(One<VECTOR3>(), _pyrRad, _eye);
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeViewLookTo(
        VECTOR3    _eye,
        QUATERNION _rotation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakePersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const float ys    = 1.f / Tan(_fovYRad * 0.5f);
        const float xs    = ys / _aspectRatio;
        const float range = _farZ / (_farZ - _nearZ);

        return MATRIX {
            {  xs, 0.f,             0.f, 0.f },
            { 0.f,  ys,             0.f, 0.f },
            { 0.f, 0.f,           range, 1.f },
            { 0.f, 0.f, -range * _nearZ, 0.f }
        };
    }

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvPersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        const float d    = _farZ - _nearZ;
        const float invF = Tan(_fovYRad * 0.5f);

        return MATRIX {
            { _aspectRatio * invF,  0.f, 0.f,                   0.f },
            {                 0.f, invF, 0.f,                   0.f },
            {                 0.f,  0.f, 0.f, -d / (_farZ * _nearZ) },
            {                 0.f,  0.f, 1.f,          1.f / _nearZ }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeOrtho(
        const float _left,
        const float _right,
        const float _bottom,
        const float _top,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsEqualApprox(_left, _right), "Invalid view volume width");
        JUG_ASSERT(!IsEqualApprox(_bottom, _top), "Invalid view volume height");
        JUG_ASSERT(_farZ > _nearZ, "Invalid near/far plane");

        const float invW  = 1.f / (_right - _left);
        const float invH  = 1.f / (_top - _bottom);
        const float range = 1.f / (_farZ - _nearZ);

        return MATRIX {
            {               2.f * invW,                      0.f,             0.f, 0.f },
            {                      0.f,               2.f * invH,             0.f, 0.f },
            {                      0.f,                      0.f,           range, 0.f },
            { -(_left + _right) * invW, -(_top + _bottom) * invH, -range * _nearZ, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    // 빠른 역행렬
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvOrtho(
        const float _left,
        const float _right,
        const float _bottom,
        const float _top,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsEqualApprox(_left, _right), "Invalid view volume width");
        JUG_ASSERT(!IsEqualApprox(_bottom, _top), "Invalid view volume height");
        JUG_ASSERT(_farZ > _nearZ, "Invalid near/far plane");

        const float w = _right - _left;
        const float h = _top - _bottom;
        const float d = _farZ - _nearZ;

        return MATRIX {
            {                w * 0.5f,                     0.f,    0.f, 0.f },
            {                     0.f,                h * 0.5f,    0.f, 0.f },
            {                     0.f,                     0.f,      d, 0.f },
            { (_right + _left) * 0.5f, (_top + _bottom) * 0.5f, _nearZ, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeInvOrtho(
        const float _width,
        const float _height,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(!IsZeroApprox(_width) && !IsZeroApprox(_height), "Invalid view volume size");
        return MakeInvOrtho(-_width * 0.5f, _width * 0.5f, -_height * 0.5f, _height * 0.5f, _nearZ, _farZ);
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator+(
        const MATRIX& _other) const
    {
        return MATRIX { r[0] + _other[0], r[1] + _other[1], r[2] + _other[2], r[3] + _other[3] };
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator-(
        const MATRIX& _other) const
    {
        return MATRIX { r[0] - _other[0], r[1] - _other[1], r[2] - _other[2], r[3] - _other[3] };
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
        const float _scalar) const
    {
        return MATRIX { r[0] * _scalar, r[1] * _scalar, r[2] * _scalar, r[3] * _scalar };
    }

    [[nodiscard]] JUG_MATH_API constexpr MATRIX operator/(
        const float _scalar) const
    {
        return *this * (1.f / _scalar);
    }

    [[nodiscard]] JUG_MATH_API JUG_FORCE_INLINE constexpr MATRIX operator*(
        const MATRIX& _other) const
    {
#ifdef JUG_SIMD_AVAILABLE
        if (!std::is_constant_evaluated())
        {
            const simd::M128 b0 = _other[0].ToM128();
            const simd::M128 b1 = _other[1].ToM128();
            const simd::M128 b2 = _other[2].ToM128();
            const simd::M128 b3 = _other[3].ToM128();

            const simd::M128 a0   = r[0].ToM128();
            simd::M128       row0 = simd::Mult(simd::Splat<0>(a0), b0);
            row0                  = simd::MultAdd(simd::Splat<1>(a0), b1, row0);
            row0                  = simd::MultAdd(simd::Splat<2>(a0), b2, row0);
            row0                  = simd::MultAdd(simd::Splat<3>(a0), b3, row0);

            const simd::M128 a1   = r[1].ToM128();
            simd::M128       row1 = simd::Mult(simd::Splat<0>(a1), b0);
            row1                  = simd::MultAdd(simd::Splat<1>(a1), b1, row1);
            row1                  = simd::MultAdd(simd::Splat<2>(a1), b2, row1);
            row1                  = simd::MultAdd(simd::Splat<3>(a1), b3, row1);

            const simd::M128 a2   = r[2].ToM128();
            simd::M128       row2 = simd::Mult(simd::Splat<0>(a2), b0);
            row2                  = simd::MultAdd(simd::Splat<1>(a2), b1, row2);
            row2                  = simd::MultAdd(simd::Splat<2>(a2), b2, row2);
            row2                  = simd::MultAdd(simd::Splat<3>(a2), b3, row2);

            const simd::M128 a3   = r[3].ToM128();
            simd::M128       row3 = simd::Mult(simd::Splat<0>(a3), b0);
            row3                  = simd::MultAdd(simd::Splat<1>(a3), b1, row3);
            row3                  = simd::MultAdd(simd::Splat<2>(a3), b2, row3);
            row3                  = simd::MultAdd(simd::Splat<3>(a3), b3, row3);

            return MakeFromM128(row0, row1, row2, row3);
        }
#endif
        const float b00 = _other[0][0], b01 = _other[0][1], b02 = _other[0][2], b03 = _other[0][3];
        const float b10 = _other[1][0], b11 = _other[1][1], b12 = _other[1][2], b13 = _other[1][3];
        const float b20 = _other[2][0], b21 = _other[2][1], b22 = _other[2][2], b23 = _other[2][3];
        const float b30 = _other[3][0], b31 = _other[3][1], b32 = _other[3][2], b33 = _other[3][3];

        // clang-format off
        return MATRIX {
            (r[0][0] * b00 + r[0][1] * b10) + (r[0][2] * b20 + r[0][3] * b30),
            (r[0][0] * b01 + r[0][1] * b11) + (r[0][2] * b21 + r[0][3] * b31),
            (r[0][0] * b02 + r[0][1] * b12) + (r[0][2] * b22 + r[0][3] * b32),
            (r[0][0] * b03 + r[0][1] * b13) + (r[0][2] * b23 + r[0][3] * b33),

            (r[1][0] * b00 + r[1][1] * b10) + (r[1][2] * b20 + r[1][3] * b30),
            (r[1][0] * b01 + r[1][1] * b11) + (r[1][2] * b21 + r[1][3] * b31),
            (r[1][0] * b02 + r[1][1] * b12) + (r[1][2] * b22 + r[1][3] * b32),
            (r[1][0] * b03 + r[1][1] * b13) + (r[1][2] * b23 + r[1][3] * b33),

            (r[2][0] * b00 + r[2][1] * b10) + (r[2][2] * b20 + r[2][3] * b30),
            (r[2][0] * b01 + r[2][1] * b11) + (r[2][2] * b21 + r[2][3] * b31),
            (r[2][0] * b02 + r[2][1] * b12) + (r[2][2] * b22 + r[2][3] * b32),
            (r[2][0] * b03 + r[2][1] * b13) + (r[2][2] * b23 + r[2][3] * b33),

            (r[3][0] * b00 + r[3][1] * b10) + (r[3][2] * b20 + r[3][3] * b30),
            (r[3][0] * b01 + r[3][1] * b11) + (r[3][2] * b21 + r[3][3] * b31),
            (r[3][0] * b02 + r[3][1] * b12) + (r[3][2] * b22 + r[3][3] * b32),
            (r[3][0] * b03 + r[3][1] * b13) + (r[3][2] * b23 + r[3][3] * b33)
        };
    }

    JUG_MATH_API constexpr MATRIX& operator+=(
        const MATRIX& _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator-=(
        const MATRIX& _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator*=(
        const MATRIX& _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr MATRIX& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const MATRIX& _other) const
    {
        for (size_t i = 0; i < kRow; ++i)
        {
            if (r[i] != _other[i])
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator!=(
        const MATRIX& _other) const
    {
        return !(*this == _other);
    }

    [[nodiscard]] JUG_MATH_API constexpr float& operator()(
        const size_t _row,
        const size_t _col)
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr const float& operator()(
        const size_t _row,
        const size_t _col) const
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4& operator[](
        const size_t _row)
    {
        return r[_row];
    }

    [[nodiscard]] JUG_MATH_API constexpr const VECTOR4& operator[](
        const size_t _row) const
    {
        return r[_row];
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return r[0].GetPtr();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return r[0].GetPtr();
    }

    const static MATRIX kIdentity;
    const static MATRIX kZero;
    
    constexpr static size_t kRow = 4;
    constexpr static size_t kCol = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };

        struct
        {
            VECTOR4 r0;
            VECTOR4 r1;
            VECTOR4 r2;
            VECTOR4 r3;
        };

        ARRAY<VECTOR4, 4>              r;
        ARRAY<float, 16>               e;
        ARRAY<ARRAY<float, 4>, 4> m;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(MATRIX) == 64, "Matrix4x4 must be tightly packed");
static_assert(alignof(MATRIX) == 16, "Matrix4x4 must be 16-byte aligned for SIMD");
static_assert(PodT<MATRIX>, "Matrix4x4 must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr MATRIX MATRIX::kIdentity = {
    { 1.f, 0.f, 0.f, 0.f },
    { 0.f, 1.f, 0.f, 0.f },
    { 0.f, 0.f, 1.f, 0.f },
    { 0.f, 0.f, 0.f, 1.f }
};

inline constexpr MATRIX MATRIX::kZero = {
    { 0.f, 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f, 0.f }
};

template<>
struct MathConstants<MATRIX>
{
    constexpr static MATRIX kIdentity = MATRIX::kIdentity;
    constexpr static MATRIX kZero     = MATRIX::kZero;
};

// =======================================================
//  Method
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
    const float   _scalar,
    const MATRIX& _mtx)
{
    return _mtx * _scalar;
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator/(
    const float   _scalar,
    const MATRIX& _mtx)
{
    return MATRIX { _scalar / _mtx[0], _scalar / _mtx[1], _scalar / _mtx[2], _scalar / _mtx[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX Abs(
    const MATRIX& _mtx)
{
    return MATRIX { Abs(_mtx[0]), Abs(_mtx[1]), Abs(_mtx[2]), Abs(_mtx[3]) };
}

[[nodiscard]] JUG_MATH_API constexpr bool IsEqualApprox(
    const MATRIX& _x,
    const MATRIX& _y,
    const float   _epsilon = kEpsilon)
{
    for (size_t i = 0; i < MATRIX::kRow; ++i)
    {
        if (!IsEqualApprox(_x[i], _y[i], _epsilon))
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX Transpose(
    const MATRIX& _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        simd::M128 r0 = _mtx[0].ToM128();
        simd::M128 r1 = _mtx[1].ToM128();
        simd::M128 r2 = _mtx[2].ToM128();
        simd::M128 r3 = _mtx[3].ToM128();
        simd::Transpose4(r0, r1, r2, r3);
        return MATRIX::MakeFromM128(r0, r1, r2, r3);
    }
#endif
    return MATRIX {
        { _mtx[0][0], _mtx[1][0], _mtx[2][0], _mtx[3][0] },
        { _mtx[0][1], _mtx[1][1], _mtx[2][1], _mtx[3][1] },
        { _mtx[0][2], _mtx[1][2], _mtx[2][2], _mtx[3][2] },
        { _mtx[0][3], _mtx[1][3], _mtx[2][3], _mtx[3][3] }
    };
}

namespace matrix_detail
{
    struct MATRIX_COFACTORS
    {
        float s0, s1, s2, s3, s4, s5;
        float c0, c1, c2, c3, c4, c5;
    };

    [[nodiscard]] JUG_MATH_API constexpr MATRIX_COFACTORS MakeMatrixCofactors(
        const MATRIX& _mtx)
    {
        const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
        const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
        const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
        const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

        MATRIX_COFACTORS cof;
        cof.s0 = m00 * m11 - m10 * m01;
        cof.s1 = m00 * m12 - m10 * m02;
        cof.s2 = m00 * m13 - m10 * m03;
        cof.s3 = m01 * m12 - m11 * m02;
        cof.s4 = m01 * m13 - m11 * m03;
        cof.s5 = m02 * m13 - m12 * m03;
        cof.c5 = m22 * m33 - m32 * m23;
        cof.c4 = m21 * m33 - m31 * m23;
        cof.c3 = m21 * m32 - m31 * m22;
        cof.c2 = m20 * m33 - m30 * m23;
        cof.c1 = m20 * m32 - m30 * m22;
        cof.c0 = m20 * m31 - m30 * m21;
        return cof;
    }
}   // namespace collision_detail

[[nodiscard]] JUG_MATH_API constexpr float Determinant(
    const MATRIX& _mtx)
{
    const matrix_detail::MATRIX_COFACTORS cof = matrix_detail::MakeMatrixCofactors(_mtx);
    return cof.s0 * cof.c5 - cof.s1 * cof.c4 + cof.s2 * cof.c3 + cof.s3 * cof.c2 - cof.s4 * cof.c1 + cof.s5 * cof.c0;
}

[[nodiscard]] JUG_MATH_API JUG_FORCE_INLINE constexpr MATRIX Inverse(
    const MATRIX& _mtx,
    float*        _outDetOrNull = nullptr)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 row0 = _mtx[0].ToM128();
        const simd::M128 row1 = _mtx[1].ToM128();
        const simd::M128 row2 = _mtx[2].ToM128();
        const simd::M128 row3 = _mtx[3].ToM128();

        const simd::M128 t1 = simd::Shuffle2<0, 1, 0, 1>(row0, row1);
        const simd::M128 t3 = simd::Shuffle2<2, 3, 2, 3>(row0, row1);
        const simd::M128 t2 = simd::Shuffle2<0, 1, 0, 1>(row2, row3);
        const simd::M128 t4 = simd::Shuffle2<2, 3, 2, 3>(row2, row3);

        const simd::M128 mt0 = simd::Shuffle2<0, 2, 0, 2>(t1, t2);
        const simd::M128 mt1 = simd::Shuffle2<1, 3, 1, 3>(t1, t2);
        const simd::M128 mt2 = simd::Shuffle2<0, 2, 0, 2>(t3, t4);
        const simd::M128 mt3 = simd::Shuffle2<1, 3, 1, 3>(t3, t4);

        simd::M128 v00 = simd::Shuffle<0, 0, 1, 1>(mt2);
        simd::M128 v10 = simd::Shuffle<2, 3, 2, 3>(mt3);
        simd::M128 v01 = simd::Shuffle<0, 0, 1, 1>(mt0);
        simd::M128 v11 = simd::Shuffle<2, 3, 2, 3>(mt1);
        simd::M128 v02 = simd::Shuffle2<0, 2, 0, 2>(mt2, mt0);
        simd::M128 v12 = simd::Shuffle2<1, 3, 1, 3>(mt3, mt1);

        simd::M128 d0 = simd::Mult(v00, v10);
        simd::M128 d1 = simd::Mult(v01, v11);
        simd::M128 d2 = simd::Mult(v02, v12);

        v00 = simd::Shuffle<2, 3, 2, 3>(mt2);
        v10 = simd::Shuffle<0, 0, 1, 1>(mt3);
        v01 = simd::Shuffle<2, 3, 2, 3>(mt0);
        v11 = simd::Shuffle<0, 0, 1, 1>(mt1);
        v02 = simd::Shuffle2<1, 3, 1, 3>(mt2, mt0);
        v12 = simd::Shuffle2<0, 2, 0, 2>(mt3, mt1);

        d0 = simd::Sub(d0, simd::Mult(v00, v10));
        d1 = simd::Sub(d1, simd::Mult(v01, v11));
        d2 = simd::Sub(d2, simd::Mult(v02, v12));

        // c0, c2, c4, c6
        simd::M128 v11b = simd::Shuffle2<1, 3, 1, 1>(d0, d2);
        v00             = simd::Shuffle<1, 2, 0, 1>(mt1);
        simd::M128 v10b = simd::Shuffle2<2, 0, 3, 0>(v11b, d0);
        v01             = simd::Shuffle<2, 0, 1, 0>(mt0);
        v11b            = simd::Shuffle2<1, 2, 1, 2>(v11b, d0);
        simd::M128 v13a = simd::Shuffle2<1, 3, 3, 3>(d1, d2);
        v02             = simd::Shuffle<1, 2, 0, 1>(mt3);
        simd::M128 v12b = simd::Shuffle2<2, 0, 3, 0>(v13a, d1);
        simd::M128 v03a = simd::Shuffle<2, 0, 1, 0>(mt2);
        v13a            = simd::Shuffle2<1, 2, 1, 2>(v13a, d1);

        simd::M128 c0 = simd::Mult(v00, v10b);
        simd::M128 c2 = simd::Mult(v01, v11b);
        simd::M128 c4 = simd::Mult(v02, v12b);
        simd::M128 c6 = simd::Mult(v03a, v13a);

        v11b = simd::Shuffle2<0, 1, 0, 0>(d0, d2);
        v00  = simd::Shuffle<2, 3, 1, 2>(mt1);
        v10b = simd::Shuffle2<3, 0, 1, 2>(d0, v11b);
        v01  = simd::Shuffle<3, 2, 3, 1>(mt0);
        v11b = simd::Shuffle2<2, 1, 2, 0>(d0, v11b);
        v13a = simd::Shuffle2<0, 1, 2, 2>(d1, d2);
        v02  = simd::Shuffle<2, 3, 1, 2>(mt3);
        v12b = simd::Shuffle2<3, 0, 1, 2>(d1, v13a);
        v03a = simd::Shuffle<3, 2, 3, 1>(mt2);
        v13a = simd::Shuffle2<2, 1, 2, 0>(d1, v13a);

        c0 = simd::Sub(c0, simd::Mult(v00, v10b));
        c2 = simd::Sub(c2, simd::Mult(v01, v11b));
        c4 = simd::Sub(c4, simd::Mult(v02, v12b));
        c6 = simd::Sub(c6, simd::Mult(v03a, v13a));

        // c1, c3, c5, c7
        v00             = simd::Shuffle<3, 0, 3, 0>(mt1);
        simd::M128 v10c = simd::Shuffle2<2, 2, 0, 1>(d0, d2);
        v10c            = simd::Shuffle<0, 3, 2, 0>(v10c);
        v01             = simd::Shuffle<1, 3, 0, 2>(mt0);
        simd::M128 v11c = simd::Shuffle2<0, 3, 0, 1>(d0, d2);
        v11c            = simd::Shuffle<3, 0, 1, 2>(v11c);
        v02             = simd::Shuffle<3, 0, 3, 0>(mt3);
        simd::M128 v12c = simd::Shuffle2<2, 2, 2, 3>(d1, d2);
        v12c            = simd::Shuffle<0, 3, 2, 0>(v12c);
        v03a            = simd::Shuffle<1, 3, 0, 2>(mt2);
        simd::M128 v13c = simd::Shuffle2<0, 3, 2, 3>(d1, d2);
        v13c            = simd::Shuffle<3, 0, 1, 2>(v13c);

        v00  = simd::Mult(v00, v10c);
        v01  = simd::Mult(v01, v11c);
        v02  = simd::Mult(v02, v12c);
        v03a = simd::Mult(v03a, v13c);

        const simd::M128 c1 = simd::Sub(c0, v00);
        c0                  = simd::Add(c0, v00);
        const simd::M128 c3 = simd::Add(c2, v01);
        c2                  = simd::Sub(c2, v01);
        const simd::M128 c5 = simd::Sub(c4, v02);
        c4                  = simd::Add(c4, v02);
        const simd::M128 c7 = simd::Add(c6, v03a);
        c6                  = simd::Sub(c6, v03a);

        c0 = simd::Shuffle2<0, 2, 1, 3>(c0, c1);
        c2 = simd::Shuffle2<0, 2, 1, 3>(c2, c3);
        c4 = simd::Shuffle2<0, 2, 1, 3>(c4, c5);
        c6 = simd::Shuffle2<0, 2, 1, 3>(c6, c7);
        c0 = simd::Shuffle<0, 2, 1, 3>(c0);
        c2 = simd::Shuffle<0, 2, 1, 3>(c2);
        c4 = simd::Shuffle<0, 2, 1, 3>(c4);
        c6 = simd::Shuffle<0, 2, 1, 3>(c6);

        const simd::M128 detV = simd::Dot4V(c0, mt0);
        const float      det  = simd::GetX(detV);
        if (_outDetOrNull)
        {
            *_outDetOrNull = det;
        }

        if (IsZeroApprox(det))
        {
            return MathConstants<MATRIX>::kZero;
        }
        const simd::M128 invDetV = simd::Div(simd::SetAll(1.f), detV);
        return MATRIX::MakeFromM128(simd::Mult(c0, invDetV), simd::Mult(c2, invDetV), simd::Mult(c4, invDetV), simd::Mult(c6, invDetV));
    }
#endif
    const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
    const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
    const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
    const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

    const matrix_detail::MATRIX_COFACTORS k = matrix_detail::MakeMatrixCofactors(_mtx);

    const float det = k.s0 * k.c5 - k.s1 * k.c4 + k.s2 * k.c3 + k.s3 * k.c2 - k.s4 * k.c1 + k.s5 * k.c0;
    if (_outDetOrNull)
    {
        *_outDetOrNull = det;
    }

    // det == 0: 역행렬이 존재하지 않음.
    if (IsZeroApprox(det))
    {
        return MathConstants<MATRIX>::kZero;
    }
    const float invDet = 1.f / det;

    return MATRIX {
        ( m11 * k.c5 - m12 * k.c4 + m13 * k.c3) * invDet,
        (-m01 * k.c5 + m02 * k.c4 - m03 * k.c3) * invDet,
        ( m31 * k.s5 - m32 * k.s4 + m33 * k.s3) * invDet,
        (-m21 * k.s5 + m22 * k.s4 - m23 * k.s3) * invDet,

        (-m10 * k.c5 + m12 * k.c2 - m13 * k.c1) * invDet,
        ( m00 * k.c5 - m02 * k.c2 + m03 * k.c1) * invDet,
        (-m30 * k.s5 + m32 * k.s2 - m33 * k.s1) * invDet,
        ( m20 * k.s5 - m22 * k.s2 + m23 * k.s1) * invDet,

        ( m10 * k.c4 - m11 * k.c2 + m13 * k.c0) * invDet,
        (-m00 * k.c4 + m01 * k.c2 - m03 * k.c0) * invDet,
        ( m30 * k.s4 - m31 * k.s2 + m33 * k.s0) * invDet,
        (-m20 * k.s4 + m21 * k.s2 - m23 * k.s0) * invDet,

        (-m10 * k.c3 + m11 * k.c1 - m12 * k.c0) * invDet,
        ( m00 * k.c3 - m01 * k.c1 + m02 * k.c0) * invDet,
        (-m30 * k.s3 + m31 * k.s1 - m32 * k.s0) * invDet,
        ( m20 * k.s3 - m21 * k.s1 + m22 * k.s0) * invDet
    };
}

JUG_MATH_API constexpr void Decompose(
    const MATRIX& _mtx,
    VECTOR3*      _pOutScaleOrNull,
    QUATERNION*   _pOutRotationOrNull,
    VECTOR3*      _pOutTranslationOrNull);

[[nodiscard]] JUG_MATH_API constexpr VECTOR4 Xform(
    const VECTOR4 _v,
    const MATRIX& _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 v   = _v.ToM128();
        simd::M128       ret = simd::Mult(simd::Splat<0>(v), _mtx[0].ToM128());
        ret                  = simd::MultAdd(simd::Splat<1>(v), _mtx[1].ToM128(), ret);
        ret                  = simd::MultAdd(simd::Splat<2>(v), _mtx[2].ToM128(), ret);
        ret                  = simd::MultAdd(simd::Splat<3>(v), _mtx[3].ToM128(), ret);
        return VECTOR4::MakeFromM128(ret);
    }
#endif
    return {
        _v[0] * _mtx[0][0] + _v[1] * _mtx[1][0] + _v[2] * _mtx[2][0] + _v[3] * _mtx[3][0],
        _v[0] * _mtx[0][1] + _v[1] * _mtx[1][1] + _v[2] * _mtx[2][1] + _v[3] * _mtx[3][1],
        _v[0] * _mtx[0][2] + _v[1] * _mtx[1][2] + _v[2] * _mtx[2][2] + _v[3] * _mtx[3][2],
        _v[0] * _mtx[0][3] + _v[1] * _mtx[1][3] + _v[2] * _mtx[2][3] + _v[3] * _mtx[3][3]
    };
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformPoint(
    const VECTOR3 _point,
    const MATRIX& _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 p   = _point.ToM128();
        simd::M128       ret = simd::MultAdd(simd::Splat<0>(p), _mtx[0].ToM128(), _mtx[3].ToM128());
        ret                  = simd::MultAdd(simd::Splat<1>(p), _mtx[1].ToM128(), ret);
        ret                  = simd::MultAdd(simd::Splat<2>(p), _mtx[2].ToM128(), ret);
        return VECTOR3::MakeFromM128(simd::Div(ret, simd::Splat<3>(ret)));
    }
#endif
    const VECTOR4 p    = Xform({ _point, 1.f }, _mtx);
    const float   invW = 1.f / p[3];
    return p.ToVector3() * invW;
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformVector(
    const VECTOR3 _v,
    const MATRIX& _mtx)
{
#ifdef JUG_SIMD_AVAILABLE
    if (!std::is_constant_evaluated())
    {
        const simd::M128 v   = _v.ToM128();
        simd::M128       ret = simd::Mult(simd::Splat<0>(v), _mtx[0].ToM128());
        ret                  = simd::MultAdd(simd::Splat<1>(v), _mtx[1].ToM128(), ret);
        ret                  = simd::MultAdd(simd::Splat<2>(v), _mtx[2].ToM128(), ret);
        return VECTOR3::MakeFromM128(ret);
    }
#endif
    return Xform({_v, 0.f}, _mtx).ToVector3();
}

}   // namespace jug

#include "Matrix.inl"
