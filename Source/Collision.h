#pragma once
#include "AABB.h"
#include "Frustum.h"
#include "Interval.h"
#include "OBB.h"
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Vector.h"

namespace jug
{

// ====================================================
//  Collision
//
//   ClosestPoint : 도형 위에서 주어진 점과 가장 가까운 점을 구한다.
//   Raycast      : 광선 - 도형 교차.
//   Intersect    : 두 도형이 겹치는지 판정한다. (접점 포함)
//   Disjoint     : !Intersect
//   Contains     : 첫 번째 도형이 두 번째 도형을 완전히 포함하는지 판정한다.
// ====================================================

constexpr INTERVAL kDefaultInterval = INTERVAL { 0.f, MathConstants<float>::kMax };

namespace collision_detail
{

    // AABB/OBB 에 대한 공통 타입
    struct BOX
    {
        VECTOR3                center;
        VECTOR3                extents;
        ARRAY<VECTOR3, 3> axes;
    };

    [[nodiscard]] JUG_MATH_API constexpr BOX MakeBox(
        const AABB& _aabb)
    {
        BOX box;
        box.center  = _aabb.center;
        box.extents = Abs(_aabb.extends);
        box.axes[0] = MathConstants<VECTOR3>::kUnitX;
        box.axes[1] = MathConstants<VECTOR3>::kUnitY;
        box.axes[2] = MathConstants<VECTOR3>::kUnitZ;
        return box;
    }

    [[nodiscard]] JUG_MATH_API constexpr BOX MakeBox(
        const OBB& _obb)
    {
        BOX box;
        box.center  = _obb.GetCenter();
        box.extents = _obb.GetExtents();
        box.axes[0] = _obb.transform.GetAxisX();
        box.axes[1] = _obb.transform.GetAxisY();
        box.axes[2] = _obb.transform.GetAxisZ();
        return box;
    }

    // 박스를 _axis 로 투영했을 때의 반지름
    [[nodiscard]] JUG_MATH_API constexpr float ProjectedRadius(
        const BOX&    _box,
        const VECTOR3 _axis)
    {
        return _box.extents.e[0] * Abs(Dot(_axis, _box.axes[0]))
             + _box.extents.e[1] * Abs(Dot(_axis, _box.axes[1]))
             + _box.extents.e[2] * Abs(Dot(_axis, _box.axes[2]));
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 ToBoxLocal(
        const BOX&    _box,
        const VECTOR3 _point)
    {
        const VECTOR3 d = _point - _box.center;
        return VECTOR3 { Dot(d, _box.axes[0]), Dot(d, _box.axes[1]), Dot(d, _box.axes[2]) };
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 ToBoxWorld(
        const BOX&    _box,
        const VECTOR3 _local)
    {
        return _box.center
             + _box.axes[0] * _local.e[0]
             + _box.axes[1] * _local.e[1]
             + _box.axes[2] * _local.e[2];
    }

    [[nodiscard]] JUG_MATH_API constexpr bool ContainBoxLocalPoint(
        const BOX&    _box,
        const VECTOR3 _point)
    {
        const VECTOR3 l = Abs(ToBoxLocal(_box, _point));
        return l.e[0] <= _box.extents.e[0] + kEpsilon
            && l.e[1] <= _box.extents.e[1] + kEpsilon
            && l.e[2] <= _box.extents.e[2] + kEpsilon;
    }

    // ====================================================
    //  Box vs Box
    // ====================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const BOX& _a,
        const BOX& _b)
    {
        ARRAY<ARRAY<float, 3>, 3> r;
        ARRAY<ARRAY<float, 3>, 3> absR;

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                // 두 축이 평행할 때 외적 축이 영벡터가 되어 생기는 수치 오차를 흡수한다.
                r[i][j]    = Dot(_a.axes[i], _b.axes[j]);
                absR[i][j] = Abs(r[i][j]) + kEpsilon;
            }
        }

        const VECTOR3              d  = _b.center - _a.center;
        const ARRAY<float, 3> t  = { Dot(d, _a.axes[0]), Dot(d, _a.axes[1]), Dot(d, _a.axes[2]) };
        const VECTOR3              ea = _a.extents;
        const VECTOR3              eb = _b.extents;

        // A 의 면 법선 3축
        for (int i = 0; i < 3; ++i)
        {
            const float ra = ea.e[i];
            const float rb = eb.e[0] * absR[i][0] + eb.e[1] * absR[i][1] + eb.e[2] * absR[i][2];
            if (Abs(t[i]) > ra + rb)
            {
                return false;
            }
        }

        // B 의 면 법선 3축
        for (int j = 0; j < 3; ++j)
        {
            const float ra = ea.e[0] * absR[0][j] + ea.e[1] * absR[1][j] + ea.e[2] * absR[2][j];
            const float rb = eb.e[j];
            if (Abs(t[0] * r[0][j] + t[1] * r[1][j] + t[2] * r[2][j]) > ra + rb)
            {
                return false;
            }
        }

        // A0 x B0
        if (Abs(t[2] * r[1][0] - t[1] * r[2][0]) > ea.e[1] * absR[2][0] + ea.e[2] * absR[1][0] + eb.e[1] * absR[0][2] + eb.e[2] * absR[0][1])
        {
            return false;
        }
        // A0 x B1
        if (Abs(t[2] * r[1][1] - t[1] * r[2][1]) > ea.e[1] * absR[2][1] + ea.e[2] * absR[1][1] + eb.e[0] * absR[0][2] + eb.e[2] * absR[0][0])
        {
            return false;
        }
        // A0 x B2
        if (Abs(t[2] * r[1][2] - t[1] * r[2][2]) > ea.e[1] * absR[2][2] + ea.e[2] * absR[1][2] + eb.e[0] * absR[0][1] + eb.e[1] * absR[0][0])
        {
            return false;
        }
        // A1 x B0
        if (Abs(t[0] * r[2][0] - t[2] * r[0][0]) > ea.e[0] * absR[2][0] + ea.e[2] * absR[0][0] + eb.e[1] * absR[1][2] + eb.e[2] * absR[1][1])
        {
            return false;
        }
        // A1 x B1
        if (Abs(t[0] * r[2][1] - t[2] * r[0][1]) > ea.e[0] * absR[2][1] + ea.e[2] * absR[0][1] + eb.e[0] * absR[1][2] + eb.e[2] * absR[1][0])
        {
            return false;
        }
        // A1 x B2
        if (Abs(t[0] * r[2][2] - t[2] * r[0][2]) > ea.e[0] * absR[2][2] + ea.e[2] * absR[0][2] + eb.e[0] * absR[1][1] + eb.e[1] * absR[1][0])
        {
            return false;
        }
        // A2 x B0
        if (Abs(t[1] * r[0][0] - t[0] * r[1][0]) > ea.e[0] * absR[1][0] + ea.e[1] * absR[0][0] + eb.e[1] * absR[2][2] + eb.e[2] * absR[2][1])
        {
            return false;
        }
        // A2 x B1
        if (Abs(t[1] * r[0][1] - t[0] * r[1][1]) > ea.e[0] * absR[1][1] + ea.e[1] * absR[0][1] + eb.e[0] * absR[2][2] + eb.e[2] * absR[2][0])
        {
            return false;
        }
        // A2 x B2
        if (Abs(t[1] * r[0][2] - t[0] * r[1][2]) > ea.e[0] * absR[1][2] + ea.e[1] * absR[0][2] + eb.e[0] * absR[2][1] + eb.e[1] * absR[2][0])
        {
            return false;
        }

        return true;
    }

    // _a 가 _b 를 완전히 포함하는가.
    // _b 를 _a 의 각 축으로 투영한 구간이 _a 의 구간 안에 들어가면 된다.
    [[nodiscard]] JUG_MATH_API constexpr bool Contains(
        const BOX& _a,
        const BOX& _b)
    {
        const VECTOR3 d = _b.center - _a.center;
        for (int i = 0; i < 3; ++i)
        {
            const float t  = Dot(d, _a.axes[i]);
            const float rb = ProjectedRadius(_b, _a.axes[i]);
            if (Abs(t) + rb > _a.extents.e[i] + kEpsilon)
            {
                return false;
            }
        }
        return true;
    }

    // ====================================================
    //  Box vs Triangle
    // ====================================================

    // 원점 중심 / 반너비 _e 인 박스와 세 점이 _axis 위에서 분리되는가.
    [[nodiscard]] JUG_MATH_API constexpr bool SeparatedBoxTriAxis(
        const VECTOR3 _axis,
        const VECTOR3 _e,
        const VECTOR3 _v0,
        const VECTOR3 _v1,
        const VECTOR3 _v2)
    {
        if (IsZeroApprox(LengthSq(_axis)))   // 퇴화 축은 분리축이 될 수 없다
        {
            return false;
        }

        const float p0 = Dot(_v0, _axis);
        const float p1 = Dot(_v1, _axis);
        const float p2 = Dot(_v2, _axis);
        const float r  = _e.e[0] * Abs(_axis.e[0]) + _e.e[1] * Abs(_axis.e[1]) + _e.e[2] * Abs(_axis.e[2]);
        return Min(p0, p1, p2) > r || Max(p0, p1, p2) < -r;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool SeparatedTriTriAxis(
        const VECTOR3   _axis,
        const TRIANGLE& _a,
        const TRIANGLE& _b)
    {
        if (IsZeroApprox(LengthSq(_axis)))
        {
            return false;
        }

        const float a0 = Dot(_a.p0, _axis);
        const float a1 = Dot(_a.p1, _axis);
        const float a2 = Dot(_a.p2, _axis);
        const float b0 = Dot(_b.p0, _axis);
        const float b1 = Dot(_b.p1, _axis);
        const float b2 = Dot(_b.p2, _axis);
        return Min(a0, a1, a2) > Max(b0, b1, b2) || Min(b0, b1, b2) > Max(a0, a1, a2);
    }

    [[nodiscard]] JUG_MATH_API constexpr bool IntersectBoxTriangle(
        const BOX&      _box,
        const TRIANGLE& _tri)
    {
        const VECTOR3 e  = _box.extents;
        const VECTOR3 v0 = ToBoxLocal(_box, _tri.p0);
        const VECTOR3 v1 = ToBoxLocal(_box, _tri.p1);
        const VECTOR3 v2 = ToBoxLocal(_box, _tri.p2);

        const VECTOR3 f0 = v1 - v0;
        const VECTOR3 f1 = v2 - v1;
        const VECTOR3 f2 = v0 - v2;

        // 9축: 박스 축 x 삼각형 변. 박스 축이 단위 기저이므로 외적을 전개해 둔다.
        const VECTOR3 a00 { 0.f, -f0.e[2], f0.e[1] };
        const VECTOR3 a01 { 0.f, -f1.e[2], f1.e[1] };
        const VECTOR3 a02 { 0.f, -f2.e[2], f2.e[1] };
        const VECTOR3 a10 { f0.e[2], 0.f, -f0.e[0] };
        const VECTOR3 a11 { f1.e[2], 0.f, -f1.e[0] };
        const VECTOR3 a12 { f2.e[2], 0.f, -f2.e[0] };
        const VECTOR3 a20 { -f0.e[1], f0.e[0], 0.f };
        const VECTOR3 a21 { -f1.e[1], f1.e[0], 0.f };
        const VECTOR3 a22 { -f2.e[1], f2.e[0], 0.f };

        if (SeparatedBoxTriAxis(a00, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a01, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a02, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a10, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a11, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a12, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a20, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a21, e, v0, v1, v2)
            || SeparatedBoxTriAxis(a22, e, v0, v1, v2))
        {
            return false;
        }

        // 3축: 박스의 면 법선
        if (SeparatedBoxTriAxis(MathConstants<VECTOR3>::kUnitX, e, v0, v1, v2)
            || SeparatedBoxTriAxis(MathConstants<VECTOR3>::kUnitY, e, v0, v1, v2)
            || SeparatedBoxTriAxis(MathConstants<VECTOR3>::kUnitZ, e, v0, v1, v2))
        {
            return false;
        }

        // 1축: 삼각형의 면 법선
        const VECTOR3 n = Cross(f0, f1);
        if (IsZeroApprox(LengthSq(n)))   // 퇴화 삼각형
        {
            return true;
        }

        const float dist = Dot(n, v0);
        const float r    = e.e[0] * Abs(n.e[0]) + e.e[1] * Abs(n.e[1]) + e.e[2] * Abs(n.e[2]);
        return Abs(dist) <= r;
    }

    // ====================================================
    //  Ray helper
    // ====================================================

    struct SLAB_RESULT
    {
        float tEnter;
        float tExit;
        bool  bValid;
    };

    // 원점 중심 / 반너비 _extents 인 박스에 대한 슬랩 테스트.
    [[nodiscard]] JUG_MATH_API constexpr SLAB_RESULT RaycastSlab(
        const VECTOR3 _origin,
        const VECTOR3 _dir,
        const VECTOR3 _extents)
    {
        JUG_ASSERT(IsNormalized(_dir), "Ray direction must be normalized to use RaycastSlab");

        if (std::is_constant_evaluated())
        {
            float tEnter = -MathConstants<float>::kMax;
            float tExit  = MathConstants<float>::kMax;

            for (int i = 0; i < 3; ++i)
            {
                const float o = _origin.e[i];
                const float d = _dir.e[i];
                const float e = _extents.e[i];

                if (Abs(d) < kEpsilon)   // 슬랩과 평행
                {
                    if (Abs(o) > e)
                    {
                        return SLAB_RESULT { tEnter, tExit, false };
                    }
                    continue;
                }

                const float inv = 1.f / d;
                const float t0  = (-e - o) * inv;
                const float t1  = (e - o) * inv;
                tEnter          = Max(tEnter, Min(t0, t1));
                tExit           = Min(tExit, Max(t0, t1));
            }
            return SLAB_RESULT { tEnter, tExit, tExit >= tEnter };
        }

        const float invX = 1.f / _dir.e[0];
        const float invY = 1.f / _dir.e[1];
        const float invZ = 1.f / _dir.e[2];

        const float lx = (-_extents.e[0] - _origin.e[0]) * invX;
        const float hx = (_extents.e[0] - _origin.e[0]) * invX;
        const float ly = (-_extents.e[1] - _origin.e[1]) * invY;
        const float hy = (_extents.e[1] - _origin.e[1]) * invY;
        const float lz = (-_extents.e[2] - _origin.e[2]) * invZ;
        const float hz = (_extents.e[2] - _origin.e[2]) * invZ;

        const float tEnter = Max(Max(Min(lx, hx), Min(ly, hy)), Min(lz, hz));
        const float tExit  = Min(Min(Max(lx, hx), Max(ly, hy)), Max(lz, hz));

        return SLAB_RESULT { tEnter, tExit, tExit >= tEnter };
    }

    // 슬랩 히트 지점의 바깥 방향 법선을 구한다.
    [[nodiscard]] JUG_MATH_API constexpr VECTOR3 CalcSlabNormal(
        const VECTOR3 _localHit,
        const VECTOR3 _extents)
    {
        const float nx = _extents.e[0] > 0.f ? _localHit.e[0] / _extents.e[0] : 0.f;
        const float ny = _extents.e[1] > 0.f ? _localHit.e[1] / _extents.e[1] : 0.f;
        const float nz = _extents.e[2] > 0.f ? _localHit.e[2] / _extents.e[2] : 0.f;

        const float ax = Abs(nx);
        const float ay = Abs(ny);
        const float az = Abs(nz);

        if (ax >= ay && ax >= az)
        {
            return VECTOR3 { nx >= 0.f ? 1.f : -1.f, 0.f, 0.f };
        }
        if (ay >= az)
        {
            return VECTOR3 { 0.f, ny >= 0.f ? 1.f : -1.f, 0.f };
        }
        return VECTOR3 { 0.f, 0.f, nz >= 0.f ? 1.f : -1.f };
    }

    // [_tEnter, _tExit] 중 _interval 안에 들어오는 가장 가까운 t 를 고른다.
    // 광선이 도형 안에서 시작하면 진입점이 버려지고 이탈점이 선택된다.
    [[nodiscard]] JUG_MATH_API constexpr bool SelectRayT(
        const float    _tEnter,
        const float    _tExit,
        const INTERVAL _interval,
        float* const   _pOutT,
        bool* const    _pOutIsEnter)
    {
        if (_tEnter >= _interval.min && _tEnter <= _interval.max)
        {
            *_pOutT       = _tEnter;
            *_pOutIsEnter = true;
            return true;
        }
        if (_tExit >= _interval.min && _tExit <= _interval.max)
        {
            *_pOutT       = _tExit;
            *_pOutIsEnter = false;
            return true;
        }
        return false;
    }

    // 법선은 항상 광선을 마주보도록 뒤집는다.
    JUG_MATH_API constexpr void FillHit(
        HIT* const    _pOutHit,
        const RAY&    _ray,
        const float   _t,
        const VECTOR3 _outwardNormal)
    {
        _pOutHit->t          = _t;
        _pOutHit->point      = RayAt(_ray, _t);
        _pOutHit->bFrontFace = Dot(_ray.dir, _outwardNormal) < 0.f;
        _pOutHit->normal     = _pOutHit->bFrontFace ? _outwardNormal : -_outwardNormal;
    }

}   // namespace collision_detail

// ====================================================
//  ClosestPoint
//   _point와 기하의 가장 가까운 점을 계산한다.
//   _point가 기하 내부에 있을 경우. _point를 바로 리턴한다.
// ====================================================

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 ClosestPoint(
    const AABB&   _aabb,
    const VECTOR3 _point)
{
    return Clamp(_point, _aabb.GetMin(), _aabb.GetMax());
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 ClosestPoint(
    const OBB&    _obb,
    const VECTOR3 _point)
{
    const collision_detail::BOX box   = collision_detail::MakeBox(_obb);
    const VECTOR3               local = collision_detail::ToBoxLocal(box, _point);
    return collision_detail::ToBoxWorld(box, Clamp(local, -box.extents, box.extents));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 ClosestPoint(
    const SPHERE& _sphere,
    const VECTOR3 _point)
{
    const VECTOR3 d     = _point - _sphere.center;
    const float   lenSq = LengthSq(d);
    if (lenSq <= _sphere.radius * _sphere.radius)   // 내부
    {
        return _point;
    }
    return _sphere.center + d * (_sphere.radius * RSqrt(lenSq));
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 ClosestPoint(
    const PLANE   _plane,
    const VECTOR3 _point)
{
    return _point - _plane.normal * Distance(_plane, _point);
}

[[nodiscard]] JUG_MATH_API constexpr VECTOR3 ClosestPoint(
    const TRIANGLE& _triangle,
    const VECTOR3   _point)
{
    const VECTOR3 ab = _triangle.p1 - _triangle.p0;
    const VECTOR3 ac = _triangle.p2 - _triangle.p0;

    const VECTOR3 ap = _point - _triangle.p0;
    const float   d1 = Dot(ab, ap);
    const float   d2 = Dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f)   // 정점 p0 영역
    {
        return _triangle.p0;
    }

    const VECTOR3 bp = _point - _triangle.p1;
    const float   d3 = Dot(ab, bp);
    const float   d4 = Dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3)   // 정점 p1 영역
    {
        return _triangle.p1;
    }

    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)   // 변 p0p1 영역
    {
        return _triangle.p0 + ab * (d1 / (d1 - d3));
    }

    const VECTOR3 cp = _point - _triangle.p2;
    const float   d5 = Dot(ab, cp);
    const float   d6 = Dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6)   // 정점 p2 영역
    {
        return _triangle.p2;
    }

    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)   // 변 p0p2 영역
    {
        return _triangle.p0 + ac * (d2 / (d2 - d6));
    }

    const float va = d3 * d6 - d5 * d4;
    if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)   // 변 p1p2 영역
    {
        return _triangle.p1 + (_triangle.p2 - _triangle.p1) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));
    }

    // 면 내부
    const float denom = 1.f / (va + vb + vc);
    return _triangle.p0 + ab * (vb * denom) + ac * (vc * denom);
}

// ====================================================
//  Raycast
//   _ray.dir 은 정규화되어 있다고 가정한다.
// ====================================================

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&     _ray,
    const AABB&    _aabb,
    HIT* const     _pOutHitOrNull = nullptr,
    const INTERVAL _interval      = kDefaultInterval)
{
    const collision_detail::SLAB_RESULT slab = collision_detail::RaycastSlab(_ray.origin - _aabb.center, _ray.dir, Abs(_aabb.extends));
    if (!slab.bValid)
    {
        return false;
    }

    float t;
    bool  bIsEnter;
    if (!collision_detail::SelectRayT(slab.tEnter, slab.tExit, _interval, &t, &bIsEnter))
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        const VECTOR3 localHit = _ray.origin + _ray.dir * t - _aabb.center;
        collision_detail::FillHit(_pOutHitOrNull, _ray, t, collision_detail::CalcSlabNormal(localHit, Abs(_aabb.extends)));
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&     _ray,
    const OBB&     _obb,
    HIT* const     _pOutHitOrNull = nullptr,
    const INTERVAL _interval      = kDefaultInterval)
{
    const collision_detail::BOX box      = collision_detail::MakeBox(_obb);
    const VECTOR3               localPos = collision_detail::ToBoxLocal(box, _ray.origin);
    const VECTOR3               localDir = VECTOR3 { Dot(_ray.dir, box.axes[0]), Dot(_ray.dir, box.axes[1]), Dot(_ray.dir, box.axes[2]) };

    const collision_detail::SLAB_RESULT slab = collision_detail::RaycastSlab(localPos, localDir, box.extents);
    if (!slab.bValid)
    {
        return false;
    }

    float t        = 0.f;
    bool  bIsEnter = false;
    if (!collision_detail::SelectRayT(slab.tEnter, slab.tExit, _interval, &t, &bIsEnter))
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        const VECTOR3 localHit    = localPos + localDir * t;
        const VECTOR3 localNormal = collision_detail::CalcSlabNormal(localHit, box.extents);

        // 로컬 법선을 박스 축으로 되돌린다. 성분이 하나만 ±1 이라 축 하나만 살아남는다.
        const VECTOR3 outward = box.axes[0] * localNormal.e[0]
                              + box.axes[1] * localNormal.e[1]
                              + box.axes[2] * localNormal.e[2];

        collision_detail::FillHit(_pOutHitOrNull, _ray, t, outward);
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&     _ray,
    const SPHERE&  _sphere,
    HIT* const     _pOutHitOrNull = nullptr,
    const INTERVAL _interval      = kDefaultInterval)
{
    const VECTOR3 oc    = _ray.origin - _sphere.center;
    const float   a     = LengthSq(_ray.dir);
    const float   halfB = Dot(oc, _ray.dir);
    const float   c     = LengthSq(oc) - _sphere.radius * _sphere.radius;
    const float   disc  = halfB * halfB - a * c;
    if (disc < 0.f || IsZeroApprox(a))
    {
        return false;
    }

    const float sq   = Sqrt(disc);
    const float invA = 1.f / a;

    float t        = 0.f;
    bool  bIsEnter = false;
    if (!collision_detail::SelectRayT((-halfB - sq) * invA, (-halfB + sq) * invA, _interval, &t, &bIsEnter))
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        const VECTOR3 point   = RayAt(_ray, t);
        const VECTOR3 outward = Normalize(point - _sphere.center);
        collision_detail::FillHit(_pOutHitOrNull, _ray, t, outward);
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&     _ray,
    const PLANE    _plane,
    HIT* const     _pOutHitOrNull = nullptr,
    const INTERVAL _interval      = kDefaultInterval)
{
    const float denom = Dot(_plane.normal, _ray.dir);
    if (Abs(denom) < kEpsilon)   // 평행
    {
        return false;
    }

    const float t = -(Dot(_plane.normal, _ray.origin) + _plane.d) / denom;
    if (t < _interval.min || t > _interval.max)
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        collision_detail::FillHit(_pOutHitOrNull, _ray, t, _plane.normal);
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&      _ray,
    const TRIANGLE& _triangle,
    HIT* const      _pOutHitOrNull = nullptr,
    const INTERVAL  _interval      = kDefaultInterval)
{
    const VECTOR3 edge1 = _triangle.p1 - _triangle.p0;
    const VECTOR3 edge2 = _triangle.p2 - _triangle.p0;
    const VECTOR3 pvec  = Cross(_ray.dir, edge2);
    const float   det   = Dot(edge1, pvec);
    if (Abs(det) < kEpsilon)   // 평행하거나 퇴화 삼각형
    {
        return false;
    }

    const float   invDet = 1.f / det;
    const VECTOR3 tvec   = _ray.origin - _triangle.p0;

    const float u = Dot(tvec, pvec) * invDet;
    if (u < 0.f || u > 1.f)
    {
        return false;
    }

    const VECTOR3 qvec = Cross(tvec, edge1);
    const float   v    = Dot(_ray.dir, qvec) * invDet;
    if (v < 0.f || u + v > 1.f)
    {
        return false;
    }

    const float t = Dot(edge2, qvec) * invDet;
    if (t < _interval.min || t > _interval.max)
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        collision_detail::FillHit(_pOutHitOrNull, _ray, t, Normalize(Cross(edge1, edge2)));
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Raycast(
    const RAY&     _ray,
    const FRUSTUM& _frustum,
    HIT* const     _pOutHitOrNull = nullptr,
    const INTERVAL _interval      = kDefaultInterval)
{
    float tEnter     = -MathConstants<float>::kMax;
    float tExit      = MathConstants<float>::kMax;
    int   enterPlane = -1;
    int   exitPlane  = -1;

    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        const PLANE plane = _frustum.planes[i];
        const float denom = Dot(plane.normal, _ray.dir);
        const float num   = Dot(plane.normal, _ray.origin) + plane.d;

        if (Abs(denom) < kEpsilon)   // 평면과 평행
        {
            if (num < 0.f)   // 바깥쪽에서 평행 -> 절대 만나지 않음
            {
                return false;
            }
            continue;
        }

        const float t = -num / denom;
        if (denom > 0.f)   // 반공간으로 진입
        {
            if (t > tEnter)
            {
                tEnter     = t;
                enterPlane = static_cast<int>(i);
            }
        }
        else   // 반공간에서 이탈
        {
            if (t < tExit)
            {
                tExit     = t;
                exitPlane = static_cast<int>(i);
            }
        }

        if (tEnter > tExit)
        {
            return false;
        }
    }

    float t        = 0.f;
    bool  bIsEnter = false;
    if (!collision_detail::SelectRayT(tEnter, tExit, _interval, &t, &bIsEnter))
    {
        return false;
    }

    const int planeIndex = bIsEnter ? enterPlane : exitPlane;
    if (planeIndex < 0)   // 경계가 없는 방향 -> 유효한 히트가 아니다
    {
        return false;
    }

    if (_pOutHitOrNull)
    {
        // 절두체 법선은 안쪽을 향하므로 바깥 법선은 부호를 뒤집는다.
        collision_detail::FillHit(_pOutHitOrNull, _ray, t, -_frustum.planes[static_cast<size_t>(planeIndex)].normal);
    }
    return true;
}

namespace collision_detail
{
    // ===================================================
    //  AABB vs ...
    // ===================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB&   _aabb,
        const VECTOR3 _point)
    {
        const VECTOR3 min = _aabb.GetMin();
        const VECTOR3 max = _aabb.GetMax();
        return _point.e[0] >= min.e[0] && _point.e[0] <= max.e[0]
            && _point.e[1] >= min.e[1] && _point.e[1] <= max.e[1]
            && _point.e[2] >= min.e[2] && _point.e[2] <= max.e[2];
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB& _x,
        const AABB& _y)
    {
        const VECTOR3 minX = _x.GetMin();
        const VECTOR3 maxX = _x.GetMax();
        const VECTOR3 minY = _y.GetMin();
        const VECTOR3 maxY = _y.GetMax();
        return minX.e[0] <= maxY.e[0] && maxX.e[0] >= minY.e[0]
            && minX.e[1] <= maxY.e[1] && maxX.e[1] >= minY.e[1]
            && minX.e[2] <= maxY.e[2] && maxX.e[2] >= minY.e[2];
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB& _aabb,
        const OBB&  _obb)
    {
        return Intersect(MakeBox(_aabb), MakeBox(_obb));
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB&   _aabb,
        const SPHERE& _sphere)
    {
        const VECTOR3 closest = ClosestPoint(_aabb, _sphere.center);
        return DistanceSq(closest, _sphere.center) <= _sphere.radius * _sphere.radius;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB&     _aabb,
        const TRIANGLE& _triangle)
    {
        return IntersectBoxTriangle(MakeBox(_aabb), _triangle);
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB& _aabb,
        const PLANE _plane)
    {
        const float dist = Distance(_plane, _aabb.center);
        const float r    = Dot(Abs(_plane.normal), Abs(_aabb.extends));
        return Abs(dist) <= r;
    }

    // 보수적 판정.
    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB&    _aabb,
        const FRUSTUM& _frustum)
    {
        const VECTOR3 extents = Abs(_aabb.extends);
        for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
        {
            const PLANE plane = _frustum.planes[i];
            const float dist  = Distance(plane, _aabb.center);
            const float r     = Dot(extents, Abs(plane.normal));
            if (dist < -r)   // 평면 바깥으로 완전히 밀려남
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const AABB& _aabb,
        const RAY&  _ray)
    {
        return Raycast(_ray, _aabb);
    }

    // ===================================================
    //  OBB vs ...
    // ===================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB&    _obb,
        const VECTOR3 _point)
    {
        return ContainBoxLocalPoint(MakeBox(_obb), _point);
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB& _x,
        const OBB& _y)
    {
        return Intersect(MakeBox(_x), MakeBox(_y));
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB&    _obb,
        const SPHERE& _sphere)
    {
        const VECTOR3 closest = ClosestPoint(_obb, _sphere.center);
        return DistanceSq(closest, _sphere.center) <= _sphere.radius * _sphere.radius;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB&      _obb,
        const TRIANGLE& _triangle)
    {
        return IntersectBoxTriangle(MakeBox(_obb), _triangle);
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB&  _obb,
        const PLANE _plane)
    {
        const BOX   box  = MakeBox(_obb);
        const float dist = Distance(_plane, box.center);
        const float r    = ProjectedRadius(box, _plane.normal);
        return Abs(dist) <= r;
    }

    // 보수적 판정.
    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB&     _obb,
        const FRUSTUM& _frustum)
    {
        const BOX box = MakeBox(_obb);
        for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
        {
            const PLANE plane = _frustum.planes[i];
            const float dist  = Distance(plane, box.center);
            const float r     = ProjectedRadius(box, plane.normal);
            if (dist < -r)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const OBB& _obb,
        const RAY& _ray)
    {
        return Raycast(_ray, _obb);
    }

    // ===================================================
    //  Sphere vs ...
    // =================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE& _sphere,
        const VECTOR3 _point)
    {
        return DistanceSq(_sphere.center, _point) <= _sphere.radius * _sphere.radius;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE& _x,
        const SPHERE& _y)
    {
        const float r = _x.radius + _y.radius;
        return DistanceSq(_x.center, _y.center) <= r * r;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE&   _sphere,
        const TRIANGLE& _triangle)
    {
        const VECTOR3 closest = ClosestPoint(_triangle, _sphere.center);
        return DistanceSq(closest, _sphere.center) <= _sphere.radius * _sphere.radius;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE& _sphere,
        const PLANE   _plane)
    {
        return Abs(Distance(_plane, _sphere.center)) <= _sphere.radius;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE&  _sphere,
        const FRUSTUM& _frustum)
    {
        for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
        {
            if (Distance(_frustum.planes[i], _sphere.center) < -_sphere.radius)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const SPHERE& _sphere,
        const RAY&    _ray)
    {
        return Raycast(_ray, _sphere);
    }

    // ===================================================
    //  Triangle vs ...
    // =================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const TRIANGLE& _triangle,
        const VECTOR3   _point)
    {
        const VECTOR3 closest = ClosestPoint(_triangle, _point);
        return DistanceSq(closest, _point) <= kEpsilon * kEpsilon;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const TRIANGLE& _x,
        const TRIANGLE& _y)
    {
        const VECTOR3 aEdges[3] = { _x.p1 - _x.p0, _x.p2 - _x.p1, _x.p0 - _x.p2 };
        const VECTOR3 bEdges[3] = { _y.p1 - _y.p0, _y.p2 - _y.p1, _y.p0 - _y.p2 };

        const VECTOR3 aNormal = Cross(aEdges[0], aEdges[1]);
        const VECTOR3 bNormal = Cross(bEdges[0], bEdges[1]);

        if (SeparatedTriTriAxis(aNormal, _x, _y) || SeparatedTriTriAxis(bNormal, _x, _y))
        {
            return false;
        }

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                if (SeparatedTriTriAxis(Cross(aEdges[i], bEdges[j]), _x, _y))
                {
                    return false;
                }
            }
        }

        for (int i = 0; i < 3; ++i)
        {
            if (SeparatedTriTriAxis(Cross(aNormal, aEdges[i]), _x, _y)
                || SeparatedTriTriAxis(Cross(bNormal, bEdges[i]), _x, _y))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const TRIANGLE& _triangle,
        const PLANE     _plane)
    {
        const float d0 = Distance(_plane, _triangle.p0);
        const float d1 = Distance(_plane, _triangle.p1);
        const float d2 = Distance(_plane, _triangle.p2);
        return !(Min(d0, d1, d2) > 0.f || Max(d0, d1, d2) < 0.f);
    }

    // 보수적 판정.
    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const TRIANGLE& _triangle,
        const FRUSTUM&  _frustum)
    {
        for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
        {
            const PLANE plane = _frustum.planes[i];
            if (Distance(plane, _triangle.p0) < 0.f
                && Distance(plane, _triangle.p1) < 0.f
                && Distance(plane, _triangle.p2) < 0.f)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const TRIANGLE& _triangle,
        const RAY&      _ray)
    {
        return Raycast(_ray, _triangle);
    }

    // =================================================
    //  Plane vs ...
    // =================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const PLANE   _plane,
        const VECTOR3 _point)
    {
        return IsZeroApprox(Distance(_plane, _point));
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const PLANE _x,
        const PLANE _y)
    {
        if (!IsZeroApprox(Cross(_x.normal, _y.normal)))   // 평행하지 않으면 반드시 교선이 있다
        {
            return true;
        }

        // 평행하면 완전히 같은 평면일 때만 교차한다.
        return Dot(_x.normal, _y.normal) > 0.f ? IsEqualApprox(_x.d, _y.d)
                                               : IsEqualApprox(_x.d, -_y.d);
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const PLANE    _plane,
        const FRUSTUM& _frustum)
    {
        const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners = _frustum.CalcCorners();

        bool bAnyFront  = false;
        bool bAnyBehind = false;
        for (const eCorner e: RangesOf<eCorner>())
        {
            const float dist = Distance(_plane, corners[e]);
            bAnyFront        = bAnyFront || dist >= 0.f;
            bAnyBehind       = bAnyBehind || dist <= 0.f;
        }
        return bAnyFront && bAnyBehind;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const PLANE _plane,
        const RAY&  _ray)
    {
        return Raycast(_ray, _plane);
    }

    // =================================================
    //  Frustum vs ...
    // =================================================

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const FRUSTUM& _frustum,
        const VECTOR3  _point)
    {
        for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
        {
            if (Distance(_frustum.planes[i], _point) < -kEpsilon)
            {
                return false;
            }
        }
        return true;
    }

    // 보수적 판정.
    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const FRUSTUM& _x,
        const FRUSTUM& _y)
    {
        const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> cornersX = _x.CalcCorners();
        const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> cornersY = _y.CalcCorners();
        for (const eFrustumPlane e0: RangesOf<eFrustumPlane>())
        {
            const PLANE plane   = _x.planes[e0];
            bool        bAllOut = true;
            for (const eCorner e1: RangesOf<eCorner>())
            {
                if (Distance(plane, cornersY[e1]) >= 0.f)
                {
                    bAllOut = false;
                    break;
                }
            }
            if (bAllOut)
            {
                return false;
            }
        }

        for (const eFrustumPlane e0: RangesOf<eFrustumPlane>())
        {
            const PLANE plane   = _y.planes[e0];
            bool        bAllOut = true;
            for (const eCorner e1: RangesOf<eCorner>())
            {
                if (Distance(plane, cornersX[e1]) >= 0.f)
                {
                    bAllOut = false;
                    break;
                }
            }
            if (bAllOut)
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool Intersect(
        const FRUSTUM& _frustum,
        const RAY&     _ray)
    {
        return Raycast(_ray, _frustum);
    }

}   // namespace collision_detail

// ====================================================
//  Intersect / Disjoint 확장
// ====================================================

template<typename T, typename U>
    requires requires(const T& _x, const U& _y) { collision_detail::Intersect(_x, _y); }
          || requires(const T& _x, const U& _y) { collision_detail::Intersect(_y, _x); }
[[nodiscard]] JUG_MATH_API constexpr bool Intersect(
    const T& _x,
    const U& _y)
{
    if constexpr (requires { collision_detail::Intersect(_x, _y); })
    {
        return collision_detail::Intersect(_x, _y);
    }
    else
    {
        return collision_detail::Intersect(_y, _x);
    }
}

template<typename T, typename U>
    requires requires(const T& _x, const U& _y) { Intersect(_x, _y); }
[[nodiscard]] JUG_MATH_API constexpr bool Disjoint(
    const T& _x,
    const U& _y)
{
    return !Intersect(_x, _y);
}

// ====================================================
//  평면과 기하 간의 관계
// ====================================================

[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const PLANE   _plane,
    const VECTOR3 _point)
{
    return Distance(_plane, _point) > 0.f;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const PLANE   _plane,
    const VECTOR3 _point)
{
    return Distance(_plane, _point) < 0.f;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const PLANE _plane,
    const AABB& _aabb)
{
    return Distance(_plane, _aabb.center) > Dot(Abs(_plane.normal), Abs(_aabb.extends));
}

[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const PLANE _plane,
    const AABB& _aabb)
{
    return Distance(_plane, _aabb.center) < -Dot(Abs(_plane.normal), Abs(_aabb.extends));
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const PLANE _plane,
    const OBB&  _obb)
{
    const collision_detail::BOX box = collision_detail::MakeBox(_obb);
    return Distance(_plane, box.center) > collision_detail::ProjectedRadius(box, _plane.normal);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const PLANE _plane,
    const OBB&  _obb)
{
    const collision_detail::BOX box = collision_detail::MakeBox(_obb);
    return Distance(_plane, box.center) < -collision_detail::ProjectedRadius(box, _plane.normal);
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const PLANE   _plane,
    const SPHERE& _sphere)
{
    return Distance(_plane, _sphere.center) > _sphere.radius;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const PLANE   _plane,
    const SPHERE& _sphere)
{
    return Distance(_plane, _sphere.center) < -_sphere.radius;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const PLANE     _plane,
    const TRIANGLE& _triangle)
{
    return Min(Distance(_plane, _triangle.p0), Distance(_plane, _triangle.p1), Distance(_plane, _triangle.p2)) > 0.f;
}

[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const PLANE     _plane,
    const TRIANGLE& _triangle)
{
    return Max(Distance(_plane, _triangle.p0), Distance(_plane, _triangle.p1), Distance(_plane, _triangle.p2)) < 0.f;
}

// ====================================================
//  Frustum 과 기하 간의 관계
// ====================================================

template<typename T>
    requires requires(const PLANE _p, const T& _t) { IsFront(_p, _t); }
[[nodiscard]] JUG_MATH_API constexpr bool IsFront(
    const FRUSTUM& _frustum,
    const T&       _t)
{
    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        if (!IsFront(_frustum.planes[i], _t))
        {
            return false;
        }
    }
    return true;
}

template<typename T>
    requires requires(const PLANE _p, const T& _t) { IsBehind(_p, _t); }
[[nodiscard]] JUG_MATH_API constexpr bool IsBehind(
    const FRUSTUM& _frustum,
    const T&       _t)
{
    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        if (!IsBehind(_frustum.planes[i], _t))
        {
            return false;
        }
    }
    return true;
}

// ====================================================
//  Contains
// ====================================================

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB&   _aabb,
    const VECTOR3 _point)
{
    return collision_detail::Intersect(_aabb, _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB& _x,
    const AABB& _y)
{
    const VECTOR3 min0 = _x.GetMin();
    const VECTOR3 max0 = _x.GetMax();
    const VECTOR3 min1 = _y.GetMin();
    const VECTOR3 max1 = _y.GetMax();
    return min1.e[0] >= min0.e[0] && max1.e[0] <= max0.e[0]
        && min1.e[1] >= min0.e[1] && max1.e[1] <= max0.e[1]
        && min1.e[2] >= min0.e[2] && max1.e[2] <= max0.e[2];
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB& _aabb,
    const OBB&  _obb)
{
    return collision_detail::Contains(collision_detail::MakeBox(_aabb), collision_detail::MakeBox(_obb));
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB&   _aabb,
    const SPHERE& _sphere)
{
    const VECTOR3 min = _aabb.GetMin();
    const VECTOR3 max = _aabb.GetMax();
    const float   r   = _sphere.radius;
    return _sphere.center.e[0] - r >= min.e[0] && _sphere.center.e[0] + r <= max.e[0]
        && _sphere.center.e[1] - r >= min.e[1] && _sphere.center.e[1] + r <= max.e[1]
        && _sphere.center.e[2] - r >= min.e[2] && _sphere.center.e[2] + r <= max.e[2];
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB&     _aabb,
    const TRIANGLE& _triangle)
{
    return collision_detail::Intersect(_aabb, _triangle.p0)
        && collision_detail::Intersect(_aabb, _triangle.p1)
        && collision_detail::Intersect(_aabb, _triangle.p2);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const AABB&    _aabb,
    const FRUSTUM& _frustum)
{
    const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners = _frustum.CalcCorners();
    for (const eCorner e: RangesOf<eCorner>())
    {
        if (!collision_detail::Intersect(_aabb, corners[e]))
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB&    _obb,
    const VECTOR3 _point)
{
    return collision_detail::ContainBoxLocalPoint(collision_detail::MakeBox(_obb), _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB&  _obb,
    const AABB& _aabb)
{
    return collision_detail::Contains(collision_detail::MakeBox(_obb), collision_detail::MakeBox(_aabb));
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB& _x,
    const OBB& _y)
{
    return collision_detail::Contains(collision_detail::MakeBox(_x), collision_detail::MakeBox(_y));
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB&    _obb,
    const SPHERE& _sphere)
{
    const collision_detail::BOX box   = collision_detail::MakeBox(_obb);
    const VECTOR3               local = Abs(collision_detail::ToBoxLocal(box, _sphere.center));
    return local.e[0] + _sphere.radius <= box.extents.e[0]
        && local.e[1] + _sphere.radius <= box.extents.e[1]
        && local.e[2] + _sphere.radius <= box.extents.e[2];
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB&      _obb,
    const TRIANGLE& _triangle)
{
    const collision_detail::BOX box = collision_detail::MakeBox(_obb);
    return collision_detail::ContainBoxLocalPoint(box, _triangle.p0)
        && collision_detail::ContainBoxLocalPoint(box, _triangle.p1)
        && collision_detail::ContainBoxLocalPoint(box, _triangle.p2);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const OBB&     _obb,
    const FRUSTUM& _frustum)
{
    const collision_detail::BOX               box     = collision_detail::MakeBox(_obb);
    const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners = _frustum.CalcCorners();
    for (const eCorner e: RangesOf<eCorner>())
    {
        if (!collision_detail::ContainBoxLocalPoint(box, corners[e]))
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE& _sphere,
    const VECTOR3 _point)
{
    return collision_detail::Intersect(_sphere, _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE& _sphere,
    const AABB&   _aabb)
{
    // 박스에서 구 중심으로부터 가장 먼 점까지의 거리로 판정한다.
    const VECTOR3 farthest = Abs(_aabb.center - _sphere.center) + Abs(_aabb.extends);
    return LengthSq(farthest) <= _sphere.radius * _sphere.radius;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE& _sphere,
    const OBB&    _obb)
{
    const collision_detail::BOX box      = collision_detail::MakeBox(_obb);
    const VECTOR3               local    = Abs(collision_detail::ToBoxLocal(box, _sphere.center));
    const VECTOR3               farthest = local + box.extents;   // 박스 축은 정규 직교이므로 로컬 거리 = 월드 거리
    return LengthSq(farthest) <= _sphere.radius * _sphere.radius;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE& _x,
    const SPHERE& _y)
{
    if (_y.radius > _x.radius)
    {
        return false;
    }
    const float d = _x.radius - _y.radius;
    return DistanceSq(_x.center, _y.center) <= d * d;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE&   _sphere,
    const TRIANGLE& _triangle)
{
    const float rSq = _sphere.radius * _sphere.radius;
    return DistanceSq(_sphere.center, _triangle.p0) <= rSq
        && DistanceSq(_sphere.center, _triangle.p1) <= rSq
        && DistanceSq(_sphere.center, _triangle.p2) <= rSq;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const SPHERE&  _sphere,
    const FRUSTUM& _frustum)
{
    const float                               rSq     = _sphere.radius * _sphere.radius;
    const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners = _frustum.CalcCorners();
    for (const eCorner e: RangesOf<eCorner>())
    {
        if (DistanceSq(_sphere.center, corners[e]) > rSq)
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const TRIANGLE& _triangle,
    const VECTOR3   _point)
{
    return collision_detail::Intersect(_triangle, _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const PLANE   _plane,
    const VECTOR3 _point)
{
    return collision_detail::Intersect(_plane, _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const PLANE     _plane,
    const TRIANGLE& _triangle)
{
    return IsZeroApprox(Distance(_plane, _triangle.p0))
        && IsZeroApprox(Distance(_plane, _triangle.p1))
        && IsZeroApprox(Distance(_plane, _triangle.p2));
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM& _frustum,
    const VECTOR3  _point)
{
    return collision_detail::Intersect(_frustum, _point);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM& _frustum,
    const AABB&    _aabb)
{
    const VECTOR3 extents = Abs(_aabb.extends);
    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        const PLANE plane = _frustum.planes[i];
        if (Distance(plane, _aabb.center) < Dot(extents, Abs(plane.normal)))
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM& _frustum,
    const OBB&     _obb)
{
    const collision_detail::BOX box = collision_detail::MakeBox(_obb);
    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        const PLANE plane = _frustum.planes[i];
        if (Distance(plane, box.center) < collision_detail::ProjectedRadius(box, plane.normal))
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM& _frustum,
    const SPHERE&  _sphere)
{
    for (size_t i = 0; i < _frustum.planes.GetSize(); ++i)
    {
        if (Distance(_frustum.planes[i], _sphere.center) < _sphere.radius)
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM&  _frustum,
    const TRIANGLE& _triangle)
{
    return collision_detail::Intersect(_frustum, _triangle.p0)
        && collision_detail::Intersect(_frustum, _triangle.p1)
        && collision_detail::Intersect(_frustum, _triangle.p2);
}

[[nodiscard]] JUG_MATH_API constexpr bool Contains(
    const FRUSTUM& _x,
    const FRUSTUM& _y)
{
    const DIRECT_ENUM_ARRAY<eCorner, VECTOR3> corners = _y.CalcCorners();
    for (const eCorner e: RangesOf<eCorner>())
    {
        if (!collision_detail::Intersect(_x, corners[e]))
        {
            return false;
        }
    }
    return true;
}

}   // namespace jug
