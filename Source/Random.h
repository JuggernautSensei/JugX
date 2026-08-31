#pragma once
#include <cstdint>
#include <random>

#include "Config.h"
#include "TypeTraits.h"
#include "Math.h"
#include "Vector.h"

namespace jug
{

struct RandomDeviceSeed
{
    struct Tag
    {
    };

    explicit constexpr RandomDeviceSeed(Tag) {}
};

inline constexpr RandomDeviceSeed kRandomDeviceSeed { RandomDeviceSeed::Tag {} };

class RngMwc
{
public:
    explicit RngMwc(RandomDeviceSeed) noexcept;
    explicit RngMwc(uint64_t _z = 123456789123ULL, uint64_t _w = 654356789123ULL) noexcept;
    void                   Reseed(RandomDeviceSeed _seed) noexcept;
    void                   Reseed(uint64_t _z, uint64_t _w) noexcept;
    [[nodiscard]] uint64_t Generate() const;

private:
    uint64_t m_z = 0;
    uint64_t m_w = 0;
};

class RngShr3
{
public:
    explicit RngShr3(RandomDeviceSeed) noexcept;
    explicit RngShr3(uint64_t _seed = 123456789123ULL) noexcept;
    void                   Reseed(RandomDeviceSeed _seed) noexcept;
    void                   Reseed(uint64_t _seed) noexcept;
    [[nodiscard]] uint64_t Generate();

private:
    uint64_t m_jsr = 0;
};

class RngMt19937
{
public:
    explicit RngMt19937(RandomDeviceSeed) noexcept;
    explicit RngMt19937(uint64_t _seed) noexcept;
    void                   Reseed(RandomDeviceSeed _seed) noexcept;
    void                   Reseed(uint64_t _seed) noexcept;
    [[nodiscard]] uint64_t Generate();

private:
    std::mt19937_64 m_mt;
};

template<typename T>
concept RngT = requires(T _rng) {
    { _rng.Generate() } -> std::same_as<uint64_t>;
};

template<ArithmeticT T, RngT TRng>
[[nodiscard]] T Random(TRng& _rng)
{
    if constexpr (std::is_floating_point_v<T>)   // [0, 1]
    {
        return static_cast<T>(_rng.Generate()) / static_cast<T>(Max<uint64_t>());
    }
    else   // [-Max<T>(), Max<T>()]
    {
        return static_cast<T>(_rng.Generate());
    }
}

template<ArithmeticT T, RngT TRng>
[[nodiscard]] T Random(TRng& _rng, T _min, T _max)
{
    JUG_ASSERT(_min <= _max, "Invalid range: min > max.\n");
    if constexpr (std::is_floating_point_v<T>)   // [_min, _max]
    {
        const T range = _max - _min;
        return Random(_rng) * range + _min;
    }
    else   // [_min, _max]
    {
        const T range = _max - _min + 1;
        return static_cast<T>(_rng.Generate() % range) + _min;
    }
}

template<RngT TRng>
[[nodiscard]] VECTOR3 RandomVector3InUnitSphere(
    TRng&      _rng,
    const bool _onSurface = false)
{
    while (true)
    {
        const VECTOR3 v = VECTOR3 {
            Random(_rng, -1.0f, 1.0f),
            Random(_rng, -1.0f, 1.0f),
            Random(_rng, -1.0f, 1.0f)
        };

        if (LengthSq(v) <= 1.f)
        {
            return _onSurface ? Normalize(v) : v;
        }
    }
}

template<RngT TRng>
[[nodiscard]] VECTOR3 RandomVector3InHemiSphere(
    TRng&         _rng,
    const VECTOR3 _normal,
    const bool    _onSurface = false)
{
    const VECTOR3 v = RandomVector3InUnitSphere(_rng, _onSurface);
    return Dot(v, _normal) > 0.f ? v : -v;
}

}   // namespace jug