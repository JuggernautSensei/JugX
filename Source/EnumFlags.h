#pragma once
#include <initializer_list>
#include <limits>

#include "EnumRefl.h"

namespace jug
{

struct ZeroFlagType
{
    struct Tag
    {
    };

    constexpr explicit ZeroFlagType(
        const Tag)
    {
    }
};

inline constexpr ZeroFlagType kZeroFlag { ZeroFlagType::Tag {} };

struct AllFlagType
{
    struct Tag
    {
    };

    constexpr explicit AllFlagType(
        const Tag)
    {
    }
};

inline constexpr AllFlagType kAllFlag { AllFlagType::Tag {} };

// ==========================================================
//  EnumFlags
//   kbBitmaskItSelf = true:  스스로 비트 마스킹을 표현할 수 있나?
//                            enum eColor { Red = 1, Green = 2, Blue = 4 }; // 이런식으로 정의되어있으면 kbBitmaskItSelf = true
//
//   kbBitmaskItSelf = false: 비트 마스킹을 표현할 수 없고, 단순히 인덱스로만 사용하나?
//                            enum eColor { Red, Green, Blue }; // 이런식으로 정의되어있으면 kbBitmaskItSelf = false
// ==========================================================

template<EnumT TEnum, bool kbBitmaskItSelf>
class BaseFlags
{
public:
    using UnderlyingT = UnderlyingT<TEnum>;

    constexpr BaseFlags() noexcept = default;

    /* implicit */ constexpr BaseFlags(
        const TEnum _value) noexcept
        : m_flags(MakeMask_(_value))
    {
    }

    /* implicit */ constexpr BaseFlags(
        const ZeroFlagType) noexcept
        : m_flags(0)
    {
    }

    /* implicit */ constexpr BaseFlags(
        const AllFlagType) noexcept
        : m_flags(static_cast<UnderlyingT>(~static_cast<UnderlyingT>(0)))
    {
    }

    /* implicit */ constexpr BaseFlags(
        const std::initializer_list<TEnum> _values) noexcept
    {
        for (const TEnum& value: _values)
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
        const TEnum _value) const noexcept
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
        const TEnum _value) noexcept
    {
        if constexpr (!kbBitmaskItSelf && std::is_signed_v<UnderlyingT>)
        {
            JUG_ASSERT(ToUnderlying(_value) >= 0, "BaseFlags: enum value must be non-negative");
        }

        if constexpr (kbBitmaskItSelf)
        {
            return static_cast<UnderlyingT>(_value);
        }
        else
        {
            JUG_ASSERT(ToUnderlying(_value) < std::numeric_limits<UnderlyingT>::digits, "BaseFlags: enum value must be less than the number of bits in the underlying type");
            return static_cast<UnderlyingT>(static_cast<UnderlyingT>(1) << static_cast<UnderlyingT>(_value));
        }
    }

    UnderlyingT m_flags = 0;
};

template<EnumT TEnum>
using Flags = BaseFlags<TEnum, true>;

template<EnumT TEnum>
using IndexedFlags = BaseFlags<TEnum, false>;

}   // namespace jug

