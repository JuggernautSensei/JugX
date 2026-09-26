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

    // =======================================================
    //  Factory
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeTranslation(
        const VECTOR3 _translation)
    {
        return MATRIX {
            VECTOR4 { 1.f, 0.f, 0.f, 0.f },
            VECTOR4 { 0.f, 1.f, 0.f, 0.f },
            VECTOR4 { 0.f, 0.f, 1.f, 0.f },
            VECTOR4 { _translation, 1.f }
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
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
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
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
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
        float s = 0.f;
        float c = 0.f;
        SinCos(_rad, s, c);
        return MATRIX {
            {   c,   s, 0.f, 0.f },
            {  -s,   c, 0.f, 0.f },
            { 0.f, 0.f, 1.f, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
    }

    // pitch(x) -> yaw(y) -> roll(z)
    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const VECTOR3 _pyrRad)
    {
        float sx = 0.f, cx = 0.f;
        float sy = 0.f, cy = 0.f;
        float sz = 0.f, cz = 0.f;
        SinCos(_pyrRad[0], sx, cx);
        SinCos(_pyrRad[1], sy, cy);
        SinCos(_pyrRad[2], sz, cz);

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
        float       s = 0.f;
        float       c = 0.f;
        SinCos(_rad, s, c);
        const float t = 1.f - c;

        return MATRIX {
            {     t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0.f },
            { t * x * y - s * z,     t * y * y + c, t * y * z + s * x, 0.f },
            { t * x * z + s * y, t * y * z - s * x,     t * z * z + c, 0.f },
            {               0.f,               0.f,               0.f, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeRotation(
        const QUATERNION& _q);

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
            VECTOR4 { r, 0.f },
            VECTOR4 { u, 0.f },
            VECTOR4 { f, 0.f },
            VECTOR4 { 0.f, 0.f, 0.f, 1.f }
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
            VECTOR4 { r * _scale[0], 0.f },
            VECTOR4 { u * _scale[1], 0.f },
            VECTOR4 { f * _scale[2], 0.f },
            VECTOR4 { _translation, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

        const MATRIX rot = MakeRotation(_pyrRad);
        return MATRIX {
            rot[0] * _scale[0],
            rot[1] * _scale[1],
            rot[2] * _scale[2],
            VECTOR4 { _translation, 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakeSRT(
        const VECTOR3     _scale,
        const QUATERNION& _rotation,
        const VECTOR3     _translation);

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
        const VECTOR3 t    = _translation;

        return MATRIX {
            {         r[0] * invs[0],         u[0] * invs[1],         f[0] * invs[2], 0.f },
            {         r[1] * invs[0],         u[1] * invs[1],         f[1] * invs[2], 0.f },
            {         r[2] * invs[0],         u[2] * invs[1],         f[2] * invs[2], 0.f },
            { -Dot(t, r) * invs[0], -Dot(t, u) * invs[1], -Dot(t, f) * invs[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX MakeInvSRT(
        const VECTOR3 _scale,
        const VECTOR3 _pyrRad,
        const VECTOR3 _translation)
    {
        JUG_ASSERT(!IsZeroApprox(_scale[0]) && !IsZeroApprox(_scale[1]) && !IsZeroApprox(_scale[2]), "Scale cannot have a zero component.");

        const MATRIX  rot  = MakeRotation(_pyrRad);
        const VECTOR3 r0   = rot[0].ToVector3();
        const VECTOR3 r1   = rot[1].ToVector3();
        const VECTOR3 r2   = rot[2].ToVector3();
        const VECTOR3 invs = Rcp(_scale);
        const VECTOR3 t    = _translation;

        return MATRIX {
            {         r0[0] * invs[0],         r1[0] * invs[1],         r2[0] * invs[2], 0.f },
            {         r0[1] * invs[0],         r1[1] * invs[1],         r2[1] * invs[2], 0.f },
            {         r0[2] * invs[0],         r1[2] * invs[1],         r2[2] * invs[2], 0.f },
            { -Dot(t, r0) * invs[0], -Dot(t, r1) * invs[1], -Dot(t, r2) * invs[2], 1.f }
        };
    }

    [[nodiscard]] JUG_MATH_API static constexpr MATRIX MakeInvSRT(
        const VECTOR3     _scale,
        const QUATERNION& _rotation,
        const VECTOR3     _translation);

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
        const VECTOR3     _eye,
        const QUATERNION& _rotation);

    [[nodiscard]] JUG_MATH_API constexpr static MATRIX MakePersp(
        const float _fovYRad,
        const float _aspectRatio,
        const float _nearZ,
        const float _farZ)
    {
        JUG_ASSERT(_nearZ > 0.f && _farZ > _nearZ, "Invalid near/far plane");
        JUG_ASSERT(!IsZeroApprox(_aspectRatio), "Aspect ratio must not be zero");

        float s = 0.f;
        float c = 0.f;
        SinCos(_fovYRad * 0.5f, s, c);
        const float ys    = c / s;
        const float xs    = ys / _aspectRatio;
        const float range = _farZ / (_farZ - _nearZ);

        return MATRIX {
            {  xs, 0.f,             0.f, 0.f },
            { 0.f,  ys,             0.f, 0.f },
            { 0.f, 0.f,           range, 1.f },
            { 0.f, 0.f, -range * _nearZ, 0.f }
        };
    }

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

    // =======================================================
    //  Access
    // =======================================================

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

    [[nodiscard]] JUG_MATH_API constexpr float& operator()(
        const size_t _row,
        const size_t _col)
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr float operator()(
        const size_t _row,
        const size_t _col) const
    {
        return r[_row][_col];
    }

    [[nodiscard]] JUG_MATH_API constexpr float* GetPtr()
    {
        return r[0].GetPtr();
    }

    [[nodiscard]] JUG_MATH_API constexpr const float* GetPtr() const
    {
        return r[0].GetPtr();
    }

    JUG_MATH_API constexpr MATRIX& operator+=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator-=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator*=(const MATRIX& _other);
    JUG_MATH_API constexpr MATRIX& operator*=(float _scalar);
    JUG_MATH_API constexpr MATRIX& operator/=(float _scalar);

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

        ARRAY<VECTOR4, 4>         r;
        ARRAY<float, 16>          e;
        ARRAY<ARRAY<float, 4>, 4> m;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(sizeof(MATRIX) == 64, "MATRIX must be tightly packed");
static_assert(alignof(MATRIX) == 16, "MATRIX must be 16-byte aligned");
static_assert(PodT<MATRIX>, "MATRIX must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr MATRIX MATRIX::kIdentity = {
    VECTOR4 { 1.f, 0.f, 0.f, 0.f },
    VECTOR4 { 0.f, 1.f, 0.f, 0.f },
    VECTOR4 { 0.f, 0.f, 1.f, 0.f },
    VECTOR4 { 0.f, 0.f, 0.f, 1.f }
};

inline constexpr MATRIX MATRIX::kZero = {
    VECTOR4 { 0.f },
    VECTOR4 { 0.f },
    VECTOR4 { 0.f },
    VECTOR4 { 0.f }
};

template<>
struct MathConstants<MATRIX>
{
    constexpr static MATRIX kIdentity = MATRIX::kIdentity;
    constexpr static MATRIX kZero     = MATRIX::kZero;
};

// =======================================================
//  Operators
// =======================================================

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator+(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return MATRIX { _x[0] + _y[0], _x[1] + _y[1], _x[2] + _y[2], _x[3] + _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator-(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return MATRIX { _x[0] - _y[0], _x[1] - _y[1], _x[2] - _y[2], _x[3] - _y[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
    const MATRIX& _mtx,
    const float   _scalar)
{
    return MATRIX { _mtx[0] * _scalar, _mtx[1] * _scalar, _mtx[2] * _scalar, _mtx[3] * _scalar };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
    const float   _scalar,
    const MATRIX& _mtx)
{
    return _mtx * _scalar;
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator/(
    const MATRIX& _mtx,
    const float   _scalar)
{
    return _mtx * (1.f / _scalar);
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator/(
    const float   _scalar,
    const MATRIX& _mtx)
{
    return MATRIX { _scalar / _mtx[0], _scalar / _mtx[1], _scalar / _mtx[2], _scalar / _mtx[3] };
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX operator*(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return MATRIX {
        (_y[0] * _x[0][0] + _y[1] * _x[0][1]) + (_y[2] * _x[0][2] + _y[3] * _x[0][3]),
        (_y[0] * _x[1][0] + _y[1] * _x[1][1]) + (_y[2] * _x[1][2] + _y[3] * _x[1][3]),
        (_y[0] * _x[2][0] + _y[1] * _x[2][1]) + (_y[2] * _x[2][2] + _y[3] * _x[2][3]),
        (_y[0] * _x[3][0] + _y[1] * _x[3][1]) + (_y[2] * _x[3][2] + _y[3] * _x[3][3])
    };
}

[[nodiscard]] JUG_MATH_API constexpr bool operator==(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return _x[0] == _y[0] && _x[1] == _y[1] && _x[2] == _y[2] && _x[3] == _y[3];
}

[[nodiscard]] JUG_MATH_API constexpr bool operator!=(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return !(_x == _y);
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator+=(
    const MATRIX& _other)
{
    *this = *this + _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator-=(
    const MATRIX& _other)
{
    *this = *this - _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator*=(
    const MATRIX& _other)
{
    *this = *this * _other;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator*=(
    const float _scalar)
{
    *this = *this * _scalar;
    return *this;
}

JUG_MATH_API constexpr MATRIX& MATRIX::operator/=(
    const float _scalar)
{
    *this = *this / _scalar;
    return *this;
}

// =======================================================
//  Method
// =======================================================

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
    return IsEqualApprox(_x[0], _y[0], _epsilon) && IsEqualApprox(_x[1], _y[1], _epsilon) && IsEqualApprox(_x[2], _y[2], _epsilon) && IsEqualApprox(_x[3], _y[3], _epsilon);
}

[[nodiscard]] JUG_MATH_API constexpr MATRIX Transpose(
    const MATRIX& _mtx)
{
    return MATRIX {
        { _mtx[0][0], _mtx[1][0], _mtx[2][0], _mtx[3][0] },
        { _mtx[0][1], _mtx[1][1], _mtx[2][1], _mtx[3][1] },
        { _mtx[0][2], _mtx[1][2], _mtx[2][2], _mtx[3][2] },
        { _mtx[0][3], _mtx[1][3], _mtx[2][3], _mtx[3][3] }
    };
}

[[nodiscard]] JUG_MATH_API constexpr float Determinant(
    const MATRIX& _mtx)
{
    const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
    const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
    const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
    const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

    const float s0 = m00 * m11 - m10 * m01;
    const float s1 = m00 * m12 - m10 * m02;
    const float s2 = m00 * m13 - m10 * m03;
    const float s3 = m01 * m12 - m11 * m02;
    const float s4 = m01 * m13 - m11 * m03;
    const float s5 = m02 * m13 - m12 * m03;
    const float c5 = m22 * m33 - m32 * m23;
    const float c4 = m21 * m33 - m31 * m23;
    const float c3 = m21 * m32 - m31 * m22;
    const float c2 = m20 * m33 - m30 * m23;
    const float c1 = m20 * m32 - m30 * m22;
    const float c0 = m20 * m31 - m30 * m21;
    return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

// 역행렬이 없으면 (행렬식이 정확히 0) 영행렬
[[nodiscard]] JUG_MATH_API constexpr MATRIX Inverse(
    const MATRIX& _mtx,
    float*        _outDetOrNull = nullptr)
{
    const float m00 = _mtx[0][0], m01 = _mtx[0][1], m02 = _mtx[0][2], m03 = _mtx[0][3];
    const float m10 = _mtx[1][0], m11 = _mtx[1][1], m12 = _mtx[1][2], m13 = _mtx[1][3];
    const float m20 = _mtx[2][0], m21 = _mtx[2][1], m22 = _mtx[2][2], m23 = _mtx[2][3];
    const float m30 = _mtx[3][0], m31 = _mtx[3][1], m32 = _mtx[3][2], m33 = _mtx[3][3];

    const float s0 = m00 * m11 - m10 * m01;
    const float s1 = m00 * m12 - m10 * m02;
    const float s2 = m00 * m13 - m10 * m03;
    const float s3 = m01 * m12 - m11 * m02;
    const float s4 = m01 * m13 - m11 * m03;
    const float s5 = m02 * m13 - m12 * m03;
    const float c5 = m22 * m33 - m32 * m23;
    const float c4 = m21 * m33 - m31 * m23;
    const float c3 = m21 * m32 - m31 * m22;
    const float c2 = m20 * m33 - m30 * m23;
    const float c1 = m20 * m32 - m30 * m22;
    const float c0 = m20 * m31 - m30 * m21;

    const float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    if (_outDetOrNull)
    {
        *_outDetOrNull = det;
    }
    if (det == 0.f)
    {
        return MathConstants<MATRIX>::kZero;
    }
    const float invDet = 1.f / det;

    // clang-format off
    return MATRIX {
        ( m11 * c5 - m12 * c4 + m13 * c3) * invDet,
        (-m01 * c5 + m02 * c4 - m03 * c3) * invDet,
        ( m31 * s5 - m32 * s4 + m33 * s3) * invDet,
        (-m21 * s5 + m22 * s4 - m23 * s3) * invDet,

        (-m10 * c5 + m12 * c2 - m13 * c1) * invDet,
        ( m00 * c5 - m02 * c2 + m03 * c1) * invDet,
        (-m30 * s5 + m32 * s2 - m33 * s1) * invDet,
        ( m20 * s5 - m22 * s2 + m23 * s1) * invDet,

        ( m10 * c4 - m11 * c2 + m13 * c0) * invDet,
        (-m00 * c4 + m01 * c2 - m03 * c0) * invDet,
        ( m30 * s4 - m31 * s2 + m33 * s0) * invDet,
        (-m20 * s4 + m21 * s2 - m23 * s0) * invDet,

        (-m10 * c3 + m11 * c1 - m12 * c0) * invDet,
        ( m00 * c3 - m01 * c1 + m02 * c0) * invDet,
        (-m30 * s3 + m31 * s1 - m32 * s0) * invDet,
        ( m20 * s3 - m21 * s1 + m22 * s0) * invDet
    };
    // clang-format on
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
    return (_mtx[0] * _v[0] + _mtx[1] * _v[1]) + (_mtx[2] * _v[2] + _mtx[3] * _v[3]);
}

// (x, y, z, 1) 을 변환하고 w 로 나눈다
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformPoint(
    const VECTOR3 _point,
    const MATRIX& _mtx)
{
    const VECTOR4 p = Xform(VECTOR4 { _point, 1.f }, _mtx);
    return p.ToVector3() / p[3];
}

// (x, y, z, 0) 을 변환한다 (이동 무시)
[[nodiscard]] JUG_MATH_API constexpr VECTOR3 XformVector(
    const VECTOR3 _v,
    const MATRIX& _mtx)
{
    return Xform(VECTOR4 { _v, 0.f }, _mtx).ToVector3();
}

}   // namespace jug

#include "Matrix.inl"
