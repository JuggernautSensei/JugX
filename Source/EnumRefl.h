#pragma once
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Config.h"
#include "Macros.h"   // NOLINT
#include "Math.h"
#include "StringHasher.h"

namespace jug
{

// =========================================================
//  EnumRefl
// =========================================================

constexpr int kEnumReflMax = 127;
constexpr int kEnumReflMin = 0;

template<typename T>
concept EnumT = std::is_enum_v<T>;

template<EnumT E>
using UnderlyingT = std::underlying_type_t<E>;

namespace enum_relf_detail
{
    inline constexpr int kReflectRange = kEnumReflMax - kEnumReflMin + 1;
    inline constexpr int KInvalidIndex = std::numeric_limits<int>::max();

    template<EnumT E, E kValue>
    [[nodiscard]] constexpr StringView ExtractFullName()
    {
        constexpr StringView kName = JUG_PRETTY_FUNCTION;
#if defined(__clang__) || defined(__GNUC__)
        constexpr StringView kMarker    = "kValue = ";
        constexpr size_t     kMarkerPos = kName.find(kMarker);
        static_assert(kMarkerPos != StringView::npos);   // NOLINT
        constexpr size_t kBegin   = kMarkerPos + kMarker.size();
        constexpr size_t kSemi    = kName.find(';', kBegin);
        constexpr size_t kComma   = kName.find(',', kBegin);
        constexpr size_t kBracket = kName.find(']', kBegin);
        constexpr size_t kEnd     = kSemi < kComma ? (kSemi < kBracket ? kSemi : kBracket) : (kComma < kBracket ? kComma : kBracket);   // NOLINT
        static_assert(kEnd != StringView::npos);
        return kName.substr(kBegin, kEnd - kBegin);
#elif defined(_MSC_VER)
        constexpr size_t kBegin = kName.rfind(',');
        constexpr size_t kEnd   = kName.rfind('>');
        static_assert(kBegin != StringView::npos && kEnd != StringView::npos);
        return kName.substr(kBegin + 1, kEnd - kBegin - 1);
#else
#    error "EnumRefl: unsupported compiler (requires MSVC, Clang, or GCC)"
#endif
    }

    [[nodiscard]] constexpr StringView ExtractName(
        const StringView _fullName)
    {
        const size_t colon = _fullName.rfind("::");
        return colon == StringView::npos ? _fullName : _fullName.substr(colon + 2);
    }

    [[nodiscard]] constexpr bool IsValidName(
        const StringView _fullName)
    {
        const size_t     colon = _fullName.rfind("::");
        const StringView tail  = colon == StringView::npos ? _fullName : _fullName.substr(colon + 2);
        return tail.find('(') == StringView::npos && tail.find(')') == StringView::npos;
    }

    template<EnumT E, int... kOffsets>
    [[nodiscard]] constexpr size_t CountOfImpl(
        std::integer_sequence<int, kOffsets...>)
    {
        return (static_cast<size_t>(IsValidName(ExtractFullName<E, static_cast<E>(kEnumReflMin + kOffsets)>())) + ...);
    }

    template<EnumT E>
    inline constexpr size_t kCount = CountOfImpl<E>(std::make_integer_sequence<int, kReflectRange> {});

    template<EnumT E>
    struct EnumMetadata
    {
        ARRAY<E, kCount<E>>          values    = {};
        ARRAY<StringView, kCount<E>> names     = {};
        ARRAY<StringView, kCount<E>> fullNames = {};
        ARRAY<uint64_t, kCount<E>>   hashes    = {};
        ARRAY<size_t, kReflectRange> lut       = {};
        E                            min       = static_cast<E>(kEnumReflMax);
        E                            max       = static_cast<E>(kEnumReflMin);
    };

    template<EnumT E, int... kOffsets>
    [[nodiscard]] constexpr EnumMetadata<E> BuildEnumMetadataImpl(
        std::integer_sequence<int, kOffsets...>)
    {
        const ARRAY<StringView, kReflectRange> fullNames = {
            ExtractFullName<E, static_cast<E>(kEnumReflMin + kOffsets)>()...
        };

        EnumMetadata<E> metadata = {};
        size_t          index    = 0;
        for (int i = 0; i < kReflectRange; ++i)
        {
            if (!IsValidName(fullNames[i]))
            {
                metadata.lut[i] = KInvalidIndex;
                continue;
            }

            // metadata
            const E e                 = static_cast<E>(kEnumReflMin + i);
            metadata.values[index]    = e;
            metadata.names[index]     = ExtractName(fullNames[i]);
            metadata.fullNames[index] = fullNames[i];
            metadata.hashes[index]    = HashString(fullNames[i]);
            metadata.lut[i]           = index;
            metadata.min              = e < metadata.min ? e : metadata.min;
            metadata.max              = e > metadata.max ? e : metadata.max;
            ++index;
        }
        return metadata;
    }

    template<EnumT E>
    constexpr EnumMetadata<E> BuildEnumMetadata()
    {
        return BuildEnumMetadataImpl<E>(std::make_integer_sequence<int, kReflectRange> {});
    }

    template<EnumT E>
    inline constexpr EnumMetadata<E> kEnumMetadata = BuildEnumMetadata<E>();

    template<EnumT E>
    constexpr const EnumMetadata<E>& GetEnumMetadata()
    {
        return kEnumMetadata<E>;
    }

    template<EnumT E>
    constexpr size_t GetIndexOrInvalid(
        const E _value)
    {
        const int value = static_cast<int>(_value);
        if (value < kEnumReflMin || value > kEnumReflMax)
        {
            return KInvalidIndex;
        }
        return enum_relf_detail::GetEnumMetadata<E>().lut[value - kEnumReflMin];
    }
}   // namespace enum_relf_detail

// ===========================================================================
//  Public API
// ===========================================================================

template<EnumT E>
[[nodiscard]] constexpr UnderlyingT<E> ToUnderlying(
    const E _value) noexcept
{
    return static_cast<UnderlyingT<E>>(_value);
}

template<EnumT E>
[[nodiscard]] constexpr size_t CountOf()
{
    return enum_relf_detail::kCount<E>;
}

template<EnumT E>
[[nodiscard]] constexpr bool IsValid(
    const E _value)
{
    return enum_relf_detail::GetIndexOrInvalid<E>(_value) != enum_relf_detail::KInvalidIndex;
}

template<EnumT E>
[[nodiscard]] constexpr StringView NameOf(
    const E _value)
{
    const size_t index = enum_relf_detail::GetIndexOrInvalid<E>(_value);
    JUG_ASSERT(index != enum_relf_detail::KInvalidIndex, "NameOf() called with invalid enum value");
    return enum_relf_detail::GetEnumMetadata<E>().names[index];
}

template<EnumT E>
[[nodiscard]] constexpr StringView FullNameOf(
    const E _value)
{
    const size_t index = enum_relf_detail::GetIndexOrInvalid<E>(_value);
    JUG_ASSERT(index != enum_relf_detail::KInvalidIndex, "FullNameOf() called with invalid enum value");
    return enum_relf_detail::GetEnumMetadata<E>().fullNames[index];
}

template<EnumT E>
[[nodiscard]] constexpr uint64_t HashOf(
    const E _value)
{
    const size_t index = enum_relf_detail::GetIndexOrInvalid<E>(_value);
    JUG_ASSERT(index != enum_relf_detail::KInvalidIndex, "FullNameOf() called with invalid enum value");
    return enum_relf_detail::GetEnumMetadata<E>().hashes[index];
}

template<EnumT E>
[[nodiscard]] constexpr size_t GetIndex(
    const E _value)
{
    const size_t seq = enum_relf_detail::GetIndexOrInvalid(_value);
    JUG_ASSERT(seq != enum_relf_detail::KInvalidIndex, "invalid enum value");
    return seq;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& RangesOf()
{
    return enum_relf_detail::GetEnumMetadata<E>().values;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& NamesOf()
{
    return enum_relf_detail::GetEnumMetadata<E>().names;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& FullNamesOf()
{
    return enum_relf_detail::GetEnumMetadata<E>().fullNames;
}

template<EnumT E>
[[nodiscard]] constexpr E Min()
{
    return enum_relf_detail::GetEnumMetadata<E>().min;
}

template<EnumT E>
[[nodiscard]] constexpr E Max()
{
    return enum_relf_detail::GetEnumMetadata<E>().max;
}

template<EnumT E>
[[nodiscard]] constexpr std::optional<E> ParseAs(
    const StringView _name)
{
    const auto& metadata = enum_relf_detail::GetEnumMetadata<E>();
    for (size_t i = 0; i < metadata.names.size(); ++i)
    {
        if (metadata.names[i] == _name)
        {
            return metadata.values[i];
        }
    }

    for (size_t i = 0; i < metadata.fullNames.size(); ++i)
    {
        if (metadata.fullNames[i] == _name)
        {
            return metadata.values[i];
        }
    }

    return std::nullopt;
}

template<EnumT E>
[[nodiscard]] constexpr std::optional<E> ParseAs(
    const UnderlyingT<E> _value)
{
    const size_t index = enum_relf_detail::GetIndexOrInvalid<E>(static_cast<E>(_value));
    if (index == enum_relf_detail::KInvalidIndex)
    {
        return std::nullopt;
    }

    return enum_relf_detail::GetEnumMetadata<E>().values[index];
}

// ===========================================================================
//  Type Traits
// ===========================================================================

template<typename E>
concept ContiguousEnumT = EnumT<E> && IsEqual(static_cast<int>(Max<E>()) - static_cast<int>(Min<E>()) + 1, CountOf<E>());

template<typename E>
concept DirectIndexableEnumT = ContiguousEnumT<E> && static_cast<int>(Min<E>()) == 0;

}   // namespace jug
