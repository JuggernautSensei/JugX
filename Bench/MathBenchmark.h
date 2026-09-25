#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#include <random>

#include <DirectXCollision.h>
#include <DirectXMath.h>

#include <benchmark/benchmark.h>

#include <JugX/JugX.h>

// jug 수학 vs DirectXMath 벤치마크.
//
//   입력은 jug 타입으로 한 번 만들고, 같은 값을 DirectXMath 저장 타입(XMFLOAT3 / XMFLOAT4A / XMFLOAT4X4A)에
//   그대로 복사해 둠. 양쪽 다 "배열에서 로드 -> 연산 -> 배열에 저장" 을 kMathCount 번 돌리니 메모리 트래픽이 같음.
//   DirectXMath 쪽 XMLoad/XMStore 도 측정에 포함 - 저장된 데이터를 다룰 때 실제로 내는 비용이라서.
//
//   BM_DXM_* 는 측정이 끝난 뒤 같은 입력으로 jug 를 한 번 더 돌려 결과를 비교하고, 최대 상대 오차를 err 카운터로
//   리포트함. err 가 크면 두 함수가 같은 일을 안 하는 것 (곱셈 순서, 평면 방향 등) - 속도보다 이걸 먼저 볼 것.
//
//   단품 연산만 재면 호출부에서 터지는 store-forwarding 실패를 못 봄. 그래서 AABB/SPHERE Xform, AABB vs FRUSTUM 처럼
//   여러 연산이 엮인 경로도 같이 잼.

namespace bench
{

constexpr size_t kMathCount = 1024;

static_assert(sizeof(jug::VECTOR3) == sizeof(DirectX::XMFLOAT3), "VECTOR3 must match XMFLOAT3 layout");
static_assert(sizeof(jug::VECTOR4) == sizeof(DirectX::XMFLOAT4A), "VECTOR4 must match XMFLOAT4A layout");
static_assert(sizeof(jug::QUATERNION) == sizeof(DirectX::XMFLOAT4A), "QUATERNION must match XMFLOAT4A layout");
static_assert(sizeof(jug::MATRIX) == sizeof(DirectX::XMFLOAT4X4A), "MATRIX must match XMFLOAT4X4A layout");
static_assert(sizeof(jug::AABB) == sizeof(DirectX::BoundingBox), "AABB must match BoundingBox layout");
static_assert(sizeof(jug::SPHERE) == sizeof(DirectX::BoundingSphere), "SPHERE must match BoundingSphere layout");

[[nodiscard]] inline float RandomFloat(
    std::mt19937& _rng,
    const float   _min,
    const float   _max)
{
    return std::uniform_real_distribution<float>(_min, _max)(_rng);
}

[[nodiscard]] inline float MaxRelError(
    const float* _x,
    const float* _y,
    const size_t _count)
{
    float err = 0.f;
    for (size_t i = 0; i < _count; ++i)
    {
        err = std::max(err, std::abs(_x[i] - _y[i]) / std::max(1.f, std::abs(_x[i])));
    }
    return err;
}

struct MathBenchInput
{
    MathBenchInput()
    {
        std::mt19937 rng(1);
        const float  pi = std::numbers::pi_v<float>;

        for (size_t i = 0; i < kMathCount; ++i)
        {
            // 역행렬이 안정적이도록 대각 우세 + 투영 열은 w 가 0 근처로 안 가게 작게.
            for (size_t r = 0; r < 4; ++r)
            {
                for (size_t c = 0; c < 3; ++c)
                {
                    mtxA[i][r].Set(c, RandomFloat(rng, -1.f, 1.f) + (r == c ? 3.f : 0.f));
                    mtxB[i][r].Set(c, RandomFloat(rng, -1.f, 1.f));
                }
                mtxA[i][r].Set(3, r == 3 ? RandomFloat(rng, 1.f, 2.f) : RandomFloat(rng, -0.2f, 0.2f));
                mtxB[i][r].Set(3, RandomFloat(rng, -1.f, 1.f));
            }

            v3a[i]    = jug::VECTOR3 { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) };
            v3b[i]    = jug::VECTOR3 { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) };
            v4a[i]    = jug::VECTOR4 { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) };
            v4b[i]    = jug::VECTOR4 { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) };
            dir[i]    = jug::Normalize(jug::VECTOR3 { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, 0.2f, 1.f) });
            qa[i]     = jug::Normalize(jug::QUATERNION { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) });
            qb[i]     = jug::Normalize(jug::QUATERNION { RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f), RandomFloat(rng, -1.f, 1.f) });
            scale[i]  = jug::VECTOR3 { RandomFloat(rng, 0.5f, 2.f), RandomFloat(rng, 0.5f, 2.f), RandomFloat(rng, 0.5f, 2.f) };
            trans[i]  = jug::VECTOR3 { RandomFloat(rng, -10.f, 10.f), RandomFloat(rng, -10.f, 10.f), RandomFloat(rng, -10.f, 10.f) };
            euler[i]  = jug::VECTOR3 { RandomFloat(rng, -pi, pi), RandomFloat(rng, -pi, pi), RandomFloat(rng, -pi, pi) };
            angle[i]  = RandomFloat(rng, -pi, pi);
            fovY[i]   = RandomFloat(rng, 0.5f, 1.5f);
            aspect[i] = RandomFloat(rng, 1.f, 2.f);
            srt[i]    = jug::MATRIX::MakeSRT(scale[i], qa[i], trans[i]);
            rot[i]    = jug::MATRIX::MakeRotation(qa[i]);
            box[i]    = jug::AABB { jug::VECTOR3 { RandomFloat(rng, -50.f, 50.f), RandomFloat(rng, -50.f, 50.f), RandomFloat(rng, 5.f, 120.f) },
                                    jug::VECTOR3 { RandomFloat(rng, 0.1f, 3.f), RandomFloat(rng, 0.1f, 3.f), RandomFloat(rng, 0.1f, 3.f) } };
            sphere[i] = jug::SPHERE { box[i].center, box[i].extends[0] };
        }

        // Distance(PLANE) 는 정규화된 평면을 요구함.
        frustum = jug::FRUSTUM::MakeFromMatrix(jug::MATRIX::MakePersp(1.f, 16.f / 9.f, 0.1f, 100.f));
        for (size_t p = 0; p < frustum.planes.GetSize(); ++p)
        {
            const float invLen       = 1.f / jug::Length(frustum.planes[p].normal);
            frustum.planes[p].normal = frustum.planes[p].normal * invLen;
            frustum.planes[p].d     *= invLen;
        }

        std::memcpy(xmtxA, mtxA, sizeof(mtxA));
        std::memcpy(xmtxB, mtxB, sizeof(mtxB));
        std::memcpy(xsrt, srt, sizeof(srt));
        std::memcpy(xrot, rot, sizeof(rot));
        std::memcpy(xv3a, v3a, sizeof(v3a));
        std::memcpy(xv3b, v3b, sizeof(v3b));
        std::memcpy(xv4a, v4a, sizeof(v4a));
        std::memcpy(xv4b, v4b, sizeof(v4b));
        std::memcpy(xdir, dir, sizeof(dir));
        std::memcpy(xqa, qa, sizeof(qa));
        std::memcpy(xqb, qb, sizeof(qb));
        std::memcpy(xscale, scale, sizeof(scale));
        std::memcpy(xtrans, trans, sizeof(trans));
        std::memcpy(xeuler, euler, sizeof(euler));
        std::memcpy(xbox, box, sizeof(box));
        std::memcpy(xsphere, sphere, sizeof(sphere));

        // jug 평면 법선은 절두체 안쪽, DirectXMath 는 바깥쪽이 양수 -> 부호 반전.
        for (size_t p = 0; p < frustum.planes.GetSize(); ++p)
        {
            const jug::PLANE plane = frustum.planes[p];
            xplanes[p]             = DirectX::XMFLOAT4A { -plane.normal[0], -plane.normal[1], -plane.normal[2], -plane.d };
        }
    }

    jug::MATRIX     mtxA[kMathCount];
    jug::MATRIX     mtxB[kMathCount];
    jug::MATRIX     srt[kMathCount];
    jug::MATRIX     rot[kMathCount];
    jug::VECTOR3    v3a[kMathCount];
    jug::VECTOR3    v3b[kMathCount];
    jug::VECTOR4    v4a[kMathCount];
    jug::VECTOR4    v4b[kMathCount];
    jug::VECTOR3    dir[kMathCount];
    jug::QUATERNION qa[kMathCount];
    jug::QUATERNION qb[kMathCount];
    jug::VECTOR3    scale[kMathCount];
    jug::VECTOR3    trans[kMathCount];
    jug::VECTOR3    euler[kMathCount];
    float           angle[kMathCount];
    float           fovY[kMathCount];
    float           aspect[kMathCount];
    jug::AABB       box[kMathCount];
    jug::SPHERE     sphere[kMathCount];
    jug::FRUSTUM    frustum;

    DirectX::XMFLOAT4X4A    xmtxA[kMathCount];
    DirectX::XMFLOAT4X4A    xmtxB[kMathCount];
    DirectX::XMFLOAT4X4A    xsrt[kMathCount];
    DirectX::XMFLOAT4X4A    xrot[kMathCount];
    DirectX::XMFLOAT3       xv3a[kMathCount];
    DirectX::XMFLOAT3       xv3b[kMathCount];
    DirectX::XMFLOAT4A      xv4a[kMathCount];
    DirectX::XMFLOAT4A      xv4b[kMathCount];
    DirectX::XMFLOAT3       xdir[kMathCount];
    DirectX::XMFLOAT4A      xqa[kMathCount];
    DirectX::XMFLOAT4A      xqb[kMathCount];
    DirectX::XMFLOAT3       xscale[kMathCount];
    DirectX::XMFLOAT3       xtrans[kMathCount];
    DirectX::XMFLOAT3       xeuler[kMathCount];
    DirectX::BoundingBox    xbox[kMathCount];
    DirectX::BoundingSphere xsphere[kMathCount];
    DirectX::XMFLOAT4A      xplanes[6];
};

struct MathBenchOutput
{
    jug::MATRIX     mtx[kMathCount];
    jug::VECTOR3    v3[kMathCount];
    jug::VECTOR4    v4[kMathCount];
    jug::QUATERNION q[kMathCount];
    float           f[kMathCount];
    jug::AABB       box[kMathCount];
    jug::SPHERE     sphere[kMathCount];
    bool            hit[kMathCount];

    DirectX::XMFLOAT4X4A    xmtx[kMathCount];
    DirectX::XMFLOAT3       xv3[kMathCount];
    DirectX::XMFLOAT4A      xv4[kMathCount];
    DirectX::XMFLOAT4A      xq[kMathCount];
    float                   xf[kMathCount];
    DirectX::BoundingBox    xbox[kMathCount];
    DirectX::BoundingSphere xsphere[kMathCount];
    bool                    xhit[kMathCount];
};

inline const MathBenchInput g_mathIn;
inline MathBenchOutput      g_mathOut;

// ===========================================
//  Vector
// ===========================================

static void BM_Jug_Vec3Dot(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.f[i] = jug::Dot(g_mathIn.v3a[i], g_mathIn.v3b[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec3Dot);

static void BM_DXM_Vec3Dot(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat(&g_mathOut.xf[i], DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i]), DirectX::XMLoadFloat3(&g_mathIn.xv3b[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.f[i] = jug::Dot(g_mathIn.v3a[i], g_mathIn.v3b[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.f, g_mathOut.xf, kMathCount);
}
BENCHMARK(BM_DXM_Vec3Dot);

static void BM_Jug_Vec3Cross(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v3[i] = jug::Cross(g_mathIn.v3a[i], g_mathIn.v3b[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec3Cross);

static void BM_DXM_Vec3Cross(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat3(&g_mathOut.xv3[i], DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i]), DirectX::XMLoadFloat3(&g_mathIn.xv3b[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v3[i] = jug::Cross(g_mathIn.v3a[i], g_mathIn.v3b[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v3[0].GetPtr(), &g_mathOut.xv3[0].x, kMathCount * 3);
}
BENCHMARK(BM_DXM_Vec3Cross);

static void BM_Jug_Vec3Normalize(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v3[i] = jug::Normalize(g_mathIn.v3a[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec3Normalize);

static void BM_DXM_Vec3Normalize(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat3(&g_mathOut.xv3[i], DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v3[i] = jug::Normalize(g_mathIn.v3a[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v3[0].GetPtr(), &g_mathOut.xv3[0].x, kMathCount * 3);
}
BENCHMARK(BM_DXM_Vec3Normalize);

static void BM_Jug_Vec4Normalize(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v4[i] = jug::Normalize(g_mathIn.v4a[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec4Normalize);

static void BM_DXM_Vec4Normalize(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xv4[i], DirectX::XMVector4Normalize(DirectX::XMLoadFloat4A(&g_mathIn.xv4a[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v4[i] = jug::Normalize(g_mathIn.v4a[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v4[0].GetPtr(), &g_mathOut.xv4[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_Vec4Normalize);

static void BM_Jug_Vec4Lerp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v4[i] = jug::Lerp(g_mathIn.v4a[i], g_mathIn.v4b[i], 0.3f);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec4Lerp);

static void BM_DXM_Vec4Lerp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xv4[i], DirectX::XMVectorLerp(DirectX::XMLoadFloat4A(&g_mathIn.xv4a[i]), DirectX::XMLoadFloat4A(&g_mathIn.xv4b[i]), 0.3f));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v4[i] = jug::Lerp(g_mathIn.v4a[i], g_mathIn.v4b[i], 0.3f);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v4[0].GetPtr(), &g_mathOut.xv4[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_Vec4Lerp);

// ===========================================
//  Matrix
// ===========================================

static void BM_Jug_MatrixMultiply(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = g_mathIn.mtxA[i] * g_mathIn.mtxB[i];
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixMultiply);

static void BM_DXM_MatrixMultiply(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i]), DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxB[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = g_mathIn.mtxA[i] * g_mathIn.mtxB[i];
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixMultiply);

static void BM_Jug_MatrixTranspose(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::Transpose(g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixTranspose);

static void BM_DXM_MatrixTranspose(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::Transpose(g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixTranspose);

static void BM_Jug_MatrixInverse(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::Inverse(g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixInverse);

static void BM_DXM_MatrixInverse(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::Inverse(g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixInverse);

static void BM_Jug_MatrixDeterminant(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.f[i] = jug::Determinant(g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixDeterminant);

static void BM_DXM_MatrixDeterminant(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat(&g_mathOut.xf[i], DirectX::XMMatrixDeterminant(DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.f[i] = jug::Determinant(g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.f, g_mathOut.xf, kMathCount);
}
BENCHMARK(BM_DXM_MatrixDeterminant);

static void BM_Jug_Vec4Xform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v4[i] = jug::Xform(g_mathIn.v4a[i], g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_Vec4Xform);

static void BM_DXM_Vec4Xform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xv4[i], DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&g_mathIn.xv4a[i]), DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v4[i] = jug::Xform(g_mathIn.v4a[i], g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v4[0].GetPtr(), &g_mathOut.xv4[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_Vec4Xform);

static void BM_Jug_XformPoint(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v3[i] = jug::XformPoint(g_mathIn.v3a[i], g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_XformPoint);

static void BM_DXM_XformPoint(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat3(&g_mathOut.xv3[i], DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i]), DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v3[i] = jug::XformPoint(g_mathIn.v3a[i], g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v3[0].GetPtr(), &g_mathOut.xv3[0].x, kMathCount * 3);
}
BENCHMARK(BM_DXM_XformPoint);

static void BM_Jug_XformVector(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v3[i] = jug::XformVector(g_mathIn.v3a[i], g_mathIn.mtxA[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_XformVector);

static void BM_DXM_XformVector(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat3(&g_mathOut.xv3[i], DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i]), DirectX::XMLoadFloat4x4A(&g_mathIn.xmtxA[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v3[i] = jug::XformVector(g_mathIn.v3a[i], g_mathIn.mtxA[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v3[0].GetPtr(), &g_mathOut.xv3[0].x, kMathCount * 3);
}
BENCHMARK(BM_DXM_XformVector);

// ===========================================
//  Factory
// ===========================================

static void BM_Jug_MatrixSRT(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::MATRIX::MakeSRT(g_mathIn.scale[i], g_mathIn.qa[i], g_mathIn.trans[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixSRT);

static void BM_DXM_MatrixSRT(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixAffineTransformation(DirectX::XMLoadFloat3(&g_mathIn.xscale[i]), DirectX::XMVectorZero(), DirectX::XMLoadFloat4A(&g_mathIn.xqa[i]), DirectX::XMLoadFloat3(&g_mathIn.xtrans[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::MATRIX::MakeSRT(g_mathIn.scale[i], g_mathIn.qa[i], g_mathIn.trans[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixSRT);

static void BM_Jug_MatrixRotationQuat(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::MATRIX::MakeRotation(g_mathIn.qa[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixRotationQuat);

static void BM_DXM_MatrixRotationQuat(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4A(&g_mathIn.xqa[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::MATRIX::MakeRotation(g_mathIn.qa[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixRotationQuat);

static void BM_Jug_MatrixPersp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::MATRIX::MakePersp(g_mathIn.fovY[i], g_mathIn.aspect[i], 0.1f, 100.f);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixPersp);

static void BM_DXM_MatrixPersp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixPerspectiveFovLH(g_mathIn.fovY[i], g_mathIn.aspect[i], 0.1f, 100.f));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::MATRIX::MakePersp(g_mathIn.fovY[i], g_mathIn.aspect[i], 0.1f, 100.f);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixPersp);

static void BM_Jug_MatrixViewLookTo(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.mtx[i] = jug::MATRIX::MakeViewLookTo(g_mathIn.trans[i], g_mathIn.dir[i], jug::Up<jug::VECTOR3>());
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_MatrixViewLookTo);

static void BM_DXM_MatrixViewLookTo(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4x4A(&g_mathOut.xmtx[i], DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&g_mathIn.xtrans[i]), DirectX::XMLoadFloat3(&g_mathIn.xdir[i]), DirectX::g_XMIdentityR1));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.mtx[i] = jug::MATRIX::MakeViewLookTo(g_mathIn.trans[i], g_mathIn.dir[i], jug::Up<jug::VECTOR3>());
    }
    _state.counters["err"] = MaxRelError(g_mathOut.mtx[0].GetPtr(), &g_mathOut.xmtx[0]._11, kMathCount * 16);
}
BENCHMARK(BM_DXM_MatrixViewLookTo);

static void BM_Jug_QuatFromMatrix(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.q[i] = jug::QUATERNION::MakeFromMatrix(g_mathIn.rot[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatFromMatrix);

static void BM_DXM_QuatFromMatrix(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xq[i], DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4A(&g_mathIn.xrot[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    // q 와 -q 는 같은 회전이라 부호를 맞춘 뒤 비교함.
    for (size_t i = 0; i < kMathCount; ++i)
    {
        const jug::QUATERNION q    = jug::QUATERNION::MakeFromMatrix(g_mathIn.rot[i]);
        const float           sign = q[0] * g_mathOut.xq[i].x + q[1] * g_mathOut.xq[i].y + q[2] * g_mathOut.xq[i].z + q[3] * g_mathOut.xq[i].w;
        g_mathOut.q[i]             = sign < 0.f ? -q : q;
    }
    _state.counters["err"] = MaxRelError(g_mathOut.q[0].GetPtr(), &g_mathOut.xq[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_QuatFromMatrix);

static void BM_Jug_QuatFromAxisAngle(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.q[i] = jug::QUATERNION::MakeFromAxisAngle(g_mathIn.dir[i], g_mathIn.angle[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatFromAxisAngle);

static void BM_DXM_QuatFromAxisAngle(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xq[i], DirectX::XMQuaternionRotationNormal(DirectX::XMLoadFloat3(&g_mathIn.xdir[i]), g_mathIn.angle[i]));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.q[i] = jug::QUATERNION::MakeFromAxisAngle(g_mathIn.dir[i], g_mathIn.angle[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.q[0].GetPtr(), &g_mathOut.xq[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_QuatFromAxisAngle);

static void BM_Jug_QuatFromEuler(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.q[i] = jug::QUATERNION::MakeFromEuler(g_mathIn.euler[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatFromEuler);

static void BM_DXM_QuatFromEuler(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xq[i], DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&g_mathIn.xeuler[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.q[i] = jug::QUATERNION::MakeFromEuler(g_mathIn.euler[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.q[0].GetPtr(), &g_mathOut.xq[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_QuatFromEuler);

// ===========================================
//  Quaternion
// ===========================================

static void BM_Jug_QuatMultiply(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.q[i] = g_mathIn.qa[i] * g_mathIn.qb[i];
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatMultiply);

static void BM_DXM_QuatMultiply(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xq[i], DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4A(&g_mathIn.xqa[i]), DirectX::XMLoadFloat4A(&g_mathIn.xqb[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.q[i] = g_mathIn.qa[i] * g_mathIn.qb[i];
    }
    _state.counters["err"] = MaxRelError(g_mathOut.q[0].GetPtr(), &g_mathOut.xq[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_QuatMultiply);

static void BM_Jug_QuatSlerp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.q[i] = jug::Slerp(g_mathIn.qa[i], g_mathIn.qb[i], 0.3f);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatSlerp);

static void BM_DXM_QuatSlerp(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat4A(&g_mathOut.xq[i], DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4A(&g_mathIn.xqa[i]), DirectX::XMLoadFloat4A(&g_mathIn.xqb[i]), 0.3f));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.q[i] = jug::Slerp(g_mathIn.qa[i], g_mathIn.qb[i], 0.3f);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.q[0].GetPtr(), &g_mathOut.xq[0].x, kMathCount * 4);
}
BENCHMARK(BM_DXM_QuatSlerp);

static void BM_Jug_QuatRotate(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.v3[i] = jug::Rotate(g_mathIn.v3a[i], g_mathIn.qa[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_QuatRotate);

static void BM_DXM_QuatRotate(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            DirectX::XMStoreFloat3(&g_mathOut.xv3[i], DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&g_mathIn.xv3a[i]), DirectX::XMLoadFloat4A(&g_mathIn.xqa[i])));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.v3[i] = jug::Rotate(g_mathIn.v3a[i], g_mathIn.qa[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.v3[0].GetPtr(), &g_mathOut.xv3[0].x, kMathCount * 3);
}
BENCHMARK(BM_DXM_QuatRotate);

// ===========================================
//  Bounding Volume
// ===========================================

static void BM_Jug_AABBXform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.box[i] = jug::Xform(g_mathIn.box[i], g_mathIn.srt[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_AABBXform);

static void BM_DXM_AABBXform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathIn.xbox[i].Transform(g_mathOut.xbox[i], DirectX::XMLoadFloat4x4A(&g_mathIn.xsrt[i]));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.box[i] = jug::Xform(g_mathIn.box[i], g_mathIn.srt[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.box[0].center.GetPtr(), &g_mathOut.xbox[0].Center.x, kMathCount * 6);
}
BENCHMARK(BM_DXM_AABBXform);

static void BM_Jug_SphereXform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.sphere[i] = jug::Xform(g_mathIn.sphere[i], g_mathIn.srt[i]);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_SphereXform);

static void BM_DXM_SphereXform(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathIn.xsphere[i].Transform(g_mathOut.xsphere[i], DirectX::XMLoadFloat4x4A(&g_mathIn.xsrt[i]));
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.sphere[i] = jug::Xform(g_mathIn.sphere[i], g_mathIn.srt[i]);
    }
    _state.counters["err"] = MaxRelError(g_mathOut.sphere[0].center.GetPtr(), &g_mathOut.xsphere[0].Center.x, kMathCount * 4);
}
BENCHMARK(BM_DXM_SphereXform);

static void BM_Jug_AABBFrustum(benchmark::State& _state)
{
    for (auto _: _state)
    {
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.hit[i] = jug::Intersect(g_mathIn.box[i], g_mathIn.frustum);
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);
}
BENCHMARK(BM_Jug_AABBFrustum);

static void BM_DXM_AABBFrustum(benchmark::State& _state)
{
    for (auto _: _state)
    {
        const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[0]);
        const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[1]);
        const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[2]);
        const DirectX::XMVECTOR p3 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[3]);
        const DirectX::XMVECTOR p4 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[4]);
        const DirectX::XMVECTOR p5 = DirectX::XMLoadFloat4A(&g_mathIn.xplanes[5]);
        for (size_t i = 0; i < kMathCount; ++i)
        {
            g_mathOut.xhit[i] = g_mathIn.xbox[i].ContainedBy(p0, p1, p2, p3, p4, p5) != DirectX::DISJOINT;
        }
        benchmark::ClobberMemory();
    }
    _state.SetItemsProcessed(_state.iterations() * kMathCount);

    int64_t hits       = 0;
    int64_t mismatches = 0;
    for (size_t i = 0; i < kMathCount; ++i)
    {
        g_mathOut.hit[i]  = jug::Intersect(g_mathIn.box[i], g_mathIn.frustum);
        hits             += g_mathOut.hit[i];
        mismatches       += g_mathOut.hit[i] != g_mathOut.xhit[i];
    }
    _state.counters["hits"] = static_cast<double>(hits);
    _state.counters["err"]  = static_cast<double>(mismatches);
}
BENCHMARK(BM_DXM_AABBFrustum);

}   // namespace bench
