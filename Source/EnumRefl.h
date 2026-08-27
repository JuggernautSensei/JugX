#pragma once
#include <array>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Macros.h"   // NOLINT
#include "Config.h"
#include "Math.h"
#include "StringHasher.h"

#define DT enum_refl_detail

namespace jug
{

// =========================================================
//  EnumRefl
// =========================================================

constexpr int kEnumReflMax = 127;
constexpr int kEnumReflMin = 0;

template<typename T>
concept EnumT = std::is_enum_v<T>;

template<EnumT TEnum>
using UnderlyingT = std::underlying_type_t<TEnum>;

template<typename T>
concept EnumReflT = EnumT<T>
                 && IsLessEqual(Min<UnderlyingT<T>>(), kEnumReflMin)
                 && IsLessEqual(kEnumReflMax, Max<UnderlyingT<T>>() - 1);

namespace DT
{
    inline constexpr int kReflectRange = kEnumReflMax - kEnumReflMin + 1;
    inline constexpr int KInvalidIndex = std::numeric_limits<int>::max();

    template<EnumT TEnum, TEnum kValue>
    [[nodiscard]] constexpr std::string_view ExtractFullName()
    {
        constexpr std::string_view kName = JUG_PRETTY_FUNCTION;
#if defined(__clang__) || defined(__GNUC__)
        constexpr std::string_view kMarker    = "kValue = ";
        constexpr size_t           kMarkerPos = kName.find(kMarker);
        static_assert(kMarkerPos != std::string_view::npos);   // NOLINT
        constexpr size_t kBegin   = kMarkerPos + kMarker.size();
        constexpr size_t kSemi    = kName.find(';', kBegin);
        constexpr size_t kComma   = kName.find(',', kBegin);
        constexpr size_t kBracket = kName.find(']', kBegin);
        constexpr size_t kEnd     = kSemi < kComma ? (kSemi < kBracket ? kSemi : kBracket) : (kComma < kBracket ? kComma : kBracket);   // NOLINT
        static_assert(kEnd != std::string_view::npos);
        return kName.substr(kBegin, kEnd - kBegin);
#elif defined(_MSC_VER)
        constexpr size_t kBegin = kName.rfind(',');
        constexpr size_t kEnd   = kName.rfind('>');
        static_assert(kBegin != std::string_view::npos && kEnd != std::string_view::npos);
        return kName.substr(kBegin + 1, kEnd - kBegin - 1);
#else
#    error "EnumRefl: unsupported compiler (requires MSVC, Clang, or GCC)"
#endif
    }

    [[nodiscard]] constexpr std::string_view ExtractName(
        const std::string_view _fullName)
    {
        const size_t colon = _fullName.rfind("::");
        return colon == std::string_view::npos ? _fullName : _fullName.substr(colon + 2);
    }

    [[nodiscard]] constexpr uint64_t HashOf(
        const std::string_view _fullName)
    {
        Fnv1a64 hasher {};
        hasher.Mix(_fullName);
        return hasher.GetHash();
    }

    [[nodiscard]] constexpr bool IsValidName(
        const std::string_view _fullName)
    {
        const size_t           colon = _fullName.rfind("::");
        const std::string_view tail  = colon == std::string_view::npos ? _fullName : _fullName.substr(colon + 2);
        return tail.find('(') == std::string_view::npos && tail.find(')') == std::string_view::npos;
    }

    template<EnumReflT TEnum, int... kOffsets>
    [[nodiscard]] constexpr size_t CountOfImpl(
        std::integer_sequence<int, kOffsets...>)
    {
        return (static_cast<size_t>(IsValidName(ExtractFullName<TEnum, static_cast<TEnum>(kEnumReflMin + kOffsets)>())) + ...);
    }

    template<EnumReflT TEnum>
    inline constexpr size_t kCount = CountOfImpl<TEnum>(std::make_integer_sequence<int, kReflectRange> {});

    template<EnumReflT TEnum>
    struct EnumMetadata
    {
        std::array<TEnum, kCount<TEnum>>            values    = {};
        std::array<std::string_view, kCount<TEnum>> names     = {};
        std::array<std::string_view, kCount<TEnum>> fullNames = {};
        std::array<uint64_t, kCount<TEnum>>         hashes    = {};
        std::array<size_t, kReflectRange>           lut       = {};
        TEnum                                       min       = static_cast<TEnum>(kEnumReflMax);
        TEnum                                       max       = static_cast<TEnum>(kEnumReflMin);
    };

    template<EnumReflT TEnum, int... kOffsets>
    [[nodiscard]] constexpr EnumMetadata<TEnum> BuildEnumMetadataImpl(
        std::integer_sequence<int, kOffsets...>)
    {
        const std::array<std::string_view, kReflectRange> fullNames = {
            ExtractFullName<TEnum, static_cast<TEnum>(kEnumReflMin + kOffsets)>()...
        };

        EnumMetadata<TEnum> metadata = {};
        size_t              index    = 0;
        for (int i = 0; i < kReflectRange; ++i)
        {
            if (!IsValidName(fullNames[i]))
            {
                metadata.lut[i] = KInvalidIndex;
                continue;
            }

            // metadata
            const TEnum e             = static_cast<TEnum>(kEnumReflMin + i);
            metadata.values[index]    = e;
            metadata.names[index]     = ExtractName(fullNames[i]);
            metadata.fullNames[index] = fullNames[i];
            metadata.hashes[index]    = HashOf(fullNames[i]);
            metadata.lut[i]           = index;
            metadata.min              = e < metadata.min ? e : metadata.min;
            metadata.max              = e > metadata.max ? e : metadata.max;
            ++index;
        }
        return metadata;
    }

    template<EnumReflT TEnum>
    constexpr EnumMetadata<TEnum> BuildEnumMetadata()
    {
        return BuildEnumMetadataImpl<TEnum>(std::make_integer_sequence<int, kReflectRange> {});
    }

    template<EnumReflT TEnum>
    inline constexpr EnumMetadata<TEnum> kEnumMetadata = BuildEnumMetadata<TEnum>();

    template<EnumReflT TEnum>
    constexpr const EnumMetadata<TEnum>& GetEnumMetadata()
    {
        return kEnumMetadata<TEnum>;
    }

    template<EnumReflT TEnum>
    constexpr size_t GetIndexOrInvalid(
        const TEnum _value)
    {
        const int value = static_cast<int>(_value);
        if (value < kEnumReflMin || value > kEnumReflMax)
        {
            return KInvalidIndex;
        }
        return DT::GetEnumMetadata<TEnum>().lut[value - kEnumReflMin];
    }
}   // namespace DT

// ===========================================================================
//  Public API
// ===========================================================================

template<EnumT TEnum>
[[nodiscard]] constexpr UnderlyingT<TEnum> ToUnderlying(
    const TEnum _value) noexcept
{
    return static_cast<UnderlyingT<TEnum>>(_value);
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr size_t CountOf()
{
    return DT::kCount<TEnum>;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr bool IsValid(
    const TEnum _value)
{
    return DT::GetIndexOrInvalid<TEnum>(_value) != DT::KInvalidIndex;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr std::string_view NameOf(
    const TEnum _value)
{
    const size_t index = DT::GetIndexOrInvalid<TEnum>(_value);
    JUG_ASSERT(index != DT::KInvalidIndex, "NameOf() called with invalid enum value");
    return DT::GetEnumMetadata<TEnum>().names[index];
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr std::string_view FullNameOf(
    const TEnum _value)
{
    const size_t index = DT::GetIndexOrInvalid<TEnum>(_value);
    JUG_ASSERT(index != DT::KInvalidIndex, "FullNameOf() called with invalid enum value");
    return DT::GetEnumMetadata<TEnum>().fullNames[index];
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr uint64_t HashOf(
    const TEnum _value)
{
    const size_t index = DT::GetIndexOrInvalid<TEnum>(_value);
    JUG_ASSERT(index != DT::KInvalidIndex, "FullNameOf() called with invalid enum value");
    return DT::GetEnumMetadata<TEnum>().hashes[index];
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr size_t GetIndex(
    const TEnum _value)
{
    const size_t seq = DT::GetIndexOrInvalid(_value);
    JUG_ASSERT(seq != DT::KInvalidIndex, "invalid enum value");
    return seq;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr const auto& RangesOf()
{
    return DT::GetEnumMetadata<TEnum>().values;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr const auto& NamesOf()
{
    return DT::GetEnumMetadata<TEnum>().names;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr const auto& FullNamesOf()
{
    return DT::GetEnumMetadata<TEnum>().fullNames;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr TEnum Min()
{
    return DT::GetEnumMetadata<TEnum>().min;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr TEnum Max()
{
    return DT::GetEnumMetadata<TEnum>().max;
}

template<EnumReflT TEnum>
[[nodiscard]] constexpr std::optional<TEnum> ParseAs(
    const std::string_view _name)
{
    const auto& metadata = DT::GetEnumMetadata<TEnum>();
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

template<EnumReflT TEnum>
[[nodiscard]] constexpr std::optional<TEnum> ParseAs(
    const UnderlyingT<TEnum> _value)
{
    const size_t index = DT::GetIndexOrInvalid<TEnum>(static_cast<TEnum>(_value));
    if (index == DT::KInvalidIndex)
    {
        return std::nullopt;
    }

    return DT::GetEnumMetadata<TEnum>().values[index];
}

// ===========================================================================
//  Type Traits
// ===========================================================================

template<typename TEnum>
concept ContiguousEnumT = EnumReflT<TEnum> && IsEqual(static_cast<int>(Max<TEnum>()) - static_cast<int>(Min<TEnum>()) + 1, CountOf<TEnum>());

template<typename TEnum>
concept DirectIndexableEnumT = ContiguousEnumT<TEnum> && static_cast<int>(Min<TEnum>()) == 0;

}   // namespace jug

#undef DT