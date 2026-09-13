#include "Random.h"
#include <cstdint>
#include <random>

namespace jug
{

namespace
{
    [[nodiscard]] uint64_t MakeRandomDeviceSeed_()
    {
        std::random_device rd = {};
        return (static_cast<uint64_t>(rd()) << 32) | static_cast<uint64_t>(rd());
    }

}   // namespace

RngMwc::RngMwc(
    const RandomDeviceSeed) noexcept
    : m_z(MakeRandomDeviceSeed_())
    , m_w(MakeRandomDeviceSeed_())
{
}

RngMwc::RngMwc(
    const uint64_t _z,
    const uint64_t _w) noexcept
    : m_z(_z)
    , m_w(_w)
{
}

void RngMwc::Reseed(
    const RandomDeviceSeed) noexcept
{
    m_z = MakeRandomDeviceSeed_();
    m_w = MakeRandomDeviceSeed_();
}

void RngMwc::Reseed(
    const uint64_t _z,
    const uint64_t _w) noexcept
{
    m_z = _z;
    m_w = _w;
}

uint64_t RngMwc::Generate() const
{
    const uint64_t hi = (36969ULL * (m_z & 0xffffULL) + (m_z >> 16)) & 0xffffffffULL;
    const uint64_t lo = (18000ULL * (m_w & 0xffffULL) + (m_w >> 16)) & 0xffffffffULL;
    return (hi << 32) | lo;
}

RngShr3::RngShr3(
    const RandomDeviceSeed) noexcept
    : m_jsr(MakeRandomDeviceSeed_())
{
}

RngShr3::RngShr3(
    const uint64_t _seed) noexcept
    : m_jsr(_seed)
{
}

void RngShr3::Reseed(
    const RandomDeviceSeed) noexcept
{
    m_jsr = MakeRandomDeviceSeed_();
}

void RngShr3::Reseed(
    const uint64_t _seed) noexcept
{
    m_jsr = _seed;
}

uint64_t RngShr3::Generate()
{
    m_jsr ^= m_jsr << 13;
    m_jsr ^= m_jsr >> 7;
    m_jsr ^= m_jsr << 17;
    return m_jsr;
}

RngMt19937::RngMt19937(
    const RandomDeviceSeed) noexcept
    : m_mt(MakeRandomDeviceSeed_())
{
}

RngMt19937::RngMt19937(
    const uint64_t _seed) noexcept
    : m_mt(_seed)
{
}

void RngMt19937::Reseed(
    const RandomDeviceSeed) noexcept
{
    m_mt.seed(MakeRandomDeviceSeed_());
}

void RngMt19937::Reseed(
    const uint64_t _seed) noexcept
{
    m_mt.seed(_seed);
}

uint64_t RngMt19937::Generate()
{
    return m_mt();
}

}   // namespace jug
