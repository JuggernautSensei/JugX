#pragma once
#include "Math.h"
#include "InvalidIndex.h"
#include "Optional.h"
#include "StringHasher.h"

#ifndef JUG_ENUM_REFL_RANGE_MIN
#    define JUG_ENUM_REFL_RANGE_MIN 0
#endif

#ifndef JUG_ENUM_REFL_RANGE_MAX
#    define JUG_ENUM_REFL_RANGE_MAX 127
#endif

namespace jug
{

// =========================================================
//  EnumRefl
// =========================================================

template<typename T>
concept EnumT = std::is_enum_v<T>;

template<EnumT E>
using UnderlyingT = std::underlying_type_t<E>;

namespace enum_relf_detail
{
    inline constexpr int kReflRangeMin = JUG_ENUM_REFL_RANGE_MIN;
    inline constexpr int kReflRangeMax = JUG_ENUM_REFL_RANGE_MAX;
    inline constexpr int kReflRange    = kReflRangeMax - kReflRangeMin + 1;

    template<EnumT E, E kValue>
    [[nodiscard]] constexpr StringView ExtractFullName()
    {
        constexpr StringView kName = JUG_FUNCSIG;
#if defined(JUG_COMPILER_CLANG) || defined(JUG_COMPILER_GCC)
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
#elif defined(JUG_COMPILER_MSVC)
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
        return (static_cast<size_t>(IsValidName(ExtractFullName<E, static_cast<E>(kReflRangeMin + kOffsets)>())) + ...);
    }

    template<EnumT E>
    inline constexpr size_t kCount = CountOfImpl<E>(std::make_integer_sequence<int, kReflRange> {});

    template<EnumT E>
    [[nodiscard]] constexpr bool IsOutOfRange()
    {
        return IsValidName(ExtractFullName<E, static_cast<E>(kReflRangeMin - 1)>())
            || IsValidName(ExtractFullName<E, static_cast<E>(kReflRangeMax + 1)>());
    }

    template<size_t kSize>
    [[nodiscard]] constexpr ARRAY<size_t, kSize> MakeInvalidLut()
    {
        ARRAY<size_t, kSize> lut = {};
        lut.fill(kInvalidIndex);
        return lut;
    }

    template<EnumT E>
    struct Metadata
    {
        static_assert(kReflRangeMin <= kReflRangeMax, "EnumRefl: JUG_ENUM_REFL_RANGE_MIN must not exceed JUG_ENUM_REFL_RANGE_MAX");
        static_assert(kCount<E> > 0, "EnumRefl: no enumerator found in range. adjust JUG_ENUM_REFL_RANGE_MIN/MAX");
        static_assert(!IsOutOfRange<E>(), "EnumRefl: enumerator found outside range. adjust JUG_ENUM_REFL_RANGE_MIN/MAX");

        ARRAY<E, kCount<E>>          values    = {};
        ARRAY<StringView, kCount<E>> names     = {};
        ARRAY<StringView, kCount<E>> fullNames = {};
        ARRAY<uint64_t, kCount<E>>   hashes    = {};
        ARRAY<size_t, kReflRange>    lut       = MakeInvalidLut<kReflRange>();   // 원소는 kInvalidIndex 가능
        E                            min       = {};
        E                            max       = {};
    };

    template<EnumT E, int... kOffsets>
    [[nodiscard]] constexpr Metadata<E> BuildMetadataImpl(
        std::integer_sequence<int, kOffsets...>)
    {
        const ARRAY<StringView, kReflRange> fullNames = {
            ExtractFullName<E, static_cast<E>(kReflRangeMin + kOffsets)>()...
        };

        Metadata<E> metadata = {};
        size_t      index    = 0;
        for (int i = 0; i < kReflRange; ++i)
        {
            if (!IsValidName(fullNames[i]))
            {
                continue;
            }

            // metadata
            const E e                 = static_cast<E>(kReflRangeMin + i);
            metadata.values[index]    = e;
            metadata.names[index]     = ExtractName(fullNames[i]);
            metadata.fullNames[index] = fullNames[i];
            metadata.hashes[index]    = HashString(fullNames[i]);
            metadata.lut[i]           = index;
            metadata.min              = index == 0 || e < metadata.min ? e : metadata.min;
            metadata.max              = index == 0 || e > metadata.max ? e : metadata.max;
            ++index;
        }
        return metadata;
    }

    template<EnumT E>
    constexpr Metadata<E> BuildMetadata()
    {
        return BuildMetadataImpl<E>(std::make_integer_sequence<int, kReflRange> {});
    }

    template<EnumT E>
    inline constexpr Metadata<E> kMetadata = BuildMetadata<E>();

    template<EnumT E>
    constexpr const Metadata<E>& GetMetadata()
    {
        return kMetadata<E>;
    }

    template<EnumT E>
    constexpr size_t FindIndexOrInvalid(
        const E _value)
    {
        const int value = static_cast<int>(_value);
        if (value < kReflRangeMin || value > kReflRangeMax)
        {
            return kInvalidIndex;
        }
        return GetMetadata<E>().lut[value - kReflRangeMin];
    }
}   // namespace enum_relf_detail

// ===========================================================================
//  API
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
    return enum_relf_detail::FindIndexOrInvalid<E>(_value) != kInvalidIndex;
}

template<EnumT E>
[[nodiscard]] constexpr StringView NameOf(
    const E _value)
{
    const size_t indexOrInvalid = enum_relf_detail::FindIndexOrInvalid<E>(_value);
    JUG_ASSERT(indexOrInvalid != kInvalidIndex, "NameOf() called with invalid enum value");
    return enum_relf_detail::GetMetadata<E>().names[indexOrInvalid];
}

template<EnumT E>
[[nodiscard]] constexpr StringView FullNameOf(
    const E _value)
{
    const size_t indexOrInvalid = enum_relf_detail::FindIndexOrInvalid<E>(_value);
    JUG_ASSERT(indexOrInvalid != kInvalidIndex, "FullNameOf() called with invalid enum value");
    return enum_relf_detail::GetMetadata<E>().fullNames[indexOrInvalid];
}

template<EnumT E>
[[nodiscard]] constexpr uint64_t HashOf(
    const E _value)
{
    const size_t indexOrInvalid = enum_relf_detail::FindIndexOrInvalid<E>(_value);
    JUG_ASSERT(indexOrInvalid != kInvalidIndex, "HashOf() called with invalid enum value");
    return enum_relf_detail::GetMetadata<E>().hashes[indexOrInvalid];
}

template<EnumT E>
[[nodiscard]] constexpr size_t GetIndex(
    const E _value)
{
    const size_t indexOrInvalid = enum_relf_detail::FindIndexOrInvalid(_value);
    JUG_ASSERT(indexOrInvalid != kInvalidIndex, "GetIndex() called with invalid enum value");
    return indexOrInvalid;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& RangeOf()
{
    return enum_relf_detail::GetMetadata<E>().values;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& NamesOf()
{
    return enum_relf_detail::GetMetadata<E>().names;
}

template<EnumT E>
[[nodiscard]] constexpr const auto& FullNamesOf()
{
    return enum_relf_detail::GetMetadata<E>().fullNames;
}

template<EnumT E>
[[nodiscard]] constexpr E Min()
{
    return enum_relf_detail::GetMetadata<E>().min;
}

template<EnumT E>
[[nodiscard]] constexpr E Max()
{
    return enum_relf_detail::GetMetadata<E>().max;
}

template<EnumT E>
[[nodiscard]] constexpr Optional<E> ParseAs(
    const StringView _name)
{
    const auto& metadata = enum_relf_detail::GetMetadata<E>();
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

    return kNullopt;
}

template<EnumT E>
[[nodiscard]] constexpr Optional<E> ParseAs(
    const UnderlyingT<E> _value)
{
    const size_t indexOrInvalid = enum_relf_detail::FindIndexOrInvalid<E>(static_cast<E>(_value));
    if (indexOrInvalid == kInvalidIndex)
    {
        return kNullopt;
    }
    return enum_relf_detail::GetMetadata<E>().values[indexOrInvalid];
}

// ===========================================================================
//  Type Traits
// ===========================================================================

template<typename E>
concept ContiguousEnumT = EnumT<E> && IsEqual(static_cast<int>(Max<E>()) - static_cast<int>(Min<E>()) + 1, CountOf<E>());

template<typename E>
concept DirectIndexableEnumT = ContiguousEnumT<E> && static_cast<int>(Min<E>()) == 0;

}   // namespace jug
