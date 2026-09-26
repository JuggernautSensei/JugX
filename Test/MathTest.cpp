// jug 수학 라이브러리 검증 (Google Test)
//
// 1. 저장 타입 (VECTOR / QUATERNION / MATRIX) 은 constexpr 로 평가되는지
// 2. 연산 타입 (*_REG, SIMD) 이 저장 타입 (순수 C++) 과 같은 값을 내는지
// 3. DirectXMath 를 기준값으로 한 비교 (Windows)
// 4. 의미 규칙: 영벡터 정규화, 특이 행렬 역행렬, 역변환 팩토리, 왕복 변환
#include <cmath>
#include <cstring>
#include <random>

#include <gtest/gtest.h>

#include <JugX/JugX.h>

#if __has_include(<DirectXMath.h>)
#    include <DirectXCollision.h>
#    include <DirectXMath.h>
#    define JUG_TEST_WITH_DXM 1
#endif

using namespace jug;

namespace
{

constexpr float kTol = 2e-5f;

[[nodiscard]] float RelDiff(
    const float _x,
    const float _y)
{
    return std::abs(_x - _y) / std::max(1.f, std::abs(_y));
}

[[nodiscard]] float MaxRelDiff(
    const float* _x,
    const float* _y,
    const size_t _count)
{
    float ret = 0.f;
    for (size_t i = 0; i < _count; ++i)
    {
        ret = std::max(ret, RelDiff(_x[i], _y[i]));
    }
    return ret;
}

[[nodiscard]] float MaxRelDiff(
    const MATRIX& _x,
    const MATRIX& _y)
{
    return MaxRelDiff(_x.GetPtr(), _y.GetPtr(), 16);
}

[[nodiscard]] float MaxRelDiff(
    const VECTOR4 _x,
    const VECTOR4 _y)
{
    return MaxRelDiff(_x.GetPtr(), _y.GetPtr(), 4);
}

[[nodiscard]] float MaxRelDiff(
    const VECTOR3 _x,
    const VECTOR3 _y)
{
    return MaxRelDiff(_x.GetPtr(), _y.GetPtr(), 3);
}

[[nodiscard]] float MaxRelDiff(
    const MATRIX_REG _x,
    const MATRIX&    _y)
{
    return MaxRelDiff(Store(_x), _y);
}

[[nodiscard]] float MaxRelDiff(
    const VECTOR3_REG _x,
    const VECTOR3     _y)
{
    return MaxRelDiff(Store(_x), _y);
}

[[nodiscard]] float MaxRelDiff(
    const VECTOR4_REG _x,
    const VECTOR4     _y)
{
    return MaxRelDiff(Store(_x), _y);
}

// q 와 -q 는 같은 회전
[[nodiscard]] float RotationDiff(
    const QUATERNION _x,
    const QUATERNION _y)
{
    return std::min(MaxRelDiff(_x.v, _y.v), MaxRelDiff(_x.v, -_y.v));
}

[[nodiscard]] float RotationDiff(
    const QUATERNION_REG _x,
    const QUATERNION     _y)
{
    return RotationDiff(Store(_x), _y);
}

struct Rng
{
    explicit Rng(
        const uint32_t _seed)
        : engine(_seed)
    {
    }

    [[nodiscard]] float Float(
        const float _min,
        const float _max)
    {
        return std::uniform_real_distribution<float>(_min, _max)(engine);
    }

    [[nodiscard]] VECTOR3 Vec3(
        const float _min,
        const float _max)
    {
        return VECTOR3 { Float(_min, _max), Float(_min, _max), Float(_min, _max) };
    }

    [[nodiscard]] VECTOR4 Vec4(
        const float _min,
        const float _max)
    {
        return VECTOR4 { Float(_min, _max), Float(_min, _max), Float(_min, _max), Float(_min, _max) };
    }

    [[nodiscard]] VECTOR3 Dir()
    {
        return Normalize(VECTOR3 { Float(-1.f, 1.f), Float(-1.f, 1.f), Float(0.2f, 1.f) });
    }

    [[nodiscard]] QUATERNION Quat()
    {
        return Normalize(QUATERNION { Vec4(-1.f, 1.f) });
    }

    // 역행렬이 안정적인 일반 행렬 (대각 우세)
    [[nodiscard]] MATRIX Mtx()
    {
        MATRIX m;
        for (size_t r = 0; r < 4; ++r)
        {
            m[r]     = Vec4(-1.f, 1.f);
            m[r][r] += 3.f;
        }
        return m;
    }

    std::mt19937 engine;
};

constexpr int kCount = 256;

}   // namespace

// ==========================================================
//  constexpr (저장 타입)
// ==========================================================

namespace constexpr_check
{

constexpr VECTOR4 kA { 1.f, 2.f, 3.f, 4.f };
constexpr VECTOR4 kB { -2.f, 0.5f, 4.f, -1.f };
static_assert((kA + kB)[0] == -1.f && (kA - kB)[3] == 5.f && (kA * 2.f)[2] == 6.f);
static_assert(Dot(kA, kB) == 7.f);
static_assert(Normalize(MathConstants<VECTOR4>::kZero) == MathConstants<VECTOR4>::kZero);

constexpr QUATERNION kQ    = QUATERNION::MakeFromAxisAngle(VECTOR3 { 0.f, 1.f, 0.f }, 0.5f);
constexpr MATRIX     kSrt  = MATRIX::MakeSRT(VECTOR3 { 1.f, 2.f, 3.f }, kQ, VECTOR3 { 4.f, 5.f, 6.f });
constexpr MATRIX     kInv  = Inverse(kSrt);
constexpr MATRIX     kBack = kSrt * kInv;
static_assert(IsEqualApprox(kBack, MATRIX::kIdentity, 1e-4f));
static_assert(IsEqualApprox(MATRIX::MakeInvSRT(VECTOR3 { 1.f, 2.f, 3.f }, kQ, VECTOR3 { 4.f, 5.f, 6.f }), kInv, 1e-4f));
static_assert(IsEqualApprox(XformPoint(VECTOR3 { 1.f, 0.f, 0.f }, MATRIX::MakeTranslation(VECTOR3 { 1.f, 2.f, 3.f })), VECTOR3 { 2.f, 2.f, 3.f }));
static_assert(IsEqualApprox(QUATERNION::MakeFromMatrix(MATRIX::MakeRotation(kQ)), kQ, 1e-5f));

constexpr float kSin = Sin(1.f);
static_assert(kSin > 0.8414709f && kSin < 0.8414711f);

constexpr MATRIX_REG  kRegIdentity = MATRIX_REG::kIdentity;
constexpr VECTOR3_REG kRegUp       = MathConstants<VECTOR3_REG>::kUp;

}   // namespace constexpr_check

TEST(MathConstexpr, MatchesRuntime)
{
    volatile float   half = 0.5f;
    const QUATERNION q    = QUATERNION::MakeFromAxisAngle(VECTOR3 { 0.f, 1.f, 0.f }, half);
    const MATRIX     srt  = MATRIX::MakeSRT(VECTOR3 { 1.f, 2.f, 3.f }, q, VECTOR3 { 4.f, 5.f, 6.f });
    volatile float   one  = 1.f;

    EXPECT_LT(RotationDiff(q, constexpr_check::kQ), 1e-6f);
    EXPECT_LT(MaxRelDiff(srt, constexpr_check::kSrt), 1e-6f);
    EXPECT_LT(MaxRelDiff(Inverse(srt), constexpr_check::kInv), 1e-5f);
    EXPECT_LT(RelDiff(Sin(one), constexpr_check::kSin), 1e-6f);
    EXPECT_EQ(Store(constexpr_check::kRegIdentity), MATRIX::kIdentity);
    EXPECT_EQ(Store(constexpr_check::kRegUp), Up<VECTOR3>());
}

// ==========================================================
//  의미 규칙
// ==========================================================

TEST(MathSemantics, NormalizeZeroIsZero)
{
    EXPECT_EQ(Normalize(Zero<VECTOR4>()), Zero<VECTOR4>());
    EXPECT_EQ(Normalize(Zero<VECTOR3>()), Zero<VECTOR3>());
    EXPECT_EQ(Normalize(Zero<VECTOR2>()), Zero<VECTOR2>());
    EXPECT_EQ(Store(Normalize(Zero<VECTOR4_REG>())), Zero<VECTOR4>());
    EXPECT_EQ(Store(Normalize(Zero<VECTOR3_REG>())), Zero<VECTOR3>());
    EXPECT_EQ(Store(Normalize(Zero<VECTOR2_REG>())), Zero<VECTOR2>());
}

TEST(MathSemantics, NormalizeTinyIsUnit)
{
    // DirectXMath 처럼 길이 0 만 0 이다 (예전 kEpsilon 판정은 길이 3e-3 미만을 전부 0 으로 만들었다)
    const VECTOR3 v3 { 1e-6f, 2e-6f, 0.f };
    const VECTOR4 v4 { 1e-6f, 0.f, 0.f, 1e-6f };
    EXPECT_NEAR(Length(Normalize(v3)), 1.f, 1e-5f);
    EXPECT_NEAR(Length(Normalize(v4)), 1.f, 1e-5f);
    EXPECT_NEAR(Length(Normalize(Load(v3))), 1.f, 1e-5f);
    EXPECT_NEAR(Length(Normalize(Load(v4))), 1.f, 1e-5f);
    EXPECT_NEAR(Length(Normalize(VECTOR3_REG { 1e-20f, 0.f, 0.f })), 1.f, 1e-5f);
}

TEST(MathSemantics, InverseSingularIsZero)
{
    const MATRIX singular { VECTOR4 { 1.f, 2.f, 3.f, 4.f }, VECTOR4 { 2.f, 4.f, 6.f, 8.f }, VECTOR4 { 0.f, 1.f, 0.f, 0.f }, VECTOR4 { 0.f, 0.f, 0.f, 1.f } };
    float        det = 1.f;
    EXPECT_EQ(Inverse(singular, &det), MATRIX::kZero);
    EXPECT_EQ(det, 0.f);

    det = 1.f;
    EXPECT_EQ(Store(Inverse(Load(singular), &det)), MATRIX::kZero);
    EXPECT_EQ(det, 0.f);
}

TEST(MathSemantics, InverseSmallScaleIsNotZero)
{
    const MATRIX m = MATRIX::MakeScale(VECTOR3 { 0.01f, 0.01f, 0.01f });
    EXPECT_LT(MaxRelDiff(m * Inverse(m), MATRIX::kIdentity), 1e-5f);
    EXPECT_LT(MaxRelDiff(Load(m) * Inverse(Load(m)), MATRIX::kIdentity), 1e-5f);
}

TEST(MathSemantics, LoadStore)
{
    const VECTOR2    v2 { 1.f, 2.f };
    const VECTOR3    v3 { 1.f, 2.f, 3.f };
    const VECTOR4    v4 { 1.f, 2.f, 3.f, 4.f };
    const QUATERNION q { 0.1f, 0.2f, 0.3f, 0.4f };
    const MATRIX     m = MATRIX::MakeTranslation(v3);
    EXPECT_EQ(Store(Load(v2)), v2);
    EXPECT_EQ(Store(Load(v3)), v3);
    EXPECT_EQ(Store(Load(v4)), v4);
    EXPECT_EQ(Store(Load(q)), q);
    EXPECT_EQ(Store(Load(m)), m);

    VECTOR4_REG r = Load(v4);
    EXPECT_EQ(r.GetX(), 1.f);
    EXPECT_EQ(r.GetW(), 4.f);
    EXPECT_EQ(r[2], 3.f);
    r.SetZ(8.f);
    EXPECT_EQ(Store(r), (VECTOR4 { 1.f, 2.f, 8.f, 4.f }));
    EXPECT_EQ(Store(VECTOR3_REG { r }), (VECTOR3 { 1.f, 2.f, 8.f }));
    EXPECT_EQ(Store(VECTOR4_REG { Load(v3), 9.f }), (VECTOR4 { 1.f, 2.f, 3.f, 9.f }));
    EXPECT_TRUE(Load(v3) == (VECTOR3_REG { 1.f, 2.f, 3.f }));
}

TEST(MathSemantics, InverseFactoriesMatchInverse)
{
    Rng rng(7);
    for (int i = 0; i < kCount; ++i)
    {
        const VECTOR3    s   = rng.Vec3(0.5f, 2.f);
        const VECTOR3    t   = rng.Vec3(-10.f, 10.f);
        const QUATERNION q   = rng.Quat();
        const VECTOR3    pyr = rng.Vec3(-3.f, 3.f);
        const VECTOR3    dir = rng.Dir();
        const VECTOR3    up  = Up<VECTOR3>();

        EXPECT_LT(MaxRelDiff(MATRIX::MakeInvSRT(s, q, t), Inverse(MATRIX::MakeSRT(s, q, t))), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeInvSRT(s, pyr, t), Inverse(MATRIX::MakeSRT(s, pyr, t))), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeInvSRT(s, dir, up, t), Inverse(MATRIX::MakeSRT(s, dir, up, t))), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeViewLookTo(t, dir, up), Inverse(MATRIX::MakeSRT(One<VECTOR3>(), dir, up, t))), 1e-4f);

        const float fov = rng.Float(0.5f, 1.5f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeInvPersp(fov, 1.7f, 0.1f, 100.f), Inverse(MATRIX::MakePersp(fov, 1.7f, 0.1f, 100.f))), 1e-3f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeInvOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f), Inverse(MATRIX::MakeOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f))), 1e-4f);
    }
}

TEST(MathSemantics, RoundTrips)
{
    Rng rng(11);
    for (int i = 0; i < kCount; ++i)
    {
        // 짐벌락 근처는 제외
        const VECTOR3    pyr = VECTOR3 { rng.Float(-1.4f, 1.4f), rng.Float(-3.f, 3.f), rng.Float(-3.f, 3.f) };
        const QUATERNION q   = QUATERNION::MakeFromEuler(pyr);
        EXPECT_LT(RotationDiff(QUATERNION::MakeFromEuler(q.ToEuler()), q), 1e-4f);

        const AXIS_ANGLE aa = q.ToAxisAngle();
        EXPECT_LT(RotationDiff(QUATERNION::MakeFromAxisAngle(aa.axis, aa.rad), q), 1e-4f);

        EXPECT_LT(RotationDiff(QUATERNION::MakeFromMatrix(MATRIX::MakeRotation(q)), q), 1e-5f);
        EXPECT_LT(MaxRelDiff(MATRIX::MakeRotation(q), MATRIX::MakeRotation(pyr)), 1e-5f);

        const VECTOR3 v = rng.Vec3(-5.f, 5.f);
        EXPECT_LT(MaxRelDiff(RotateInv(Rotate(v, q), q), v), 1e-5f);
        EXPECT_LT(MaxRelDiff(Rotate(v, q), XformVector(v, MATRIX::MakeRotation(q))), 1e-5f);

        const VECTOR3 from = rng.Dir();
        const VECTOR3 to   = rng.Dir();
        EXPECT_LT(MaxRelDiff(Rotate(from, QUATERNION::MakeFromTo(from, to)), to), 1e-4f);

        const VECTOR3 s = rng.Vec3(0.5f, 2.f);
        const VECTOR3 t = rng.Vec3(-10.f, 10.f);
        VECTOR3       outS;
        QUATERNION    outQ;
        VECTOR3       outT;
        Decompose(MATRIX::MakeSRT(s, q, t), &outS, &outQ, &outT);
        EXPECT_LT(MaxRelDiff(outS, s), 1e-5f);
        EXPECT_LT(RotationDiff(outQ, q), 1e-4f);
        EXPECT_LT(MaxRelDiff(outT, t), 1e-6f);

        VECTOR3_REG    regS;
        QUATERNION_REG regQ;
        VECTOR3_REG    regT;
        Decompose(MATRIX_REG::MakeSRT(Load(s), Load(q), Load(t)), &regS, &regQ, &regT);
        EXPECT_LT(MaxRelDiff(regS, s), 1e-5f);
        EXPECT_LT(RotationDiff(regQ, q), 1e-4f);
        EXPECT_LT(MaxRelDiff(regT, t), 1e-6f);
    }
}

TEST(MathSemantics, ScalarTrigMatchesStd)
{
    for (int i = -2000; i <= 2000; ++i)
    {
        const float x = static_cast<float>(i) * 0.0157f;   // 약 [-10pi, 10pi]
        EXPECT_NEAR(Sin(x), std::sin(static_cast<double>(x)), 2e-6) << x;
        EXPECT_NEAR(Cos(x), std::cos(static_cast<double>(x)), 2e-6) << x;
        float s = 0.f;
        float c = 0.f;
        SinCos(x, s, c);
        EXPECT_EQ(s, Sin(x));
        EXPECT_EQ(c, Cos(x));
    }
    for (int i = -100; i <= 100; ++i)
    {
        const float x = static_cast<float>(i) * 0.01f;
        EXPECT_NEAR(ACos(x), std::acos(static_cast<double>(x)), 2e-6) << x;
        EXPECT_NEAR(ASin(x), std::asin(static_cast<double>(x)), 2e-6) << x;
    }
}

// ==========================================================
//  연산 타입 == 저장 타입
// ==========================================================

TEST(MathRegister, VectorMatchesStorage)
{
    Rng rng(21);
    for (int i = 0; i < kCount; ++i)
    {
        const VECTOR4 a = rng.Vec4(-3.f, 3.f);
        const VECTOR4 b = rng.Vec4(-3.f, 3.f);
        const VECTOR3 c = rng.Vec3(-3.f, 3.f);
        const VECTOR3 d = rng.Vec3(-3.f, 3.f);
        const VECTOR2 e = c.ToVector2();
        const VECTOR2 f = d.ToVector2();

        EXPECT_LT(MaxRelDiff(Load(a) + Load(b), a + b), kTol);
        EXPECT_LT(MaxRelDiff(Load(a) * 3.f - Load(b) / 2.f, a * 3.f - b / 2.f), kTol);
        EXPECT_LT(MaxRelDiff(Normalize(Load(a)), Normalize(a)), kTol);
        EXPECT_LT(MaxRelDiff(Lerp(Load(a), Load(b), 0.3f), Lerp(a, b, 0.3f)), kTol);
        EXPECT_LT(MaxRelDiff(Clamp(Load(a), Load(b), Load(b) + VECTOR4_REG { 1.f }), Clamp(a, b, b + VECTOR4 { 1.f })), kTol);
        EXPECT_LT(RelDiff(Dot(Load(a), Load(b)), Dot(a, b)), kTol);
        EXPECT_LT(RelDiff(Length(Load(a)), Length(a)), kTol);

        EXPECT_LT(MaxRelDiff(Normalize(Load(c)), Normalize(c)), kTol);
        EXPECT_LT(MaxRelDiff(Cross(Load(c), Load(d)), Cross(c, d)), kTol);
        EXPECT_LT(RelDiff(Dot(Load(c), Load(d)), Dot(c, d)), kTol);
        EXPECT_LT(RelDiff(Distance(Load(c), Load(d)), Distance(c, d)), kTol);

        EXPECT_LT(RelDiff(Dot(Load(e), Load(f)), Dot(e, f)), kTol);
        EXPECT_LT(RelDiff(Cross(Load(e), Load(f)), Cross(e, f)), kTol);
        EXPECT_LT(MaxRelDiff(Store(Normalize(Load(e))).GetPtr(), Normalize(e).GetPtr(), 2), kTol);
    }
}

TEST(MathRegister, MatrixMatchesStorage)
{
    Rng rng(22);
    for (int i = 0; i < kCount; ++i)
    {
        const MATRIX  a = rng.Mtx();
        const MATRIX  b = rng.Mtx();
        const VECTOR4 v = rng.Vec4(-3.f, 3.f);
        const VECTOR3 p = rng.Vec3(-3.f, 3.f);

        EXPECT_LT(MaxRelDiff(Load(a) * Load(b), a * b), kTol);
        EXPECT_LT(MaxRelDiff(Load(a) + Load(b) * 2.f, a + b * 2.f), kTol);
        EXPECT_LT(MaxRelDiff(Transpose(Load(a)), Transpose(a)), kTol);
        EXPECT_LT(MaxRelDiff(Inverse(Load(a)), Inverse(a)), 1e-4f);
        EXPECT_LT(RelDiff(Determinant(Load(a)), Determinant(a)), 1e-4f);
        EXPECT_LT(MaxRelDiff(Xform(Load(v), Load(a)), Xform(v, a)), kTol);
        EXPECT_LT(MaxRelDiff(XformPoint(Load(p), Load(a)), XformPoint(p, a)), kTol);
        EXPECT_LT(MaxRelDiff(XformVector(Load(p), Load(a)), XformVector(p, a)), kTol);
    }
}

TEST(MathRegister, MatrixFactoryMatchesStorage)
{
    Rng rng(23);
    for (int i = 0; i < kCount; ++i)
    {
        const float      rad  = rng.Float(-6.f, 6.f);
        const VECTOR3    axis = rng.Dir();
        const VECTOR3    pyr  = rng.Vec3(-3.f, 3.f);
        const QUATERNION q    = rng.Quat();
        const VECTOR3    s    = rng.Vec3(0.5f, 2.f);
        const VECTOR3    t    = rng.Vec3(-10.f, 10.f);
        const VECTOR3    dir  = rng.Dir();
        const VECTOR3    up   = Up<VECTOR3>();
        const float      fov  = rng.Float(0.5f, 1.5f);

        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeTranslation(Load(t)), MATRIX::MakeTranslation(t)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeScale(Load(s)), MATRIX::MakeScale(s)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationX(rad), MATRIX::MakeRotationX(rad)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationY(rad), MATRIX::MakeRotationY(rad)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationZ(rad), MATRIX::MakeRotationZ(rad)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(pyr)), MATRIX::MakeRotation(pyr)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(axis), rad), MATRIX::MakeRotation(axis, rad)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(q)), MATRIX::MakeRotation(q)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationLookTo(Load(dir), Load(up)), MATRIX::MakeRotationLookTo(dir, up)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeSRT(Load(s), Load(dir), Load(up), Load(t)), MATRIX::MakeSRT(s, dir, up, t)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeSRT(Load(s), Load(pyr), Load(t)), MATRIX::MakeSRT(s, pyr, t)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeSRT(Load(s), Load(q), Load(t)), MATRIX::MakeSRT(s, q, t)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeInvSRT(Load(s), Load(dir), Load(up), Load(t)), MATRIX::MakeInvSRT(s, dir, up, t)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeInvSRT(Load(s), Load(pyr), Load(t)), MATRIX::MakeInvSRT(s, pyr, t)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeInvSRT(Load(s), Load(q), Load(t)), MATRIX::MakeInvSRT(s, q, t)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeViewLookTo(Load(t), Load(dir), Load(up)), MATRIX::MakeViewLookTo(t, dir, up)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeViewLookTo(Load(t), Load(pyr)), MATRIX::MakeViewLookTo(t, pyr)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeViewLookTo(Load(t), Load(q)), MATRIX::MakeViewLookTo(t, q)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakePersp(fov, 1.7f, 0.1f, 100.f), MATRIX::MakePersp(fov, 1.7f, 0.1f, 100.f)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeInvPersp(fov, 1.7f, 0.1f, 100.f), MATRIX::MakeInvPersp(fov, 1.7f, 0.1f, 100.f)), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f), MATRIX::MakeOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeOrtho(8.f, 6.f, 0.5f, 50.f), MATRIX::MakeOrtho(8.f, 6.f, 0.5f, 50.f)), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeInvOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f), MATRIX::MakeInvOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f)), kTol);
    }
}

TEST(MathRegister, QuaternionMatchesStorage)
{
    Rng rng(24);
    for (int i = 0; i < kCount; ++i)
    {
        const QUATERNION a    = rng.Quat();
        const QUATERNION b    = rng.Quat();
        const VECTOR3    v    = rng.Vec3(-3.f, 3.f);
        const VECTOR3    axis = rng.Dir();
        const float      rad  = rng.Float(-6.f, 6.f);
        const VECTOR3    pyr  = rng.Vec3(-3.f, 3.f);
        const VECTOR3    from = rng.Dir();
        const VECTOR3    to   = rng.Dir();
        const float      t    = rng.Float(0.f, 1.f);

        EXPECT_LT(RotationDiff(Load(a) * Load(b), a * b), kTol);
        EXPECT_LT(RotationDiff(Slerp(Load(a), Load(b), t), Slerp(a, b, t)), 1e-4f);
        EXPECT_LT(RotationDiff(Nlerp(Load(a), Load(b), t), Nlerp(a, b, t)), kTol);
        EXPECT_LT(RotationDiff(Conjugate(Load(a)), Conjugate(a)), kTol);
        EXPECT_LT(RotationDiff(Inverse(Load(a)), Inverse(a)), kTol);
        EXPECT_LT(MaxRelDiff(Rotate(Load(v), Load(a)), Rotate(v, a)), kTol);
        EXPECT_LT(MaxRelDiff(RotateInv(Load(v), Load(a)), RotateInv(v, a)), kTol);
        EXPECT_LT(RotationDiff(QUATERNION_REG::MakeFromAxisAngle(Load(axis), rad), QUATERNION::MakeFromAxisAngle(axis, rad)), kTol);
        EXPECT_LT(RotationDiff(QUATERNION_REG::MakeFromEuler(Load(pyr)), QUATERNION::MakeFromEuler(pyr)), kTol);
        EXPECT_LT(RotationDiff(QUATERNION_REG::MakeFromTo(Load(from), Load(to)), QUATERNION::MakeFromTo(from, to)), 1e-4f);
        EXPECT_LT(RotationDiff(QUATERNION_REG::MakeFromMatrix(MATRIX_REG::MakeRotation(Load(a))), a), 1e-5f);
    }
}

// ==========================================================
//  DirectXMath 기준값
// ==========================================================

#ifdef JUG_TEST_WITH_DXM

namespace
{

[[nodiscard]] DirectX::XMMATRIX ToXM(
    const MATRIX& _mtx)
{
    return DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(_mtx.GetPtr()));
}

[[nodiscard]] MATRIX FromXM(
    DirectX::FXMMATRIX _mtx)
{
    MATRIX ret;
    DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(ret.GetPtr()), _mtx);
    return ret;
}

[[nodiscard]] DirectX::XMVECTOR ToXM(
    const VECTOR4 _v)
{
    return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(_v.GetPtr()));
}

[[nodiscard]] DirectX::XMVECTOR ToXM(
    const VECTOR3 _v)
{
    return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(_v.GetPtr()));
}

[[nodiscard]] DirectX::XMVECTOR ToXM(
    const QUATERNION _q)
{
    return ToXM(_q.v);
}

[[nodiscard]] VECTOR4 FromXM4(
    DirectX::FXMVECTOR _v)
{
    VECTOR4 ret;
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(ret.GetPtr()), _v);
    return ret;
}

[[nodiscard]] VECTOR3 FromXM3(
    DirectX::FXMVECTOR _v)
{
    VECTOR3 ret;
    DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(ret.GetPtr()), _v);
    return ret;
}

}   // namespace

TEST(MathDXM, Vector)
{
    Rng rng(1);
    for (int i = 0; i < kCount; ++i)
    {
        const VECTOR4 a = rng.Vec4(-3.f, 3.f);
        const VECTOR4 b = rng.Vec4(-3.f, 3.f);
        const VECTOR3 c = rng.Vec3(-3.f, 3.f);
        const VECTOR3 d = rng.Vec3(-3.f, 3.f);

        EXPECT_LT(MaxRelDiff(Normalize(Load(a)), FromXM4(DirectX::XMVector4Normalize(ToXM(a)))), kTol);
        EXPECT_LT(RelDiff(Dot(Load(a), Load(b)), DirectX::XMVectorGetX(DirectX::XMVector4Dot(ToXM(a), ToXM(b)))), kTol);
        EXPECT_LT(RelDiff(Length(Load(a)), DirectX::XMVectorGetX(DirectX::XMVector4Length(ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(Lerp(Load(a), Load(b), 0.3f), FromXM4(DirectX::XMVectorLerp(ToXM(a), ToXM(b), 0.3f))), kTol);
        EXPECT_LT(MaxRelDiff(Min(Load(a), Load(b)), FromXM4(DirectX::XMVectorMin(ToXM(a), ToXM(b)))), kTol);
        EXPECT_LT(MaxRelDiff(Abs(Load(a)), FromXM4(DirectX::XMVectorAbs(ToXM(a)))), kTol);

        EXPECT_LT(MaxRelDiff(Normalize(Load(c)), FromXM3(DirectX::XMVector3Normalize(ToXM(c)))), kTol);
        EXPECT_LT(MaxRelDiff(Cross(Load(c), Load(d)), FromXM3(DirectX::XMVector3Cross(ToXM(c), ToXM(d)))), kTol);
        EXPECT_LT(RelDiff(Dot(Load(c), Load(d)), DirectX::XMVectorGetX(DirectX::XMVector3Dot(ToXM(c), ToXM(d)))), kTol);
        EXPECT_LT(MaxRelDiff(Normalize(c), FromXM3(DirectX::XMVector3Normalize(ToXM(c)))), kTol);
    }
}

TEST(MathDXM, Matrix)
{
    Rng rng(2);
    for (int i = 0; i < kCount; ++i)
    {
        const MATRIX  a = rng.Mtx();
        const MATRIX  b = rng.Mtx();
        const VECTOR4 v = rng.Vec4(-3.f, 3.f);
        const VECTOR3 p = rng.Vec3(-3.f, 3.f);

        EXPECT_LT(MaxRelDiff(Load(a) * Load(b), FromXM(DirectX::XMMatrixMultiply(ToXM(a), ToXM(b)))), kTol);
        EXPECT_LT(MaxRelDiff(Transpose(Load(a)), FromXM(DirectX::XMMatrixTranspose(ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(Inverse(Load(a)), FromXM(DirectX::XMMatrixInverse(nullptr, ToXM(a)))), 1e-4f);
        EXPECT_LT(RelDiff(Determinant(Load(a)), DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(ToXM(a)))), 1e-4f);
        EXPECT_LT(MaxRelDiff(Xform(Load(v), Load(a)), FromXM4(DirectX::XMVector4Transform(ToXM(v), ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(XformPoint(Load(p), Load(a)), FromXM3(DirectX::XMVector3TransformCoord(ToXM(p), ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(XformVector(Load(p), Load(a)), FromXM3(DirectX::XMVector3TransformNormal(ToXM(p), ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(a * b, FromXM(DirectX::XMMatrixMultiply(ToXM(a), ToXM(b)))), kTol);
        EXPECT_LT(MaxRelDiff(Inverse(a), FromXM(DirectX::XMMatrixInverse(nullptr, ToXM(a)))), 1e-4f);
    }
}

TEST(MathDXM, MatrixFactory)
{
    Rng rng(3);
    for (int i = 0; i < kCount; ++i)
    {
        const float      rad  = rng.Float(-6.f, 6.f);
        const VECTOR3    axis = rng.Dir();
        const VECTOR3    pyr  = rng.Vec3(-3.f, 3.f);
        const QUATERNION q    = rng.Quat();
        const VECTOR3    s    = rng.Vec3(0.5f, 2.f);
        const VECTOR3    t    = rng.Vec3(-10.f, 10.f);
        const VECTOR3    dir  = rng.Dir();

        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationX(rad), FromXM(DirectX::XMMatrixRotationX(rad))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationY(rad), FromXM(DirectX::XMMatrixRotationY(rad))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotationZ(rad), FromXM(DirectX::XMMatrixRotationZ(rad))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(axis), rad), FromXM(DirectX::XMMatrixRotationNormal(ToXM(axis), rad))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(pyr)), FromXM(DirectX::XMMatrixRotationRollPitchYaw(pyr[0], pyr[1], pyr[2]))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeRotation(Load(q)), FromXM(DirectX::XMMatrixRotationQuaternion(ToXM(q)))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeSRT(Load(s), Load(q), Load(t)), FromXM(DirectX::XMMatrixAffineTransformation(ToXM(s), DirectX::XMVectorZero(), ToXM(q), ToXM(t)))), kTol);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeViewLookTo(Load(t), Load(dir), MathConstants<VECTOR3_REG>::kUp), FromXM(DirectX::XMMatrixLookToLH(ToXM(t), ToXM(dir), DirectX::g_XMIdentityR1))), 1e-4f);

        const float fov = rng.Float(0.5f, 1.5f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakePersp(fov, 1.7f, 0.1f, 100.f), FromXM(DirectX::XMMatrixPerspectiveFovLH(fov, 1.7f, 0.1f, 100.f))), 1e-4f);
        EXPECT_LT(MaxRelDiff(MATRIX_REG::MakeOrtho(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f), FromXM(DirectX::XMMatrixOrthographicOffCenterLH(-3.f, 5.f, -2.f, 4.f, 0.5f, 50.f))), kTol);
    }
}

TEST(MathDXM, Quaternion)
{
    Rng rng(4);
    for (int i = 0; i < kCount; ++i)
    {
        const QUATERNION a    = rng.Quat();
        const QUATERNION b    = rng.Quat();
        const VECTOR3    v    = rng.Vec3(-3.f, 3.f);
        const VECTOR3    axis = rng.Dir();
        const float      rad  = rng.Float(-6.f, 6.f);
        const VECTOR3    pyr  = rng.Vec3(-3.f, 3.f);

        EXPECT_LT(MaxRelDiff(Store(Load(a) * Load(b)).v, FromXM4(DirectX::XMQuaternionMultiply(ToXM(a), ToXM(b)))), kTol);
        EXPECT_LT(MaxRelDiff(Store(Slerp(Load(a), Load(b), 0.3f)).v, FromXM4(DirectX::XMQuaternionSlerp(ToXM(a), ToXM(b), 0.3f))), 1e-4f);
        EXPECT_LT(MaxRelDiff(Rotate(Load(v), Load(a)), FromXM3(DirectX::XMVector3Rotate(ToXM(v), ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(RotateInv(Load(v), Load(a)), FromXM3(DirectX::XMVector3InverseRotate(ToXM(v), ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(Store(Conjugate(Load(a))).v, FromXM4(DirectX::XMQuaternionConjugate(ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(Store(Inverse(Load(a))).v, FromXM4(DirectX::XMQuaternionInverse(ToXM(a)))), kTol);
        EXPECT_LT(MaxRelDiff(Store(QUATERNION_REG::MakeFromAxisAngle(Load(axis), rad)).v, FromXM4(DirectX::XMQuaternionRotationNormal(ToXM(axis), rad))), kTol);
        EXPECT_LT(MaxRelDiff(Store(QUATERNION_REG::MakeFromEuler(Load(pyr))).v, FromXM4(DirectX::XMQuaternionRotationRollPitchYaw(pyr[0], pyr[1], pyr[2]))), kTol);
        EXPECT_LT(RotationDiff(QUATERNION_REG::MakeFromMatrix(MATRIX_REG::MakeRotation(Load(a))), QUATERNION { FromXM4(DirectX::XMQuaternionRotationMatrix(ToXM(MATRIX::MakeRotation(a)))) }), kTol);
    }
}

TEST(MathDXM, BoundingVolume)
{
    Rng     rng(5);
    FRUSTUM frustum = FRUSTUM::MakeFromMatrix(MATRIX::MakePersp(1.f, 16.f / 9.f, 0.1f, 100.f));
    for (PLANE& plane: frustum.planes)   // Distance(PLANE) 는 정규화된 평면을 요구한다
    {
        const float invLen = 1.f / Length(plane.normal);
        plane.normal       = plane.normal * invLen;
        plane.d           *= invLen;
    }
    for (int i = 0; i < kCount; ++i)
    {
        const MATRIX srt = MATRIX::MakeSRT(rng.Vec3(0.5f, 2.f), rng.Quat(), rng.Vec3(-10.f, 10.f));
        const AABB   box { rng.Vec3(-50.f, 50.f), rng.Vec3(0.1f, 3.f) };
        const SPHERE sphere { rng.Vec3(-50.f, 50.f), rng.Float(0.1f, 3.f) };

        DirectX::BoundingBox xbox;
        std::memcpy(&xbox, &box, sizeof(box));
        DirectX::BoundingBox xboxOut;
        xbox.Transform(xboxOut, ToXM(srt));
        for (const AABB& boxOut: { Xform(box, srt), Xform(box, Load(srt)) })
        {
            EXPECT_LT(MaxRelDiff(boxOut.center, VECTOR3 { xboxOut.Center.x, xboxOut.Center.y, xboxOut.Center.z }), kTol);
            EXPECT_LT(MaxRelDiff(boxOut.extends, VECTOR3 { xboxOut.Extents.x, xboxOut.Extents.y, xboxOut.Extents.z }), kTol);
        }

        DirectX::BoundingSphere xsphere;
        std::memcpy(&xsphere, &sphere, sizeof(sphere));
        DirectX::BoundingSphere xsphereOut;
        xsphere.Transform(xsphereOut, ToXM(srt));
        for (const SPHERE& sphereOut: { Xform(sphere, srt), Xform(sphere, Load(srt)) })
        {
            EXPECT_LT(MaxRelDiff(sphereOut.center, VECTOR3 { xsphereOut.Center.x, xsphereOut.Center.y, xsphereOut.Center.z }), kTol);
            EXPECT_LT(RelDiff(sphereOut.radius, xsphereOut.Radius), kTol);
        }

        // jug 평면 법선은 안쪽, DirectXMath 는 바깥쪽이 양수
        DirectX::XMVECTOR planes[6];
        for (size_t p = 0; p < 6; ++p)
        {
            const PLANE plane = frustum.planes[static_cast<eFrustumPlane>(p)];
            planes[p]         = DirectX::XMVectorNegate(DirectX::XMVectorSet(plane.normal[0], plane.normal[1], plane.normal[2], plane.d));
        }
        const bool hit = xbox.ContainedBy(planes[0], planes[1], planes[2], planes[3], planes[4], planes[5]) != DirectX::DISJOINT;
        EXPECT_EQ(Intersect(box, frustum), hit);
    }
}

#endif   // JUG_TEST_WITH_DXM
