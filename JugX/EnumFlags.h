#pragma once
#include "EnumRefl.h"
#include "Tag.h"

namespace jug
{

JUG_DEFINE_TAG(NoneFlagsType, kNoneFlags)
JUG_DEFINE_TAG(AllFlagsType, kAllFlags)

// ==========================================================
//  EnumFlags
//   kbBitmaskItself = true:  스스로 비트 마스킹을 표현할 수 있나?
//   kbBitmaskItself = false: 비트 마스킹을 표현할 수 없고, 단순히 인덱스로만 사용하나?
// ==========================================================

template<EnumT E, bool kbBitmaskItself>
class BaseFlags
{
public:
    using UnderlyingT = UnderlyingT<E>;

    constexpr BaseFlags() noexcept = default;

    /* implicit */ constexpr BaseFlags(
        const E _value) noexcept
        : m_flags(MakeMask_(_value))
    {
    }

    /* implicit */ constexpr BaseFlags(
        const NoneFlagsType) noexcept
        : m_flags(0)
    {
    }

    /* implicit */ constexpr BaseFlags(
        const AllFlagsType) noexcept
        : m_flags(static_cast<UnderlyingT>(~static_cast<UnderlyingT>(0)))
    {
    }

    constexpr BaseFlags(
        const std::initializer_list<E> _values) noexcept
    {
        for (const E& value: _values)
        {
            m_flags = static_cast<UnderlyingT>(m_flags | MakeMask_(value));
        }
    }

    explicit constexpr BaseFlags(
        const UnderlyingT _flags) noexcept
        : m_flags(_flags)
    {
    }

    [[nodiscard]] constexpr bool Has(
        const E _value) const noexcept
    {
        return (m_flags & MakeMask_(_value)) != 0;
    }

    [[nodiscard]] constexpr bool HasAny(
        const BaseFlags _other) const noexcept
    {
        return (m_flags & _other.m_flags) != 0;
    }

    [[nodiscard]] constexpr bool HasAll(
        const BaseFlags _other) const noexcept
    {
        return (m_flags & _other.m_flags) == _other.m_flags;
    }

    constexpr void Enable(
        const BaseFlags _flags) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags | _flags.m_flags);
    }

    constexpr void Disable(
        const BaseFlags _value) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags & static_cast<UnderlyingT>(~_value.m_flags));
    }

    constexpr void Set(
        const BaseFlags _value,
        const bool      _bEnable) noexcept
    {
        if (_bEnable)
        {
            Enable(_value);
        }
        else
        {
            Disable(_value);
        }
    }

    constexpr void Toggle(
        const BaseFlags _value) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags ^ _value.m_flags);
    }

    constexpr BaseFlags& operator|=(
        const BaseFlags _other) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags | _other.m_flags);
        return *this;
    }

    constexpr BaseFlags& operator&=(
        const BaseFlags _other) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags & _other.m_flags);
        return *this;
    }

    constexpr BaseFlags& operator^=(
        const BaseFlags _other) noexcept
    {
        m_flags = static_cast<UnderlyingT>(m_flags ^ _other.m_flags);
        return *this;
    }

    [[nodiscard]] constexpr BaseFlags operator|(
        const BaseFlags _other) const noexcept
    {
        return BaseFlags { static_cast<UnderlyingT>(m_flags | _other.m_flags) };
    }

    [[nodiscard]] constexpr BaseFlags operator&(
        const BaseFlags _other) const noexcept
    {
        return BaseFlags { static_cast<UnderlyingT>(m_flags & _other.m_flags) };
    }

    [[nodiscard]] constexpr BaseFlags operator^(
        const BaseFlags _other) const noexcept
    {
        return BaseFlags { static_cast<UnderlyingT>(m_flags ^ _other.m_flags) };
    }

    [[nodiscard]] constexpr bool operator==(
        const BaseFlags _other) const noexcept
    {
        return m_flags == _other.m_flags;
    }

    [[nodiscard]] constexpr UnderlyingT GetFlags() const noexcept
    {
        return m_flags;
    }

    explicit constexpr operator bool() const noexcept
    {
        return m_flags != 0;
    }

private:
    [[nodiscard]] static constexpr UnderlyingT MakeMask_(
        const E _value) noexcept
    {
        if constexpr (!kbBitmaskItself && std::is_signed_v<UnderlyingT>)
        {
            JUG_ASSERT(ToUnderlying(_value) >= 0, "BaseFlags: enum time must be non-negative");
        }

        if constexpr (kbBitmaskItself)
        {
            return static_cast<UnderlyingT>(_value);
        }
        else
        {
            JUG_ASSERT(ToUnderlying(_value) < std::numeric_limits<UnderlyingT>::digits, "BaseFlags: enum time must be less than the number of bits in the underlying type");
            return static_cast<UnderlyingT>(static_cast<UnderlyingT>(1) << static_cast<UnderlyingT>(_value));
        }
    }

    UnderlyingT m_flags = 0;
};

template<EnumT E>
using Flags = BaseFlags<E, true>;

template<EnumT E>
using IndexedFlags = BaseFlags<E, false>;

// ==========================================================
//  택일 그룹 (비트 패킹)
//   0 << n (default), 1 << n, 2 << n ... 으로 나란히 놓은 값들 중 하나만 고르는 그룹
// ==========================================================

// _lowestNotZero: 0 이 아닌 가장 작은 값, _highest: 가장 큰 값
template<EnumT E, typename U = UnderlyingT<E>>
[[nodiscard]] constexpr U MakeFieldMask(
    const E _lowestNotZero,
    const E _highest)
{
    JUG_ASSERT(static_cast<U>(_lowestNotZero) > 0, "_lowestNotZero must be greater than zero");
    JUG_ASSERT(static_cast<U>(_lowestNotZero) <= static_cast<U>(_highest), "_lowestNotZero must not exceed _highest");

    const U high = static_cast<U>(std::bit_ceil(static_cast<U>(static_cast<U>(_highest) + 1)) - 1);
    const U low  = static_cast<U>(std::bit_floor(static_cast<U>(_lowestNotZero)) - 1);
    return static_cast<U>(high ^ low);
}

// 그룹 값을 0 부터 시작하는 번호로 꺼냄 (0 << n 이면 0, 1 << n 이면 1 ...)
template<EnumT E>
[[nodiscard]] constexpr uint32_t ExtractField(
    const Flags<E> _flags,
    const Flags<E> _mask)
{
    return static_cast<uint32_t>((_flags & _mask).GetFlags()) >> std::countr_zero(_mask.GetFlags());
}

}   // namespace jug
